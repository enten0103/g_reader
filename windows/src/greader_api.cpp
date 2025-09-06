#define GREADER_API_EXPORTS
#include "greader_api.h"
// Vendor SDK header; path provided via CMake include directories
#include "GClient.h"
#include "message.h"
#include "delegate.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <future>
#include <chrono>
#include <thread>
#include <set>
#include <atomic>
#include <cstdarg>
#ifdef _WIN32
#  include <windows.h>
#  include <setupapi.h>
#  include <hidsdi.h>
#  pragma comment(lib, "setupapi.lib")
#  pragma comment(lib, "hid.lib")
#endif

// Simple debug logger: OutputDebugStringA on Windows and stderr otherwise.
static std::atomic<int> g_verbose{1};
static void debug_log(const char* fmt, ...) {
    if (!g_verbose.load()) return;
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
#ifdef _WIN32
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
#else
    std::fprintf(stderr, "%s\n", buf);
#endif
}

// --- Small helpers: UTF-8 conversion and JSON escaping ---
static std::string to_utf8_from_acp(const char* s) {
    if (!s) return std::string();
#ifdef _WIN32
    if (*s == '\0') return std::string();
    int wlen = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
    if (wlen <= 0) return std::string(s);
    std::wstring wbuf; wbuf.resize(wlen);
    MultiByteToWideChar(CP_ACP, 0, s, -1, &wbuf[0], wlen);
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wbuf.c_str(), -1, NULL, 0, NULL, NULL);
    if (u8len <= 0) return std::string();
    std::string utf8; utf8.resize(u8len);
    WideCharToMultiByte(CP_UTF8, 0, wbuf.c_str(), -1, &utf8[0], u8len, NULL, NULL);
    if (!utf8.empty() && utf8.back() == '\0') utf8.pop_back();
    return utf8;
#else
    return std::string(s);
#endif
}

static std::string json_escape(const std::string& s) {
    std::string out; out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04X", (unsigned)c);
                    out += buf;
                } else {
                    out += (char)c;
                }
        }
    }
    return out;
}

GREADER_API void greader_set_verbose_logging(int enabled) {
    g_verbose.store(enabled ? 1 : 0);
}

// Per-handle event queue to marshal callbacks to Dart without function pointers.
struct EventQueue {
    std::mutex mtx;
    std::queue<std::string> q; // each item is a JSON string
};

struct HandleState {
    GClient* client{nullptr};
    std::unique_ptr<EventQueue> events{new EventQueue()};
    std::string readerName; // provided at open
    std::string truncatedName; // vendor may truncate to 127 bytes
    bool isUsbHid{false}; // mark if this handle is USB HID
    std::string transport; // rs232 / tcp / rs485 / usbhid
    // Avoid flooding: report UsbHidRemoved at most once per handle
    std::atomic<bool> usbHidRemovedReported{false};
    // When the handle was opened (for initial settle delay before first write)
    std::chrono::steady_clock::time_point openedAt{std::chrono::steady_clock::now()};
    // Per-handle worker to serialize all SDK calls on a single OS thread
    std::mutex work_mtx;
    std::condition_variable work_cv;
    std::queue<std::function<void()>> work_q;
    std::thread work_thr;
    bool work_stop{false};
    bool work_started{false};
};

static HandleState* to_state(GClientHandle h) { return reinterpret_cast<HandleState*>(h); }
static GClient* to_client(GClientHandle h) { return h ? to_state(h)->client : nullptr; }
static GClientHandle from_client(GClient* c) {
    if (!c) return nullptr;
    auto* st = new HandleState();
    st->client = c;
    return reinterpret_cast<GClientHandle>(st);
}

// Map reader name -> handle state for callback routing
static std::mutex g_map_mtx;
static std::unordered_map<std::string, HandleState*> g_name_to_state;

static std::string make_truncated_key(const char* name) {
    if (!name) return std::string();
    std::string s(name);
    if (s.size() > 127) s.resize(127);
    return s;
}

static void map_register(HandleState* st, const char* name) {
    if (!st || !name) return;
    std::lock_guard<std::mutex> lk(g_map_mtx);
    st->readerName = name;
    st->truncatedName = make_truncated_key(name);
    st->openedAt = std::chrono::steady_clock::now();
    // Reset per-handle removal de-dup when (re)registering
    st->usbHidRemovedReported.store(false);
    g_name_to_state[st->readerName] = st;
    if (st->truncatedName != st->readerName) {
        g_name_to_state[st->truncatedName] = st;
    }
}

// Start a per-handle worker thread for serialized SDK calls
static void start_worker(HandleState* st) {
    if (!st || st->work_started) return;
    st->work_stop = false;
    st->work_started = true;
    st->work_thr = std::thread([st]() {
        debug_log("[GReaderDiag] worker-start name=%s", st->readerName.c_str());
        for (;;) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lk(st->work_mtx);
                st->work_cv.wait(lk, [st]() { return st->work_stop || !st->work_q.empty(); });
                if (st->work_stop && st->work_q.empty()) break;
                job = std::move(st->work_q.front());
                st->work_q.pop();
            }
            try { job(); } catch (...) {
                debug_log("[GReaderDiag] worker-job threw exception");
            }
        }
        debug_log("[GReaderDiag] worker-stop name=%s", st->readerName.c_str());
    });
}

// Stop worker and drain queue (without executing remaining jobs)
static void stop_worker(HandleState* st) {
    if (!st || !st->work_started) return;
    {
        std::lock_guard<std::mutex> lk(st->work_mtx);
        st->work_stop = true;
        // drop remaining jobs
        std::queue<std::function<void()>> empty;
        st->work_q.swap(empty);
    }
    st->work_cv.notify_all();
    if (st->work_thr.joinable()) st->work_thr.join();
    st->work_started = false;
}

// Helper to synchronously run a functor<int()> on the worker thread
static int run_on_worker(HandleState* st, std::function<int()> fn, const char* tag) {
    if (!st) return -1;
    if (!st->work_started) start_worker(st);
    debug_log("[GReaderDiag] queue %s", tag ? tag : "op");
    std::packaged_task<int()> task(std::move(fn));
    auto fut = task.get_future();
    {
        std::lock_guard<std::mutex> lk(st->work_mtx);
        st->work_q.push([pt = std::make_shared<std::packaged_task<int()>>(std::move(task)), tag]() mutable {
            debug_log("[GReaderDiag] exec %s", tag ? tag : "op");
            (*pt)();
        });
    }
    st->work_cv.notify_one();
    return fut.get();
}

// Ensure at least min_ms has elapsed since open before sending first write
static void ensure_settle_delay(HandleState* st, int min_ms) {
    if (!st || min_ms <= 0) return;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - st->openedAt).count();
    if (elapsed < min_ms) {
        int wait_ms = (int)(min_ms - elapsed);
        debug_log("[GReaderDiag] settle-wait %d ms before first write", wait_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
    }
}

static void map_unregister(HandleState* st) {
    if (!st) return;
    std::lock_guard<std::mutex> lk(g_map_mtx);
    if (!st->readerName.empty()) {
        auto it = g_name_to_state.find(st->readerName);
        if (it != g_name_to_state.end() && it->second == st) {
            g_name_to_state.erase(it);
        }
    }
    if (!st->truncatedName.empty()) {
        auto it2 = g_name_to_state.find(st->truncatedName);
        if (it2 != g_name_to_state.end() && it2->second == st) {
            g_name_to_state.erase(it2);
        }
    }
}

static void push_event(HandleState* st, const std::string& json) {
    if (!st) return;
    std::lock_guard<std::mutex> lock(st->events->mtx);
    st->events->q.push(json);
}

// Global diagnostics queue for messages not tied to a handle (e.g., HID open before handle is ready)
static EventQueue g_diag_queue;
static void push_diag(const std::string& json) {
    std::lock_guard<std::mutex> lock(g_diag_queue.mtx);
    g_diag_queue.q.push(json);
}

// Check if a HID device path (ACP) is currently present using SetupAPI (Windows only)
static bool hid_path_present_acp(const char* device_path_acp) {
#ifdef _WIN32
    if (!device_path_acp || !*device_path_acp) return false;
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO devInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devInfo == INVALID_HANDLE_VALUE) return false;
    bool found = false;
    SP_DEVICE_INTERFACE_DATA ifaceData;
    ifaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    for (DWORD index = 0; SetupDiEnumDeviceInterfaces(devInfo, NULL, &hidGuid, index, &ifaceData); ++index) {
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetail(devInfo, &ifaceData, NULL, 0, &requiredSize, NULL);
        if (requiredSize == 0) continue;
        std::vector<char> buffer(requiredSize);
        auto detail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(buffer.data());
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (SetupDiGetDeviceInterfaceDetail(devInfo, &ifaceData, detail, requiredSize, NULL, NULL)) {
#ifdef UNICODE
            int wlen = (int)wcslen(detail->DevicePath);
            int alen = WideCharToMultiByte(CP_ACP, 0, detail->DevicePath, wlen, NULL, 0, NULL, NULL);
            if (alen > 0) {
                std::string acp(alen, '\0');
                WideCharToMultiByte(CP_ACP, 0, detail->DevicePath, wlen, &acp[0], alen, NULL, NULL);
                if (_stricmp(acp.c_str(), device_path_acp) == 0) { found = true; break; }
            }
#else
            if (_stricmp(detail->DevicePath, device_path_acp) == 0) { found = true; break; }
#endif
        }
    }
    SetupDiDestroyDeviceInfoList(devInfo);
    return found;
#else
    (void)device_path_acp;
    return false;
#endif
}

// Best-effort cleanup for any existing USB HID handles to avoid stale callbacks/events
static void close_all_hid_handles() {
    std::vector<HandleState*> to_close;
    {
        std::lock_guard<std::mutex> lk(g_map_mtx);
        // Collect unique HandleState* marked as HID
        std::set<HandleState*> uniq;
        for (auto it = g_name_to_state.begin(); it != g_name_to_state.end(); ) {
            HandleState* st = it->second;
            if (st && st->isUsbHid) {
                uniq.insert(st);
                it = g_name_to_state.erase(it);
            } else {
                ++it;
            }
        }
        to_close.assign(uniq.begin(), uniq.end());
    }
    for (auto* st : to_close) {
        if (!st) continue;
        // 通知上层该 HID 会话失效
        push_event(st, "{\"type\":\"UsbHidDisconnected\"}");
        // 停止该句柄的工作线程，防止后续操作排队
        stop_worker(st);
        // 关键：关闭底层 GClient 来注销 SDK 回调，避免旧会话的移除事件在新映射建立后误路由
        if (st->client) {
            Close(st->client);
            st->client = nullptr;
        }
        // 不删除 HandleState，避免 Dart 侧仍持有的句柄悬空；由 Dart 最终调用 greader_close 做彻底回收
    }
    if (!to_close.empty()) {
        debug_log("[GReaderDiag] closed %zu stale HID handles before opening a new one", to_close.size());
    }
}

// 删除全局抑制逻辑，改为在 open 前主动关闭旧 HID 客户端来根治残留回调

// no-op helper removed; keep file clean under /WX

// Example callbacks -> JSON events
static void __stdcall on_tag_epc_log(char* readerName, LogBaseEpcInfo msg) {
    std::string epc = reinterpret_cast<const char*>(msg.Epc);
    std::string tid = reinterpret_cast<const char*>(msg.Tid);
    HandleState* st = nullptr;
    {
        std::lock_guard<std::mutex> lk(g_map_mtx);
        auto it = g_name_to_state.find(readerName ? std::string(readerName) : std::string());
        if (it != g_name_to_state.end()) st = it->second;
    }
    if (!st) return;
    char buf[4096];
    std::snprintf(buf, sizeof(buf),
        "{\"type\":\"TagEpcLog\",\"epc\":\"%s\",\"tid\":\"%s\",\"ant\":%d,\"rssi\":%d}",
        epc.c_str(), tid.c_str(), (int)msg.AntId, (int)msg.Rssi);
    push_event(st, std::string(buf));
}

static void __stdcall on_tag_epc_over(char* readerName, LogBaseEpcOver /*msg*/) {
    HandleState* st = nullptr;
    {
        std::lock_guard<std::mutex> lk(g_map_mtx);
        auto it = g_name_to_state.find(readerName ? std::string(readerName) : std::string());
        if (it != g_name_to_state.end()) st = it->second;
    }
    if (!st) return;
    push_event(st, "{\"type\":\"TagEpcOver\"}");
}

static void __stdcall on_tcp_disconnected(char* readerName) {
    HandleState* st = nullptr;
    {
        std::lock_guard<std::mutex> lk(g_map_mtx);
        auto it = g_name_to_state.find(readerName ? std::string(readerName) : std::string());
        if (it != g_name_to_state.end()) st = it->second;
    }
    if (!st) return;
    push_event(st, "{\"type\":\"TcpDisconnected\"}");
}

static void __stdcall on_usb_hid_removed(char* readerName) {
    HandleState* st = nullptr;
    {
        std::lock_guard<std::mutex> lk(g_map_mtx);
        auto it = g_name_to_state.find(readerName ? std::string(readerName) : std::string());
        if (it != g_name_to_state.end()) st = it->second;
    }
    if (!st) return; // Ignore removals for unknown/untracked devices to avoid noisy logs
    // If the OS still reports this HID path as present, treat as spurious and ignore.
    // 为了对抗瞬时重枚举，将进行多次确认：若任一采样发现仍在，则忽略本次回调。
    {
        const int samples = 3;
        const int interval_ms = 150; // 总计 ~300ms 延迟，换取更高可靠性
        for (int i = 0; i < samples; ++i) {
            if (hid_path_present_acp(readerName)) {
                debug_log("[GReaderDiag] ignore UsbHidRemoved because device still present (sample %d/%d): %s", i+1, samples, readerName ? readerName : "");
                return;
            }
            if (i + 1 < samples) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            }
        }
    }
    // Guard: ignore spurious removal callbacks that may fire immediately after open
    // Some stacks briefly toggle interfaces on first registration; treat removals within
    // a short grace window as noise.
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - st->openedAt).count();
        if (elapsed_ms >= 0 && elapsed_ms < 3000) {
            debug_log("[GReaderDiag] ignore early UsbHidRemoved (elapsed=%d ms)", (int)elapsed_ms);
            return;
        }
    }
    // De-duplicate repeated callbacks from vendor SDK
    if (st->usbHidRemovedReported.exchange(true)) {
        return;
    }
    // Emit both per-handle event and a single global diagnostic (with readerName) for visibility
    push_event(st, "{\"type\":\"UsbHidRemoved\"}");
    {
        const char* rn = (readerName && *readerName) ? readerName : "";
        char j[256];
        std::snprintf(j, sizeof(j), "{\"type\":\"UsbHidRemoved\",\"readerName\":\"%s\"}", rn);
        push_diag(j);
    }
}

GREADER_API GClientHandle greader_open_rs232(const char* conn_str, int timeout) {
    auto fut = std::async(std::launch::async, [conn_str, timeout]() -> GClient* {
        return OpenRS232(conn_str, timeout);
    });
    const auto wait_ms = (timeout > 0 ? timeout * 1000 : 3000) + 1500;
    if (fut.wait_for(std::chrono::milliseconds(wait_ms)) == std::future_status::ready) {
        GClient* client = fut.get();
        auto h = from_client(client);
        if (h) {
            auto* st = to_state(h);
            map_register(st, conn_str);
            st->transport = "rs232";
            start_worker(st);
            push_event(st, std::string("{\"type\":\"Connected\"}"));
        }
        return h;
    } else {
        // Detach the async task to avoid blocking on future destructor.
        std::thread([f = std::move(fut)]() mutable {
            try { (void)f.get(); } catch (...) {}
        }).detach();
        return nullptr;
    }
}

GREADER_API GClientHandle greader_open_tcpclient(const char* conn_str, int timeout) {
    auto fut = std::async(std::launch::async, [conn_str, timeout]() -> GClient* {
        return OpenTcpClient(conn_str, timeout);
    });
    const auto wait_ms = (timeout > 0 ? timeout * 1000 : 3000) + 1500;
    if (fut.wait_for(std::chrono::milliseconds(wait_ms)) == std::future_status::ready) {
        GClient* client = fut.get();
        auto h = from_client(client);
        if (h) {
            auto* st = to_state(h);
            map_register(st, conn_str);
            st->transport = "tcp";
            start_worker(st);
            // Optional: give device a brief settle time to be ready for immediate writes
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            push_event(st, std::string("{\"type\":\"Connected\"}"));
        }
        return h;
    } else {
        std::thread([f = std::move(fut)]() mutable {
            try { (void)f.get(); } catch (...) {}
        }).detach();
        return nullptr;
    }
}

GREADER_API GClientHandle greader_open_rs485(const char* conn_str, int timeout) {
    auto fut = std::async(std::launch::async, [conn_str, timeout]() -> GClient* {
        return OpenRS485(conn_str, timeout);
    });
    const auto wait_ms = (timeout > 0 ? timeout * 1000 : 3000) + 1500;
    if (fut.wait_for(std::chrono::milliseconds(wait_ms)) == std::future_status::ready) {
        GClient* client = fut.get();
        auto h = from_client(client);
        if (h) {
            auto* st = to_state(h);
            map_register(st, conn_str);
            st->transport = "rs485";
            start_worker(st);
            push_event(st, std::string("{\"type\":\"Connected\"}"));
        }
        return h;
    } else {
        std::thread([f = std::move(fut)]() mutable {
            try { (void)f.get(); } catch (...) {}
        }).detach();
        return nullptr;
    }
}

GREADER_API GClientHandle greader_open_usbhid(const char* device_path, int timeout) {
    // Emit a quick-start diagnostic so UI can show something immediately
    {
        char j[256];
        std::snprintf(j, sizeof(j),
            "{\"type\":\"HidOpenStart\",\"timeout\":%d,\"pathProvided\":%s}",
            timeout, (device_path && *device_path) ? "true" : "false");
        push_diag(j);
    }

    // Best-effort: unregister any stale HID routes to prevent ghost EUsbHidRemoved routed to new sessions
    close_all_hid_handles();

    auto utf8_to_acp = [](const char* s) -> std::string {
        if (!s || !*s) return std::string();
#ifdef _WIN32
        int wlen = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
        if (wlen <= 0) return std::string(s);
        std::wstring wpath;
        wpath.resize(wlen);
        MultiByteToWideChar(CP_UTF8, 0, s, -1, &wpath[0], wlen);
        int alen = WideCharToMultiByte(CP_ACP, 0, wpath.c_str(), -1, NULL, 0, NULL, NULL);
        if (alen <= 0) return std::string(s);
        std::string acp;
        acp.resize(alen);
        WideCharToMultiByte(CP_ACP, 0, wpath.c_str(), -1, &acp[0], alen, NULL, NULL);
        return acp;
#else
        return std::string(s);
#endif
    };

    auto enum_hid_paths_acp = []() -> std::vector<std::string> {
        std::vector<std::string> out;
#ifdef _WIN32
        GUID hidGuid;
        HidD_GetHidGuid(&hidGuid);
        HDEVINFO devInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if (devInfo == INVALID_HANDLE_VALUE) return out;
        SP_DEVICE_INTERFACE_DATA ifaceData;
        ifaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        for (DWORD index = 0; SetupDiEnumDeviceInterfaces(devInfo, NULL, &hidGuid, index, &ifaceData); ++index) {
            DWORD requiredSize = 0;
            SetupDiGetDeviceInterfaceDetail(devInfo, &ifaceData, NULL, 0, &requiredSize, NULL);
            if (requiredSize == 0) continue;
            std::vector<char> buffer(requiredSize);
            auto detail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(buffer.data());
            detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            if (SetupDiGetDeviceInterfaceDetail(devInfo, &ifaceData, detail, requiredSize, NULL, NULL)) {
#ifdef UNICODE
                int wlen = (int)wcslen(detail->DevicePath);
                int alen = WideCharToMultiByte(CP_ACP, 0, detail->DevicePath, wlen, NULL, 0, NULL, NULL);
                if (alen > 0) {
                    std::string acp(alen, '\0');
                    WideCharToMultiByte(CP_ACP, 0, detail->DevicePath, wlen, &acp[0], alen, NULL, NULL);
                    out.push_back(std::move(acp));
                }
#else
                out.emplace_back(detail->DevicePath);
#endif
            }
        }
        SetupDiDestroyDeviceInfoList(devInfo);
#endif
        return out;
    };

    // Build candidate list: vendor-first to match UsbHid.cpp, then provided path (ACP, raw), then SetupAPI
    std::vector<std::string> candidates;
    std::unordered_map<std::string, std::string> origin; // path -> origin label
    std::set<std::string> seen;
    // 1) Vendor paths
    try {
        std::vector<std::string> vpaths;
        int n = GetAttachedUsbHid(&vpaths);
        if (n > 0) {
            for (auto& vp : vpaths) {
                if (!seen.count(vp)) { candidates.push_back(vp); seen.insert(vp); origin.try_emplace(vp, "vendor"); }
            }
            // Emit vendor list count for parity
            push_diag("{\"type\":\"HidVendorList\",\"count\":" + std::to_string((int)vpaths.size()) + "}");
        }
    } catch (...) {
        // ignore
    }
    // 2) Provided path
    if (device_path && *device_path) {
        auto a = utf8_to_acp(device_path);
        if (!a.empty() && !seen.count(a)) { candidates.push_back(a); seen.insert(a); origin.try_emplace(a, "input-acp"); }
        if (!seen.count(device_path)) { candidates.emplace_back(device_path); seen.insert(device_path); origin.try_emplace(std::string(device_path), "input-raw"); }
    }
    // 3) SetupAPI ACP paths
    auto all = enum_hid_paths_acp();
    for (auto& p : all) {
        if (!seen.count(p)) { candidates.push_back(p); seen.insert(p); origin.try_emplace(p, "setupapi"); }
    }

    int total = timeout > 0 ? timeout : 3;
    int per = total;
    if (per < 1) per = 1;
    if (per > 3) per = 3;
    bool hadVendor = false, hadSetup = false, hadInputAcp = false, hadInputRaw = false;
    for (auto& kv : origin) {
        if (kv.second == "vendor") hadVendor = true;
        else if (kv.second == "setupapi") hadSetup = true;
        else if (kv.second == "input-acp") hadInputAcp = true;
        else if (kv.second == "input-raw") hadInputRaw = true;
    }
    debug_log("[GReaderDiag] HID open(start same-thread): timeout=%d, perTry=%d, candidates=%zu (vendor=%d, setup=%d, inputAcp=%d, inputRaw=%d)",
              timeout, per, candidates.size(), hadVendor, hadSetup, hadInputAcp, hadInputRaw);

    size_t idx = 0;
    for (auto& c : candidates) {
        const char* src = "unknown";
        auto it = origin.find(c);
        if (it != origin.end()) src = it->second.c_str();
        char tj[192];
        std::snprintf(tj, sizeof(tj),
            "{\"type\":\"HidOpenTry\",\"idx\":%zu,\"src\":\"%s\"}", idx, src);
        push_diag(tj);
        debug_log("[GReaderDiag] HID open attempt #%zu src=%s", idx, src);
    GClient* g = OpenUSBHID(c.c_str(), per);
    if (g) {
            debug_log("[GReaderDiag] HID open success at idx=%zu src=%s", idx, src);
            char j[256];
            std::snprintf(j, sizeof(j),
                          "{\"type\":\"HidOpenDiag\",\"tried\":%zu,\"perSec\":%d,\"succ\":true,\"succIdx\":%zu,\"hadVendor\":%s,\"hadSetup\":%s,\"inputAcp\":%s,\"inputRaw\":%s}",
                          candidates.size(), per, idx,
                          hadVendor?"true":"false", hadSetup?"true":"false", hadInputAcp?"true":"false", hadInputRaw?"true":"false");
            // Create handle state and map using provided name for callback routing
            auto h = from_client(g);
            if (h) {
                auto* st = to_state(h);
        // Important: callbacks provide the same readerName passed into OpenUSBHID.
        // We must register routing key with the exact string used for OpenUSBHID (candidate 'c').
        // Using the original device_path (UTF-8) may not match ACP candidate 'c', causing event loss.
    map_register(st, c.c_str());
    st->isUsbHid = true;
                st->transport = "usbhid";
                start_worker(st);
                // Register callbacks immediately to avoid races before Dart registers
                RegCallBack(st->client, ETagEpcLog, (void*)on_tag_epc_log);
                RegCallBack(st->client, ETagEpcOver, (void*)on_tag_epc_over);
                RegCallBack(st->client, ETcpDisconnected, (void*)on_tcp_disconnected);
                RegCallBack(st->client, EUsbHidRemoved, (void*)on_usb_hid_removed);
                push_event(st, "{\"type\":\"CallbacksRegistered\"}");
                // also push summary into the handle's queue for UI association
                push_event(st, std::string(j));
                // Lifecycle connected
                push_event(st, std::string("{\"type\":\"Connected\"}"));
            }
            // Also push to global diag for visibility
            push_diag(j);
        // Give device a longer settle time before first write
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
            return h;
        }
        ++idx;
    }
    debug_log("[GReaderDiag] HID open failed after %zu attempts", candidates.size());
    char j[256];
    std::snprintf(j, sizeof(j),
                  "{\"type\":\"HidOpenDiag\",\"tried\":%zu,\"perSec\":%d,\"succ\":false,\"succIdx\":-1,\"hadVendor\":%s,\"hadSetup\":%s,\"inputAcp\":%s,\"inputRaw\":%s}",
                  candidates.size(), per,
                  hadVendor?"true":"false", hadSetup?"true":"false", hadInputAcp?"true":"false", hadInputRaw?"true":"false");
    push_diag(j);
    return nullptr;
}

GREADER_API const char* greader_get_attached_usbhid_list(int* out_len) {
#ifdef _WIN32
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO devInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devInfo == INVALID_HANDLE_VALUE) return nullptr;
    std::vector<std::string> paths;

    SP_DEVICE_INTERFACE_DATA ifaceData;
    ifaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    for (DWORD index = 0; SetupDiEnumDeviceInterfaces(devInfo, NULL, &hidGuid, index, &ifaceData); ++index) {
        // First get required buffer size
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetail(devInfo, &ifaceData, NULL, 0, &requiredSize, NULL);
        if (requiredSize == 0) continue;
        std::vector<char> buffer(requiredSize);
        auto detail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(buffer.data());
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (SetupDiGetDeviceInterfaceDetail(devInfo, &ifaceData, detail, requiredSize, NULL, NULL)) {
            // detail->DevicePath is TCHAR*, with UNICODE builds it's WCHAR*
#ifdef UNICODE
            // Convert wide-char to UTF-8
            int wlen = (int)wcslen(detail->DevicePath);
            int len = WideCharToMultiByte(CP_UTF8, 0, detail->DevicePath, wlen, NULL, 0, NULL, NULL);
            if (len > 0) {
                std::string utf8(len, '\0');
                WideCharToMultiByte(CP_UTF8, 0, detail->DevicePath, wlen, utf8.data(), len, NULL, NULL);
                paths.push_back(std::move(utf8));
            }
#else
            paths.emplace_back(detail->DevicePath);
#endif
        }
    }
    SetupDiDestroyDeviceInfoList(devInfo);

    debug_log("[GReaderDiag] HID list count=%zu", paths.size());
    for (size_t i = 0; i < paths.size(); ++i) {
        // Only show prefix to avoid overly long lines
        const std::string& p = paths[i];
        std::string head = p.substr(0, p.size() > 80 ? 80 : p.size());
        debug_log("[GReaderDiag] HID #%zu path(head)=%s", i, head.c_str());
    }
    {
        char j[128];
        std::snprintf(j, sizeof(j), "{\"type\":\"HidList\",\"count\":%zu}", paths.size());
        push_diag(j);
    }

    std::string joined;
    for (size_t i = 0; i < paths.size(); ++i) {
        joined += paths[i];
        if (i + 1 < paths.size()) joined += "\n";
    }
    char* buf = (char*)::malloc(joined.size() + 1);
    if (!buf) return nullptr;
    std::memcpy(buf, joined.c_str(), joined.size() + 1);
    if (out_len) *out_len = (int)joined.size();
    return buf;
#else
    (void)out_len;
    return nullptr;
#endif
}

GREADER_API void greader_close(GClientHandle handle) {
    if (!handle) return;
    auto* st = to_state(handle);
    // Emit a synthetic disconnect event for UI symmetry
    if (st) {
        if (st->isUsbHid) {
            push_event(st, "{\"type\":\"UsbHidDisconnected\"}");
        } else {
            push_event(st, "{\"type\":\"Disconnected\"}");
        }
    }
    map_unregister(st);
    stop_worker(st);
    if (st->client) {
        Close(st->client);
        st->client = nullptr;
    }
    delete st;
}

GREADER_API int greader_get_status_json(GClientHandle handle, const char** out_cstr, int* out_len) {
    if (!handle || !out_cstr || !out_len) return 0;
    auto* st = to_state(handle);
    bool connected = st && st->client != nullptr;
    std::string name = (st ? st->readerName : std::string());
    std::string trans = (st ? st->transport : std::string());
    bool isHid = st ? st->isUsbHid : false;
    // Build a small JSON; we avoid querying version synchronously to keep this light.
    char buf[1024];
    auto name_u8 = json_escape(to_utf8_from_acp(name.c_str()));
    auto trans_u8 = json_escape(to_utf8_from_acp(trans.c_str()));
    std::snprintf(buf, sizeof(buf),
        "{\"connected\":%s,\"readerName\":\"%s\",\"transport\":\"%s\",\"isUsbHid\":%s}",
        connected ? "true" : "false",
        name_u8.c_str(), trans_u8.c_str(), isHid ? "true" : "false");
    size_t n = std::strlen(buf);
    char* out = (char*)::malloc(n + 1);
    if (!out) return 0;
    std::memcpy(out, buf, n + 1);
    *out_cstr = out;
    *out_len = (int)n;
    return 1;
}

GREADER_API int greader_base_stop(GClientHandle handle, char* err_buf, int err_buf_len) {
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return -1;
    return run_on_worker(st, [st, client, err_buf, err_buf_len]() -> int {
        ensure_settle_delay(st, 500);
        MsgBaseStop stop; std::memset(&stop, 0, sizeof(stop));
        debug_log("[GReaderDiag] Send BaseStop");
        SendSynMsgTimeoutRetry(client, EMESS_BaseStop, &stop, 1500, 1);
        if (stop.rst.RtCode != 0 && err_buf && err_buf_len > 0) {
            std::snprintf(err_buf, err_buf_len, "%s", stop.rst.RtMsg);
        }
        return stop.rst.RtCode;
    }, "BaseStop");
}

GREADER_API int greader_base_set_power(
    GClientHandle handle,
    int antenna_no,
    unsigned char power,
    char* err_buf,
    int err_buf_len) {
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return -1;
    return run_on_worker(st, [st, client, antenna_no, power, err_buf, err_buf_len]() -> int {
        ensure_settle_delay(st, 500);
        MsgBaseSetPower msg; std::memset(&msg, 0, sizeof(msg));
        msg.DicPower[0].AntennaNo = (unsigned short)antenna_no;
        msg.DicPower[0].Power = power;
        msg.DicCount = 1;
        debug_log("[GReaderDiag] Send BaseSetPower a=%d p=%u", (int)msg.DicPower[0].AntennaNo, (unsigned)msg.DicPower[0].Power);
        SendSynMsgTimeoutRetry(client, EMESS_BaseSetPower, &msg, 3000, 2);
        if (msg.rst.RtCode != 0 && err_buf && err_buf_len > 0) {
            std::snprintf(err_buf, err_buf_len, "%s", msg.rst.RtMsg);
        }
        return msg.rst.RtCode;
    }, "BaseSetPower");
}

GREADER_API int greader_inventory_epc_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int filter_area,
    const char* filter_hex,
    int filter_bit_start,
    int read_tid_len,
    int timeout_ms,
    char* err_buf,
    int err_buf_len) {
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return -1;
    return run_on_worker(st, [st, client, antenna_enable, inventory_mode, filter_area, filter_hex, filter_bit_start, read_tid_len, timeout_ms, err_buf, err_buf_len]() -> int {
        ensure_settle_delay(st, 500);
        MsgBaseInventoryEpc msg; std::memset(&msg, 0, sizeof(msg));
        msg.AntennaEnable = antenna_enable;
        msg.InventoryMode = (char)inventory_mode;
        if (read_tid_len > 0) {
            msg.ReadTid.Mode = (char)0;
            msg.ReadTid.Len = (unsigned char)read_tid_len;
        }
        if (timeout_ms > 0) {
            msg.Timeout = timeout_ms;
        }
        if (filter_area >= 0 && filter_hex && *filter_hex) {
            msg.Filter.Area = (char)filter_area;
            msg.Filter.BitStart = (unsigned short)filter_bit_start;
            size_t n = std::strlen(filter_hex);
            if (n > sizeof(msg.Filter.HexData) - 1) n = sizeof(msg.Filter.HexData) - 1;
            std::memcpy(msg.Filter.HexData, filter_hex, n);
            msg.Filter.HexData[n] = '\0';
            msg.Filter.BitLen = (int)n * 4;
        }
        debug_log("[GReaderDiag] Send InventoryEpc ae=%d mode=%d tidLen=%d to=%d", msg.AntennaEnable, (int)msg.InventoryMode, (int)msg.ReadTid.Len, (int)msg.Timeout);
        SendSynMsgTimeoutRetry(client, EMESS_BaseInventoryEpc, &msg, 3000, 2);
        if (msg.rst.RtCode != 0 && err_buf && err_buf_len > 0) {
            std::snprintf(err_buf, err_buf_len, "%s", msg.rst.RtMsg);
        }
        return msg.rst.RtCode;
    }, "InventoryEpc");
}

GREADER_API int greader_inventory_gb_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int /*timeout_ms*/,
    char* err_buf,
    int err_buf_len) {
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return -1;
    return run_on_worker(st, [client, antenna_enable, inventory_mode, err_buf, err_buf_len]() -> int {
        MsgBaseInventoryGB msg; std::memset(&msg, 0, sizeof(msg));
        msg.AntennaEnable = antenna_enable;
        msg.InventoryMode = (char)inventory_mode;
        SendSynMsg(client, EMESS_BaseInventoryGb, &msg);
        if (msg.rst.RtCode != 0 && err_buf && err_buf_len > 0) {
            std::snprintf(err_buf, err_buf_len, "%s", msg.rst.RtMsg);
        }
        return msg.rst.RtCode;
    }, "InventoryGb");
}

GREADER_API int greader_inventory_gjb_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int /*timeout_ms*/,
    char* err_buf,
    int err_buf_len) {
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return -1;
    return run_on_worker(st, [client, antenna_enable, inventory_mode, err_buf, err_buf_len]() -> int {
        MsgBaseInventoryGJB msg; std::memset(&msg, 0, sizeof(msg));
        msg.AntennaEnable = antenna_enable;
        msg.InventoryMode = (char)inventory_mode;
        SendSynMsg(client, EMESS_BaseInventoryGjb, &msg);
        if (msg.rst.RtCode != 0 && err_buf && err_buf_len > 0) {
            std::snprintf(err_buf, err_buf_len, "%s", msg.rst.RtMsg);
        }
        return msg.rst.RtCode;
    }, "InventoryGjb");
}

GREADER_API int greader_inventory_tl_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int /*timeout_ms*/,
    char* err_buf,
    int err_buf_len) {
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return -1;
    return run_on_worker(st, [client, antenna_enable, inventory_mode, err_buf, err_buf_len]() -> int {
        MsgBaseInventoryTL msg; std::memset(&msg, 0, sizeof(msg));
        msg.AntennaEnable = antenna_enable;
        msg.InventoryMode = (char)inventory_mode;
        SendSynMsg(client, EMESS_BaseInventoryTL, &msg);
        if (msg.rst.RtCode != 0 && err_buf && err_buf_len > 0) {
            std::snprintf(err_buf, err_buf_len, "%s", msg.rst.RtMsg);
        }
        return msg.rst.RtCode;
    }, "InventoryTl");
}

GREADER_API int greader_write_epc(
    GClientHandle handle,
    int antenna_enable,
    int area,
    unsigned short start,
    const char* hex_data,
    const char* password,
    int block,
    int stay_cw,
    int filter_area,
    const char* filter_hex,
    int filter_bit_start,
    char* err_buf,
    int err_buf_len) {
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return -1;
    return run_on_worker(st, [st, client, antenna_enable, area, start, hex_data, password, block, stay_cw, filter_area, filter_hex, filter_bit_start, err_buf, err_buf_len]() -> int {
        ensure_settle_delay(st, 500);
        MsgBaseWriteEpc msg; std::memset(&msg, 0, sizeof(msg));
        msg.AntennaEnable = antenna_enable;
        msg.Area = (char)area;
        msg.Start = start;
        const size_t epc_len = (hex_data ? std::strlen(hex_data) : 0);
        const size_t filter_len = (filter_hex ? std::strlen(filter_hex) : 0);
        debug_log("[GReaderDiag] Send WriteEpc ae=%d area=%d start=%u hexLen=%zu block=%d stay=%d filterArea=%d filterLen=%zu bitStart=%d",
                  (int)msg.AntennaEnable, (int)msg.Area, (unsigned)msg.Start, epc_len,
                  (int)block, (int)stay_cw, (int)filter_area, filter_len, (int)filter_bit_start);
        if (hex_data) {
            size_t n = epc_len;
            if (n > sizeof(msg.StrHexWriteData) - 1) n = sizeof(msg.StrHexWriteData) - 1;
            std::memcpy(msg.StrHexWriteData, hex_data, n);
            msg.StrHexWriteData[n] = '\0';
        }
        if (password) {
            size_t n = std::strlen(password);
            if (n > sizeof(msg.StrHexPassword) - 1) n = sizeof(msg.StrHexPassword) - 1;
            std::memcpy(msg.StrHexPassword, password, n);
            msg.StrHexPassword[n] = '\0';
        }
        msg.Block = block;
        msg.StayCarrierWave = stay_cw;
        if (filter_area >= 0 && filter_hex && *filter_hex) {
            msg.Filter.Area = (char)filter_area;
            msg.Filter.BitStart = (unsigned short)filter_bit_start;
            size_t n = filter_len;
            if (n > sizeof(msg.Filter.HexData) - 1) n = sizeof(msg.Filter.HexData) - 1;
            std::memcpy(msg.Filter.HexData, filter_hex, n);
            msg.Filter.HexData[n] = '\0';
            msg.Filter.BitLen = (int)n * 4;
        }
        SendSynMsgTimeoutRetry(client, EMESS_BaseWriteEpc, &msg, 3000, 2);
        debug_log("[GReaderDiag] WriteEpc result code=%d", (int)msg.rst.RtCode);
        if (msg.rst.RtCode != 0 && err_buf && err_buf_len > 0) {
            std::snprintf(err_buf, err_buf_len, "%s", msg.rst.RtMsg);
        }
        return msg.rst.RtCode;
    }, "WriteEpc");
}

GREADER_API int greader_lock_epc(
    GClientHandle handle,
    int antenna_enable,
    int area,
    int mode,
    const char* password,
    int filter_area,
    const char* filter_hex,
    int filter_bit_start,
    char* err_buf,
    int err_buf_len) {
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return -1;
    return run_on_worker(st, [st, client, antenna_enable, area, mode, password, filter_area, filter_hex, filter_bit_start, err_buf, err_buf_len]() -> int {
        ensure_settle_delay(st, 300);
        MsgBaseLockEpc msg; std::memset(&msg, 0, sizeof(msg));
        msg.AntennaEnable = antenna_enable;
        msg.Area = (char)area;
        msg.Mode = (char)mode;
        if (password) {
            size_t n = std::strlen(password);
            if (n > sizeof(msg.StrHexPassword) - 1) n = sizeof(msg.StrHexPassword) - 1;
            std::memcpy(msg.StrHexPassword, password, n);
            msg.StrHexPassword[n] = '\0';
        }
        if (filter_area >= 0 && filter_hex && *filter_hex) {
            msg.Filter.Area = (char)filter_area;
            msg.Filter.BitStart = (unsigned short)filter_bit_start;
            size_t n = std::strlen(filter_hex);
            if (n > sizeof(msg.Filter.HexData) - 1) n = sizeof(msg.Filter.HexData) - 1;
            std::memcpy(msg.Filter.HexData, filter_hex, n);
            msg.Filter.HexData[n] = '\0';
            msg.Filter.BitLen = (int)n * 4;
        }
        SendSynMsgTimeoutRetry(client, EMESS_BaseLockEpc, &msg, 3000, 2);
        if (msg.rst.RtCode != 0 && err_buf && err_buf_len > 0) {
            std::snprintf(err_buf, err_buf_len, "%s", msg.rst.RtMsg);
        }
        return msg.rst.RtCode;
    }, "LockEpc");
}

GREADER_API void greader_register_default_callbacks(GClientHandle handle) {
    GClient* client = to_client(handle);
    if (!client) return;
    RegCallBack(client, ETagEpcLog, (void*)on_tag_epc_log);
    RegCallBack(client, ETagEpcOver, (void*)on_tag_epc_over);
    RegCallBack(client, ETcpDisconnected, (void*)on_tcp_disconnected);
    RegCallBack(client, EUsbHidRemoved, (void*)on_usb_hid_removed);
}

GREADER_API int greader_events_next_json(GClientHandle handle, const char** out_cstr, int* out_len) {
    if (!handle || !out_cstr || !out_len) return 0;
    auto* st = to_state(handle);
    std::lock_guard<std::mutex> lock(st->events->mtx);
    if (st->events->q.empty()) return 0;
    const std::string& top = st->events->q.front();
    char* buf = (char*)::malloc(top.size() + 1);
    if (!buf) return 0;
    std::memcpy(buf, top.c_str(), top.size() + 1);
    *out_cstr = buf;
    *out_len = (int)top.size();
    st->events->q.pop();
    return 1;
}

GREADER_API void greader_free_cstr(const char* p) {
    if (p) ::free((void*)p);
}

GREADER_API int greader_diag_next_json(const char** out_cstr, int* out_len) {
    if (!out_cstr || !out_len) return 0;
    std::lock_guard<std::mutex> lock(g_diag_queue.mtx);
    if (g_diag_queue.q.empty()) return 0;
    const std::string& top = g_diag_queue.q.front();
    char* buf = (char*)::malloc(top.size() + 1);
    if (!buf) return 0;
    std::memcpy(buf, top.c_str(), top.size() + 1);
    *out_cstr = buf;
    *out_len = (int)top.size();
    g_diag_queue.q.pop();
    return 1;
}

GREADER_API void greader_diag_emit(const char* json_utf8) {
    if (!json_utf8) return;
    push_diag(json_utf8);
}

GREADER_API int greader_get_realtime_json(GClientHandle handle, const char** out_cstr, int* out_len) {
    if (!handle || !out_cstr || !out_len) return 0;
    auto* st = to_state(handle);
    GClient* client = to_client(handle);
    if (!client) return 0;
    // 在工作线程上串行执行，避免与其他命令并发
    int rc = run_on_worker(st, [st, client, out_cstr, out_len]() -> int {
        ensure_settle_delay(st, 300);
    // 读取器能力
    MsgBaseGetCapabilities caps; std::memset(&caps, 0, sizeof(caps));
        SendSynMsgTimeoutRetry(client, EMESS_BaseGetCapabilities, &caps, 1500, 1);
        // 功率（简化：查询读功率）
        MsgBaseGetPower pwr; std::memset(&pwr, 0, sizeof(pwr));
    pwr.setReadOrWrite = true; // 0=read, 1=write
    pwr.ReadOrWrite = 0;
        SendSynMsgTimeoutRetry(client, EMESS_BaseGetPower, &pwr, 1500, 1);
        // 频段
        MsgBaseGetFreqRange fr; std::memset(&fr, 0, sizeof(fr));
        SendSynMsgTimeoutRetry(client, EMESS_BaseGetFreqRange, &fr, 1500, 1);
        // 基带
        MsgBaseGetBaseband bb; std::memset(&bb, 0, sizeof(bb));
        SendSynMsgTimeoutRetry(client, EMESS_BaseGetBaseband, &bb, 1500, 1);
        // GPI 状态
        MsgAppGetGpiState gpi; std::memset(&gpi, 0, sizeof(gpi));
        SendSynMsgTimeoutRetry(client, EMESS_AppGetGpiState, &gpi, 1500, 1);
        // 读写器信息（上电时间等）
        MsgAppGetReaderInfo info; std::memset(&info, 0, sizeof(info));
        SendSynMsgTimeoutRetry(client, EMESS_AppGetReaderInfo, &info, 1500, 1);

        // 事件队列长度（近似）
        int evtq = 0; {
            std::lock_guard<std::mutex> lk(st->events->mtx);
            evtq = (int)st->events->q.size();
        }

    // 组装简单 JSON（为稳妥避免转义复杂性，逐字段拼接）
        std::string json = "{";
    json += "\"connected\":" + std::string(st->client?"true":"false");
    auto rn_u8 = json_escape(to_utf8_from_acp(st->readerName.c_str()));
    auto tp_u8 = json_escape(to_utf8_from_acp(st->transport.c_str()));
    json += ",\"readerName\":\"" + rn_u8 + "\"";
    json += ",\"transport\":\"" + tp_u8 + "\"";
        json += ",\"isUsbHid\":" + std::string(st->isUsbHid?"true":"false");
        // 能力
        json += ",\"capabilities\":{";
        json += "\"maxPower\":" + std::to_string((int)caps.MaxPower);
        json += ",\"minPower\":" + std::to_string((int)caps.MinPower);
        json += ",\"antennaCount\":" + std::to_string((int)caps.AntennaCount);
        json += "}";
        // 功率（仅取 DicPower[0..DicCount-1]）
        json += ",\"power\":[";
        for (int i=0;i<(int)pwr.DicCount;i++){
            if (i) json += ",";
            json += "{";
            json += "\"antenna\":" + std::to_string((int)pwr.DicPower[i].AntennaNo);
            json += ",\"read\":" + std::to_string((int)pwr.DicPower[i].Power);
            json += "}";
        }
        json += "]";
        // 频段索引
        json += ",\"freqRangeIndex\":" + std::to_string((int)fr.FreqRangeIndex);
        // 基带（只列出关键项）
        json += ",\"baseband\":{";
        json += "\"baseSpeed\":" + std::to_string((int)bb.BaseSpeed);
        json += ",\"qValue\":" + std::to_string((int)bb.QValue);
        json += ",\"session\":" + std::to_string((int)bb.Session);
        json += ",\"inventoryFlag\":" + std::to_string((int)bb.InventoryFlag);
        json += "}";
        // GPI 状态
        json += ",\"gpi\":[";
        for (int i=0;i<(int)gpi.DicGpiCount;i++){
            if (i) json += ",";
            json += "{";
            json += "\"port\":" + std::to_string((int)gpi.DicGpi[i].GpiIndex);
            json += ",\"level\":" + std::to_string((int)gpi.DicGpi[i].Gpi);
            json += "}";
        }
        json += "]";
        // 上电时间/应用版本
    json += ",\"readerInfo\":{";
    auto serial_u8 = json_escape(to_utf8_from_acp(info.SerialNum));
    auto appver_u8 = json_escape(to_utf8_from_acp(info.AppVersion));
    auto poweron_u8 = json_escape(to_utf8_from_acp(info.PowerOnTime));
    json += "\"serial\":\""; json += serial_u8; json += "\"";
    json += ",\"appVersion\":\""; json += appver_u8; json += "\"";
    json += ",\"powerOnTime\":\""; json += poweron_u8; json += "\"";
        json += "}";
        // 事件队列长度
        json += ",\"pendingEvents\":" + std::to_string(evtq);
        json += "}";

        char* buf = (char*)::malloc(json.size() + 1);
        if (!buf) return 0;
        std::memcpy(buf, json.c_str(), json.size() + 1);
        *out_cstr = buf;
        *out_len = (int)json.size();
        return 1;
    }, "RealtimeSnapshot");
    return rc;
}
