#ifndef GREADER_API_H
#define GREADER_API_H

// Standard C++ export macros
#if defined(_WIN32)
    #if defined(GREADER_API_EXPORTS)
        #define GREADER_API __declspec(dllexport)
    #else
        #define GREADER_API __declspec(dllimport)
    #endif
#else
    #define GREADER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// We use a void* as an opaque handle to the GClient C++ object,
// as Dart FFI cannot directly handle C++ objects.
typedef void* GClientHandle;

/**
 * @brief Opens a connection to the reader via RS232.
 * 
 * @param conn_str 连接字符串，例如 "COM3:115200"（串口:波特率）。
 *                 参考开发指南 3.1：OpenRS232("COMx:baud")。
 * @param timeout  连接确认超时时间（秒），例如 5。
 * @return A handle to the client instance, or NULL on failure.
 */
GREADER_API GClientHandle greader_open_rs232(const char* conn_str, int timeout);

/**
 * @brief Opens a connection to the reader via TCP client.
 *
 * @param conn_str 连接字符串，例如 "192.168.1.168:8160"（默认端口 8160）。
 *                 开发指南 3.3：OpenTcpClient("ip:port")。
 * @param timeout  连接确认超时时间（秒）。
 * @return A handle to the client instance, or NULL on failure.
 */
GREADER_API GClientHandle greader_open_tcpclient(const char* conn_str, int timeout);

/**
 * Open reader via RS485。
 * @param conn_str 连接字符串，例如 "COM3:115200:1"（串口:波特率:地址）。
 *                 开发指南 3.2：OpenRS485("COMx:baud:addr")。
 * @param timeout  连接确认超时时间（秒）。
 */
GREADER_API GClientHandle greader_open_rs485(const char* conn_str, int timeout);

/**
 * Open reader via USB HID by device path。
 * @param device_path USBHID 设备路径字符串，可用枚举接口获取；
 *                    对应开发指南 3.6：OpenUSBHID(path)。
 * @param timeout     连接确认超时时间（秒）。
 */
GREADER_API GClientHandle greader_open_usbhid(const char* device_path, int timeout);

/**
 * Enumerate attached USB HID devices.
 * @return 一个新分配的 UTF-8 字符串，包含设备路径列表，按'\n'分隔。
 * @param out_len 输出参数：返回字符串的字节长度（不含终止符）。
 * 备注：返回的内存需由 greader_free_cstr() 释放；路径可直接传给 greader_open_usbhid。
 */
GREADER_API const char* greader_get_attached_usbhid_list(int* out_len);

/**
 * @brief Closes the connection to the reader.
 * @param handle 连接句柄；NULL 时无操作。
 * 语义：释放底层连接资源；对于已失效的链接对象也需主动关闭（参见开发指南 3.7）。
 */
GREADER_API void greader_close(GClientHandle handle);

/**
 * Execute BaseStop synchronously.
 * @param handle 连接句柄。
 * @param err_buf 错误信息输出缓冲区（UTF-8，可为 NULL）。
 * @param err_buf_len 缓冲区长度（字节）。
 * @return RtCode（0 成功，非 0 失败）。
 * 说明：停止所有 RFID 操作并进入空闲（开发指南 6.2.1）。
 */
GREADER_API int greader_base_stop(GClientHandle handle, char* err_buf, int err_buf_len);

/**
 * Execute BaseSetPower for a single antenna. Returns RtCode (0 on success).
 * 语义：配置某天线的功率。
 * - antenna_no: 天线号（1~16）。
 * - power: 功率（0~33，单位按设备定义）。
 * 提示：实现仅设置 DicPower[0] 且 DicCount=1，等同 SDK 示例。
 */
GREADER_API int greader_base_set_power(
    GClientHandle handle,
    int antenna_no,
    unsigned char power,
    char* err_buf,
    int err_buf_len);

/**
/** Start EPC inventory (registers callbacks internally and sends InventoryEpc once).
 * 参数含义（参考开发指南 6.2.6/7.1）：
 * - antenna_enable: 天线位图（AntennaNo_* 的按位或），如 0x1=天线1，0x3=1+2。
 * - inventory_mode: 读取模式（0=单次；1=连续）。
 * - filter_area: 选择参数数据区（-1=不使用过滤；0=保留区；1=EPC 区；2=TID 区；3=用户区）。
 * - filter_hex: 选择参数内容（十六进制）；为空表示不使用过滤。
 * - filter_bit_start: 选择匹配起始位（匹配 EPC 时通常为 32）。
 * - read_tid_len: 读取 TID 的字数（word，0=不读；>0 指定读取长度，常用 6）。
 * - timeout_ms: 指令超时（毫秒，0=使用 SDK 默认）。
 * 返回：RtCode（0 成功）；失败原因写入 err_buf。
 */
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
    int err_buf_len);

/** Start GB inventory（开发指南 6.2.14）。
 * @param handle 连接句柄。
 * @param antenna_enable 天线位图（1=天线1）。
 * @param inventory_mode 读取模式（0=单次；1=连续）。
 * @param timeout_ms 指令超时（毫秒，0=默认）。
 * @param err_buf 错误信息输出（UTF-8，可为 NULL）。
 * @param err_buf_len 错误缓冲区长度。
 * @return RtCode（0 成功）。
 */
GREADER_API int greader_inventory_gb_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int timeout_ms,
    char* err_buf,
    int err_buf_len);

/** Start GJB inventory（开发指南 6.2.18）。
 * 参数与 greader_inventory_gb_start 相同。
 */
GREADER_API int greader_inventory_gjb_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int timeout_ms,
    char* err_buf,
    int err_buf_len);

/** Start TL inventory（国标 TL，如适用）。
 * 参数与 greader_inventory_gb_start 相同。
 */
GREADER_API int greader_inventory_tl_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int timeout_ms,
    char* err_buf,
    int err_buf_len);

/** Write EPC simplified wrapper. Returns RtCode.
 * 参数（开发指南 6.2.7）：
 * - antenna_enable: 天线位图。
 * - area: 待写数据区（0=保留；1=EPC；2=TID；3=用户）。
 * - start: 字起始地址；写 EPC 区时：1=PC，2=EPC 首字，推荐 PC+EPC 从 1 开始。
 * - hex_data: 写入内容（十六进制）。
 * - password: 访问密码（8hex，可为空串表示无密码）。
 * - block: 写入模式（0=整段写，建议；非0=分块写）。
 * - stay_cw: 写入完成后是否保持载波（0=否）。
 * - filter_area/filter_hex/filter_bit_start: 选择参数（建议按 TID 精确过滤）。
 */
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
    int err_buf_len);

/** Lock EPC simplified wrapper. Returns RtCode.
 * 参数（开发指南 6.2.8）：
 * - antenna_enable: 天线位图。
 * - area: 待锁定数据区（0=灭活密码；1=访问密码；2=EPC；3=TID；4=用户）。
 * - mode: 锁类型（0=解锁；1=锁定；2=永久解锁；3=永久锁定）。
 * - password: 访问密码（8hex，可为空）。
 * - filter_*: 选择参数（推荐按 TID 过滤）。
 */
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
    int err_buf_len);

/** Register default callbacks to push events into queue (idempotent).
 * @param handle 连接句柄；注册后将收集如下事件队列：
 *  - EPC/6B/GB/GJB 标签上报与结束（开发指南 4.x）；
 *  - 连接断开：ETcpDisconnected/EUsbHidRemoved（4.12/4.13）。
 */
GREADER_API void greader_register_default_callbacks(GClientHandle handle);

/** Pop next event as JSON string（分配内存）。
 * @param handle 连接句柄。
 * @param out_cstr 输出：事件 JSON（UTF-8，需 greader_free_cstr 释放）。
 * @param out_len 输出：字节长度。
 * @return 1=弹出成功；0=队列为空。
 */
GREADER_API int greader_events_next_json(GClientHandle handle, const char** out_cstr, int* out_len);

/** Free a C string allocated by this library.
 * @param p 由本库返回的字符串指针；可为 NULL（忽略）。
 */
GREADER_API void greader_free_cstr(const char* p);

/** Enable or disable verbose native diagnostics.
 * @param enabled 0=关闭；非 0=打开。开启后将通过 OutputDebugString 与诊断队列输出详细日志。
 */
GREADER_API void greader_set_verbose_logging(int enabled);

/** Pop next global diagnostic event as JSON（与句柄无关的全局诊断队列）。
 * @param out_cstr 输出：诊断 JSON（UTF-8，需释放）。
 * @param out_len 输出：字节长度。
 * @return 1=有事件；0=无事件。
 */
GREADER_API int greader_diag_next_json(const char** out_cstr, int* out_len);
/** Emit a diagnostic JSON string into the global queue（用于测试）。
 * @param json_utf8 UTF-8 编码的 JSON 字符串（将原样入队）。
 */
GREADER_API void greader_diag_emit(const char* json_utf8);

/**
 * Query current reader status and return as a JSON string.
 * Example JSON:
 * {"connected":true, "readerName":"COM12:115200", "transport":"rs232", "isUsbHid":false, "baseVersion":"0.1.0"}
 * Returns 1 on success and writes an allocated UTF-8 string to out_cstr and its length to out_len.
 * The caller must free the returned string with greader_free_cstr().
 */
/**
 * @param handle 连接句柄。
 * @param out_cstr 输出：UTF-8 JSON 字符串（需 greader_free_cstr 释放）。
 * @param out_len 输出：字节长度。
 */
GREADER_API int greader_get_status_json(GClientHandle handle, const char** out_cstr, int* out_len);

/**
 * Query a richer, realtime snapshot as JSON, including power map, freq range,
 * capabilities, baseband, GPI states, reader info, and pending event count.
 * 返回的 JSON 大致包含：connected/readerName/transport/isUsbHid、
 * capabilities(maxPower/minPower/antennaCount)、power(每天线功率)、
 * freqRangeIndex、baseband(baseSpeed/qValue/session/inventoryFlag)、
 * gpi(端口电平数组)、readerInfo(serial/appVersion/powerOnTime)、
 * pendingEvents。
 * Returns 1 on success, 0 on failure. Caller must free string with greader_free_cstr.
 */
/**
 * @param handle 连接句柄。
 * @param out_cstr 输出：UTF-8 JSON 字符串（需释放）。
 * @param out_len 输出：字节长度。
 */
GREADER_API int greader_get_realtime_json(GClientHandle handle, const char** out_cstr, int* out_len);

/** Query antenna count quickly.
 * 语义：通过 EMESS_BaseGetCapabilities 读取 AntennaCount。
 * @param handle 连接句柄。
 * @return >=0 天线数量；失败返回 -1。
 */
GREADER_API int greader_get_antenna_count(GClientHandle handle);

// We will add more functions like writing EPC data later.

#ifdef __cplusplus
}
#endif

#endif // GREADER_API_H
