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
 * @param conn_str The connection string, e.g., "COM12:115200".
 * @param timeout The communication timeout in seconds.
 * @return A handle to the client instance, or NULL on failure.
 */
GREADER_API GClientHandle greader_open_rs232(const char* conn_str, int timeout);

/**
 * @brief Opens a connection to the reader via TCP client.
 *
 * @param conn_str The connection string, e.g., "192.168.1.168:8160".
 * @param timeout The communication timeout in seconds.
 * @return A handle to the client instance, or NULL on failure.
 */
GREADER_API GClientHandle greader_open_tcpclient(const char* conn_str, int timeout);

/** Open reader via RS485 (e.g., "COM3:115200", timeout seconds). */
GREADER_API GClientHandle greader_open_rs485(const char* conn_str, int timeout);

/** Open reader via USB HID by device path (from enumeration). */
GREADER_API GClientHandle greader_open_usbhid(const char* device_path, int timeout);

/** Enumerate attached USB HID devices; returns a newly-allocated UTF-8 string
 * with device paths separated by '\n'. The length in bytes is written to out_len.
 * The returned memory must be freed by greader_free_cstr().
 */
GREADER_API const char* greader_get_attached_usbhid_list(int* out_len);

/**
 * @brief Closes the connection to the reader.
 * 
 * @param handle The handle returned by greader_open_rs232.
 */
GREADER_API void greader_close(GClientHandle handle);

/**
 * Execute BaseStop synchronously. Returns RtCode (0 on success).
 * If err_buf is provided, native error message (UTF-8) is written (null-terminated).
 */
GREADER_API int greader_base_stop(GClientHandle handle, char* err_buf, int err_buf_len);

/**
 * Execute BaseSetPower for a single antenna. Returns RtCode (0 on success).
 * Only DicPower[0] and DicCount=1 are set (like SDK samples).
 */
GREADER_API int greader_base_set_power(
    GClientHandle handle,
    int antenna_no,
    unsigned char power,
    char* err_buf,
    int err_buf_len);

/**
 * Start EPC inventory (registers callbacks internally and sends InventoryEpc once).
 * antenna_enable: bitmask of AntennaNo_*
 * inventory_mode: 0 single, 1 inventory
 * filter_area: -1 means no filter, else 0=RES,1=EPC,2=TID,3=USR
 * filter_hex: optional hex string (nullable/empty -> no filter)
 * filter_bit_start: bit offset (e.g., 0)
 * read_tid_len: 0 for none, else words count to read
 * timeout_ms: 0 for SDK default; else set MsgBaseInventoryEpc.Timeout
 * Returns RtCode (0 on success). Error text to err_buf if provided.
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

/** Start GB/GJB/TL inventory (similar semantics to EPC version). */
GREADER_API int greader_inventory_gb_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int timeout_ms,
    char* err_buf,
    int err_buf_len);

GREADER_API int greader_inventory_gjb_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int timeout_ms,
    char* err_buf,
    int err_buf_len);

GREADER_API int greader_inventory_tl_start(
    GClientHandle handle,
    int antenna_enable,
    int inventory_mode,
    int timeout_ms,
    char* err_buf,
    int err_buf_len);

/** Write EPC simplified wrapper. Returns RtCode. */
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

/** Lock EPC simplified wrapper. Returns RtCode. */
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

/** Register default callbacks to push events into queue (idempotent). */
GREADER_API void greader_register_default_callbacks(GClientHandle handle);

/** Pop next event as JSON string (allocated). Returns 1 if an event is returned; 0 if queue empty. */
GREADER_API int greader_events_next_json(GClientHandle handle, const char** out_cstr, int* out_len);

/** Free a C string allocated by this library. */
GREADER_API void greader_free_cstr(const char* p);

/** Enable or disable verbose native diagnostics (OutputDebugString and diag events). */
GREADER_API void greader_set_verbose_logging(int enabled);

/** Pop next global diagnostic event as JSON (not bound to any handle). */
GREADER_API int greader_diag_next_json(const char** out_cstr, int* out_len);
/** Emit a diagnostic JSON string into the global queue (for testing). */
GREADER_API void greader_diag_emit(const char* json_utf8);

// We will add more functions like writing EPC data later.

#ifdef __cplusplus
}
#endif

#endif // GREADER_API_H
