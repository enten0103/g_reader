import 'dart:ffi' as ffi;
import 'dart:async';
import 'dart:io' show Platform;
import 'package:ffi/ffi.dart' as pkg_ffi;
import 'dart:isolate';
import 'dart:convert' as convert;

typedef _OpenRS232Native =
    ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>, ffi.Int32);
typedef _OpenRS232Dart =
    ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>, int);
typedef _OpenTcpNative =
    ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>, ffi.Int32);
typedef _OpenTcpDart =
    ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>, int);
typedef _CloseNative = ffi.Void Function(ffi.Pointer<ffi.Void>);
typedef _CloseDart = void Function(ffi.Pointer<ffi.Void>);

// callbacks / events
typedef _RegisterCbsNative = ffi.Void Function(ffi.Pointer<ffi.Void>);
typedef _RegisterCbsDart = void Function(ffi.Pointer<ffi.Void>);
typedef _EventsNextNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Void>,
      ffi.Pointer<ffi.Pointer<ffi.Int8>>,
      ffi.Pointer<ffi.Int32>,
    );
typedef _EventsNextDart =
    int Function(
      ffi.Pointer<ffi.Void>,
      ffi.Pointer<ffi.Pointer<ffi.Int8>>,
      ffi.Pointer<ffi.Int32>,
    );
typedef _FreeCStrNative = ffi.Void Function(ffi.Pointer<ffi.Int8>);
typedef _FreeCStrDart = void Function(ffi.Pointer<ffi.Int8>);

// base ops
typedef _BaseStopNative =
    ffi.Int32 Function(ffi.Pointer<ffi.Void>, ffi.Pointer<ffi.Int8>, ffi.Int32);
typedef _BaseStopDart =
    int Function(ffi.Pointer<ffi.Void>, ffi.Pointer<ffi.Int8>, int);

typedef _BaseSetPowerNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Void>,
      ffi.Int32,
      ffi.Uint8,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
    );
typedef _BaseSetPowerDart =
    int Function(ffi.Pointer<ffi.Void>, int, int, ffi.Pointer<ffi.Int8>, int);

// inventory epc
typedef _InvEpcNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Void>,
      ffi.Int32,
      ffi.Int32,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
      ffi.Int32,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
    );
typedef _InvEpcDart =
    int Function(
      ffi.Pointer<ffi.Void>,
      int,
      int,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
      int,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
    );

// RS485 / USB HID / List HID
typedef _OpenRs485Native =
    ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>, ffi.Int32);
typedef _OpenRs485Dart =
    ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>, int);
typedef _OpenUsbHidNative =
    ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>, ffi.Int32);
typedef _OpenUsbHidDart =
    ffi.Pointer<ffi.Void> Function(ffi.Pointer<ffi.Int8>, int);
typedef _ListUsbHidNative =
    ffi.Pointer<ffi.Int8> Function(ffi.Pointer<ffi.Int32>);
typedef _ListUsbHidDart =
    ffi.Pointer<ffi.Int8> Function(ffi.Pointer<ffi.Int32>);

// Inventory GB
typedef _InvGbNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Void>,
      ffi.Int32,
      ffi.Int32,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
    );
typedef _InvGbDart =
    int Function(
      ffi.Pointer<ffi.Void>,
      int,
      int,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
    );

// Inventory GJB/TL
typedef _InvGjbNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Void>,
      ffi.Int32,
      ffi.Int32,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
    );
typedef _InvGjbDart =
    int Function(
      ffi.Pointer<ffi.Void>,
      int,
      int,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
    );
typedef _InvTlNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Void>,
      ffi.Int32,
      ffi.Int32,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
    );
typedef _InvTlDart =
    int Function(
      ffi.Pointer<ffi.Void>,
      int,
      int,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
    );

// Write/Lock EPC
typedef _WriteEpcNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Void>,
      ffi.Int32,
      ffi.Int32,
      ffi.Uint16,
      ffi.Pointer<ffi.Int8>,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
      ffi.Int32,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
    );
typedef _WriteEpcDart =
    int Function(
      ffi.Pointer<ffi.Void>,
      int,
      int,
      int,
      ffi.Pointer<ffi.Int8>,
      ffi.Pointer<ffi.Int8>,
      int,
      int,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
    );
typedef _LockEpcNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Void>,
      ffi.Int32,
      ffi.Int32,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
      ffi.Pointer<ffi.Int8>,
      ffi.Int32,
    );
typedef _LockEpcDart =
    int Function(
      ffi.Pointer<ffi.Void>,
      int,
      int,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
      ffi.Pointer<ffi.Int8>,
      int,
    );
typedef _SetVerboseNative = ffi.Void Function(ffi.Int32);
typedef _SetVerboseDart = void Function(int);

typedef _DiagNextNative =
    ffi.Int32 Function(
      ffi.Pointer<ffi.Pointer<ffi.Int8>>,
      ffi.Pointer<ffi.Int32>,
    );
typedef _DiagNextDart =
    int Function(ffi.Pointer<ffi.Pointer<ffi.Int8>>, ffi.Pointer<ffi.Int32>);
typedef _DiagEmitNative = ffi.Void Function(ffi.Pointer<ffi.Int8>);
typedef _DiagEmitDart = void Function(ffi.Pointer<ffi.Int8>);

class GReaderFfi {
  GReaderFfi._();
  static final GReaderFfi instance = GReaderFfi._();

  late final ffi.DynamicLibrary _lib = _openLibrary();
  late final _OpenRS232Dart _open = _lib
      .lookupFunction<_OpenRS232Native, _OpenRS232Dart>('greader_open_rs232');
  late final _OpenTcpDart _openTcp = _lib
      .lookupFunction<_OpenTcpNative, _OpenTcpDart>('greader_open_tcpclient');
  late final _CloseDart _close = _lib.lookupFunction<_CloseNative, _CloseDart>(
    'greader_close',
  );
  late final _RegisterCbsDart _registerCallbacks = _lib
      .lookupFunction<_RegisterCbsNative, _RegisterCbsDart>(
        'greader_register_default_callbacks',
      );
  late final _EventsNextDart _eventsNext = _lib
      .lookupFunction<_EventsNextNative, _EventsNextDart>(
        'greader_events_next_json',
      );
  late final _FreeCStrDart _freeCstr = _lib
      .lookupFunction<_FreeCStrNative, _FreeCStrDart>('greader_free_cstr');
  late final _BaseStopDart _baseStop = _lib
      .lookupFunction<_BaseStopNative, _BaseStopDart>('greader_base_stop');
  late final _BaseSetPowerDart _baseSetPower = _lib
      .lookupFunction<_BaseSetPowerNative, _BaseSetPowerDart>(
        'greader_base_set_power',
      );
  late final _InvEpcDart _invEpc = _lib
      .lookupFunction<_InvEpcNative, _InvEpcDart>(
        'greader_inventory_epc_start',
      );
  late final _OpenRs485Dart _openRs485 = _lib
      .lookupFunction<_OpenRs485Native, _OpenRs485Dart>('greader_open_rs485');
  late final _OpenUsbHidDart _openUsbHid = _lib
      .lookupFunction<_OpenUsbHidNative, _OpenUsbHidDart>(
        'greader_open_usbhid',
      );
  late final _ListUsbHidDart _listUsbHid = _lib
      .lookupFunction<_ListUsbHidNative, _ListUsbHidDart>(
        'greader_get_attached_usbhid_list',
      );
  late final _InvGbDart _invGb = _lib.lookupFunction<_InvGbNative, _InvGbDart>(
    'greader_inventory_gb_start',
  );
  late final _InvGjbDart _invGjb = _lib
      .lookupFunction<_InvGjbNative, _InvGjbDart>(
        'greader_inventory_gjb_start',
      );
  late final _InvTlDart _invTl = _lib.lookupFunction<_InvTlNative, _InvTlDart>(
    'greader_inventory_tl_start',
  );
  late final _WriteEpcDart _writeEpc = _lib
      .lookupFunction<_WriteEpcNative, _WriteEpcDart>('greader_write_epc');
  late final _LockEpcDart _lockEpc = _lib
      .lookupFunction<_LockEpcNative, _LockEpcDart>('greader_lock_epc');
  late final _SetVerboseDart _setVerbose = _lib
      .lookupFunction<_SetVerboseNative, _SetVerboseDart>(
        'greader_set_verbose_logging',
      );
  late final _DiagNextDart _diagNext = _lib
      .lookupFunction<_DiagNextNative, _DiagNextDart>('greader_diag_next_json');
  late final _DiagEmitDart _diagEmit = _lib
      .lookupFunction<_DiagEmitNative, _DiagEmitDart>('greader_diag_emit');

  String? diagNextJson() {
    final outStrPtr = pkg_ffi.malloc<ffi.Pointer<ffi.Int8>>();
    final outLenPtr = pkg_ffi.malloc<ffi.Int32>();
    try {
      final has = _diagNext(outStrPtr, outLenPtr);
      if (has == 0) return null;
      final ptr = outStrPtr.value;
      final len = outLenPtr.value;
      final s = ptr.cast<pkg_ffi.Utf8>().toDartString(length: len);
      _freeCstr(ptr);
      return s;
    } finally {
      pkg_ffi.malloc.free(outStrPtr);
      pkg_ffi.malloc.free(outLenPtr);
    }
  }

  void close(ffi.Pointer<ffi.Void> handle) => _close(handle);

  // Dart-style aliases (preferred)
  ffi.Pointer<ffi.Void> openSerial(String conn, {int timeoutSeconds = 3}) {
    final connPtrUtf8 = conn.toNativeUtf8();
    final connPtr = connPtrUtf8.cast<ffi.Int8>();
    try {
      return _open(connPtr, timeoutSeconds);
    } finally {
      pkg_ffi.malloc.free(connPtrUtf8);
    }
  }

  // Non-blocking version using a background isolate.
  Future<ffi.Pointer<ffi.Void>> openSerialAsync(
    String conn, {
    int timeoutSeconds = 3,
  }) async {
    final addr = await Isolate.run<int>(() {
      final p = GReaderFfi.instance.openSerial(
        conn,
        timeoutSeconds: timeoutSeconds,
      );
      return p.address;
    });
    return ffi.Pointer.fromAddress(addr);
  }

  // Deprecated names kept for backward compatibility.
  @Deprecated('Use openSerial instead')
  ffi.Pointer<ffi.Void> openRS232(String conn, {int timeoutSeconds = 3}) =>
      openSerial(conn, timeoutSeconds: timeoutSeconds);

  @Deprecated('Use openSerialAsync instead')
  Future<ffi.Pointer<ffi.Void>> openRS232Async(
    String conn, {
    int timeoutSeconds = 3,
  }) => openSerialAsync(conn, timeoutSeconds: timeoutSeconds);

  ffi.Pointer<ffi.Void> openTcp(String hostPort, {int timeoutSeconds = 3}) {
    final hpUtf8 = hostPort.toNativeUtf8();
    final hp = hpUtf8.cast<ffi.Int8>();
    try {
      return _openTcp(hp, timeoutSeconds);
    } finally {
      pkg_ffi.malloc.free(hpUtf8);
    }
  }

  // Non-blocking version using a background isolate.
  Future<ffi.Pointer<ffi.Void>> openTcpAsync(
    String hostPort, {
    int timeoutSeconds = 3,
  }) async {
    final addr = await Isolate.run<int>(() {
      final p = GReaderFfi.instance.openTcp(
        hostPort,
        timeoutSeconds: timeoutSeconds,
      );
      return p.address;
    });
    return ffi.Pointer.fromAddress(addr);
  }

  // RS485
  ffi.Pointer<ffi.Void> openRs485(String conn, {int timeoutSeconds = 3}) {
    final p = conn.toNativeUtf8().cast<ffi.Int8>();
    try {
      return _openRs485(p, timeoutSeconds);
    } finally {
      pkg_ffi.malloc.free(p.cast());
    }
  }

  Future<ffi.Pointer<ffi.Void>> openRs485Async(
    String conn, {
    int timeoutSeconds = 3,
  }) async {
    final addr = await Isolate.run<int>(() {
      final h = openRs485(conn, timeoutSeconds: timeoutSeconds);
      return h.address;
    });
    return ffi.Pointer.fromAddress(addr);
  }

  // USB HID
  ffi.Pointer<ffi.Void> openUsbHid(String path, {int timeoutSeconds = 3}) {
    final p = path.toNativeUtf8().cast<ffi.Int8>();
    try {
      return _openUsbHid(p, timeoutSeconds);
    } finally {
      pkg_ffi.malloc.free(p.cast());
    }
  }

  Future<ffi.Pointer<ffi.Void>> openUsbHidAsync(
    String path, {
    int timeoutSeconds = 3,
  }) async {
    // Dart-side diagnostic to ensure UI->FFI call path works
    try {
      diagEmit('{"type":"DartHidOpenStart","len":${path.length}}');
    } catch (_) {}
    // Important: Many vendor HID SDKs require all API calls to run on the same OS thread.
    // To avoid disconnection on write (UsbHidRemoved), perform open on the main isolate/thread.
    final h = GReaderFfi.instance.openUsbHid(
      path,
      timeoutSeconds: timeoutSeconds,
    );
    return h;
  }

  List<String> listUsbHid() {
    final lenPtr = pkg_ffi.malloc<ffi.Int32>();
    try {
      final ptr = _listUsbHid(lenPtr);
      if (ptr.address == 0) return const [];
      final s = ptr.cast<pkg_ffi.Utf8>().toDartString(length: lenPtr.value);
      _freeCstr(ptr);
      if (s.isEmpty) return const [];
      return s.split('\n');
    } finally {
      pkg_ffi.malloc.free(lenPtr);
    }
  }

  // Register native callbacks to queue events.
  void registerCallbacks(ffi.Pointer<ffi.Void> handle) {
    _registerCallbacks(handle);
  }

  // Alias with more natural name.
  void registerDefaultCallbacks(ffi.Pointer<ffi.Void> handle) =>
      registerCallbacks(handle);

  // Pop next JSON event; returns null if queue empty.
  String? eventsNextJson(ffi.Pointer<ffi.Void> handle) {
    final outStrPtr = pkg_ffi.malloc<ffi.Pointer<ffi.Int8>>();
    final outLenPtr = pkg_ffi.malloc<ffi.Int32>();
    try {
      final has = _eventsNext(handle, outStrPtr, outLenPtr);
      if (has == 0) return null;
      final ptr = outStrPtr.value;
      final len = outLenPtr.value;
      // Use extension override from package:ffi to avoid ambiguity.
      final s = ptr.cast<pkg_ffi.Utf8>().toDartString(length: len);
      _freeCstr(ptr);
      return s;
    } finally {
      pkg_ffi.malloc.free(outStrPtr);
      pkg_ffi.malloc.free(outLenPtr);
    }
  }

  // Alias with more natural name.
  String? nextEventJson(ffi.Pointer<ffi.Void> handle) => eventsNextJson(handle);

  // BaseStop with error buffer.
  ({int code, String? error}) baseStop(ffi.Pointer<ffi.Void> handle) {
    final errBuf = pkg_ffi.malloc.allocate<ffi.Int8>(512);
    try {
      errBuf[0] = 0;
      final code = _baseStop(handle, errBuf, 512);
      String? msg;
      if (code != 0) {
        msg = errBuf.cast<pkg_ffi.Utf8>().toDartString();
      }
      return (code: code, error: msg);
    } finally {
      pkg_ffi.malloc.free(errBuf);
    }
  }

  // BaseSetPower for antenna 1 by default.
  ({int code, String? error}) baseSetPower(
    ffi.Pointer<ffi.Void> handle, {
    int antennaNo = 1,
    required int power,
  }) {
    final errBuf = pkg_ffi.malloc.allocate<ffi.Int8>(256);
    try {
      errBuf[0] = 0;
      final code = _baseSetPower(handle, antennaNo, power, errBuf, 256);
      String? msg;
      if (code != 0) {
        msg = errBuf.cast<pkg_ffi.Utf8>().toDartString();
      }
      return (code: code, error: msg);
    } finally {
      pkg_ffi.malloc.free(errBuf);
    }
  }

  // Start EPC inventory with minimal params.
  ({int code, String? error}) inventoryEpcStart(
    ffi.Pointer<ffi.Void> handle, {
    required int antennaEnable,
    int inventoryMode = 1,
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
    int readTidLen = 0,
    int timeoutMs = 0,
  }) {
    final filterPtr = (filterHex ?? '').toNativeUtf8().cast<ffi.Int8>();
    final errBuf = pkg_ffi.malloc.allocate<ffi.Int8>(256);
    try {
      errBuf[0] = 0;
      final code = _invEpc(
        handle,
        antennaEnable,
        inventoryMode,
        filterArea,
        filterPtr,
        filterBitStart,
        readTidLen,
        timeoutMs,
        errBuf,
        256,
      );
      String? msg;
      if (code != 0) {
        msg = errBuf.cast<pkg_ffi.Utf8>().toDartString();
      }
      return (code: code, error: msg);
    } finally {
      pkg_ffi.malloc.free(errBuf);
      pkg_ffi.malloc.free(filterPtr.cast());
    }
  }

  // Start GB inventory with minimal params.
  ({int code, String? error}) inventoryGbStart(
    ffi.Pointer<ffi.Void> handle, {
    required int antennaEnable,
    int inventoryMode = 1,
    int timeoutMs = 0,
  }) {
    final errBuf = pkg_ffi.malloc.allocate<ffi.Int8>(256);
    try {
      errBuf[0] = 0;
      final code = _invGb(
        handle,
        antennaEnable,
        inventoryMode,
        timeoutMs,
        errBuf,
        256,
      );
      String? msg;
      if (code != 0) {
        msg = errBuf.cast<pkg_ffi.Utf8>().toDartString();
      }
      return (code: code, error: msg);
    } finally {
      pkg_ffi.malloc.free(errBuf);
    }
  }

  // Start GJB inventory with minimal params.
  ({int code, String? error}) inventoryGjbStart(
    ffi.Pointer<ffi.Void> handle, {
    required int antennaEnable,
    int inventoryMode = 1,
    int timeoutMs = 0,
  }) {
    final errBuf = pkg_ffi.malloc.allocate<ffi.Int8>(256);
    try {
      errBuf[0] = 0;
      final code = _invGjb(
        handle,
        antennaEnable,
        inventoryMode,
        timeoutMs,
        errBuf,
        256,
      );
      String? msg;
      if (code != 0) {
        msg = pkg_ffi.Utf8Pointer(errBuf.cast<pkg_ffi.Utf8>()).toDartString();
      }
      return (code: code, error: msg);
    } finally {
      pkg_ffi.malloc.free(errBuf);
    }
  }

  // Start TL inventory with minimal params.
  ({int code, String? error}) inventoryTlStart(
    ffi.Pointer<ffi.Void> handle, {
    required int antennaEnable,
    int inventoryMode = 1,
    int timeoutMs = 0,
  }) {
    final errBuf = pkg_ffi.malloc.allocate<ffi.Int8>(256);
    try {
      errBuf[0] = 0;
      final code = _invTl(
        handle,
        antennaEnable,
        inventoryMode,
        timeoutMs,
        errBuf,
        256,
      );
      String? msg;
      if (code != 0) {
        msg = pkg_ffi.Utf8Pointer(errBuf.cast<pkg_ffi.Utf8>()).toDartString();
      }
      return (code: code, error: msg);
    } finally {
      pkg_ffi.malloc.free(errBuf);
    }
  }

  // Write EPC simplified wrapper.
  ({int code, String? error}) writeEpc(
    ffi.Pointer<ffi.Void> handle, {
    required int antennaEnable,
    int area = 1, // EPC bank
    int startWord = 2,
    required String hexData,
    String passwordHex = '',
    int block = 1,
    int stayCw = 0,
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
  }) {
    final dataPtr = hexData.toNativeUtf8().cast<ffi.Int8>();
    final pwdPtr = passwordHex.toNativeUtf8().cast<ffi.Int8>();
    final filterPtr = (filterHex ?? '').toNativeUtf8().cast<ffi.Int8>();
    final errBuf = pkg_ffi.malloc.allocate<ffi.Int8>(256);
    try {
      errBuf[0] = 0;
      final code = _writeEpc(
        handle,
        antennaEnable,
        area,
        startWord,
        dataPtr,
        pwdPtr,
        block,
        stayCw,
        filterArea,
        filterPtr,
        filterBitStart,
        errBuf,
        256,
      );
      String? msg;
      if (code != 0) {
        msg = pkg_ffi.Utf8Pointer(errBuf.cast<pkg_ffi.Utf8>()).toDartString();
      }
      return (code: code, error: msg);
    } finally {
      pkg_ffi.malloc.free(errBuf);
      pkg_ffi.malloc.free(dataPtr.cast());
      pkg_ffi.malloc.free(pwdPtr.cast());
      pkg_ffi.malloc.free(filterPtr.cast());
    }
  }

  // Lock EPC simplified wrapper.
  ({int code, String? error}) lockEpc(
    ffi.Pointer<ffi.Void> handle, {
    required int antennaEnable,
    int area = 1, // EPC bank
    required int mode,
    String passwordHex = '',
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
  }) {
    final pwdPtr = passwordHex.toNativeUtf8().cast<ffi.Int8>();
    final filterPtr = (filterHex ?? '').toNativeUtf8().cast<ffi.Int8>();
    final errBuf = pkg_ffi.malloc.allocate<ffi.Int8>(256);
    try {
      errBuf[0] = 0;
      final code = _lockEpc(
        handle,
        antennaEnable,
        area,
        mode,
        pwdPtr,
        filterArea,
        filterPtr,
        filterBitStart,
        errBuf,
        256,
      );
      String? msg;
      if (code != 0) {
        msg = pkg_ffi.Utf8Pointer(errBuf.cast<pkg_ffi.Utf8>()).toDartString();
      }
      return (code: code, error: msg);
    } finally {
      pkg_ffi.malloc.free(errBuf);
      pkg_ffi.malloc.free(pwdPtr.cast());
      pkg_ffi.malloc.free(filterPtr.cast());
    }
  }

  void setVerboseLogging(bool enabled) => _setVerbose(enabled ? 1 : 0);

  void diagEmit(String json) {
    final p = json.toNativeUtf8().cast<ffi.Int8>();
    try {
      _diagEmit(p);
    } finally {
      pkg_ffi.malloc.free(p.cast());
    }
  }

  ffi.DynamicLibrary _openLibrary() {
    if (Platform.isWindows) {
      // Flutter copies the plugin DLL into the application directory.
      return ffi.DynamicLibrary.open('greader_plugin_plugin.dll');
    }
    throw UnsupportedError('Only Windows is supported');
  }
}

/// 事件枚举（来自原生回调的 JSON 字段 `type`）。
enum GEventKind {
  tagEpcLog,
  tagEpcOver,
  tagGbLog,
  tagGbOver,
  tagGjbLog,
  tagGjbOver,
  tagTlLog,
  tagTlOver,
  tag6bLog,
  tag6bOver,
  tag6DLog,
  tag6DOver,
  gpiStart,
  gpiOver,
  tcpDisconnected,
  usbHidRemoved,
  unknown,
}

/// Dart 侧强类型事件基类。
abstract class GEvent {
  final GEventKind kind;
  final Map<String, dynamic> raw; // 原始 JSON map，便于取用额外字段
  const GEvent(this.kind, this.raw);

  static GEvent fromJson(String json) {
    Map<String, dynamic> m;
    try {
      m = convert.jsonDecode(json) as Map<String, dynamic>;
    } catch (_) {
      return GUnknownEvent(json);
    }
    final t = (m['type'] ?? '').toString();
    switch (t) {
      case 'TagEpcLog':
        return GTagEpcLogEvent(
          epc: (m['epc'] ?? '').toString(),
          tid: (m['tid']?.toString().isEmpty ?? true)
              ? null
              : m['tid'].toString(),
          ant: _asIntOrNull(m['ant']),
          rssi: _asIntOrNull(m['rssi']),
          raw: m,
        );
      case 'TagEpcOver':
        return GSimpleEvent(GEventKind.tagEpcOver, m);
      case 'TagGbLog':
        return GSimpleEvent(GEventKind.tagGbLog, m);
      case 'TagGbOver':
        return GSimpleEvent(GEventKind.tagGbOver, m);
      case 'TagGjbLog':
        return GSimpleEvent(GEventKind.tagGjbLog, m);
      case 'TagGjbOver':
        return GSimpleEvent(GEventKind.tagGjbOver, m);
      case 'TagTLLog':
        return GSimpleEvent(GEventKind.tagTlLog, m);
      case 'TagTLOver':
        return GSimpleEvent(GEventKind.tagTlOver, m);
      case 'Tag6bLog':
        return GSimpleEvent(GEventKind.tag6bLog, m);
      case 'Tag6bOver':
        return GSimpleEvent(GEventKind.tag6bOver, m);
      case 'Tag6DLog':
        return GSimpleEvent(GEventKind.tag6DLog, m);
      case 'Tag6DOver':
        return GSimpleEvent(GEventKind.tag6DOver, m);
      case 'GpiStart':
        return GSimpleEvent(GEventKind.gpiStart, m);
      case 'GpiOver':
        return GSimpleEvent(GEventKind.gpiOver, m);
      case 'TcpDisconnected':
        return GSimpleEvent(GEventKind.tcpDisconnected, m);
      case 'UsbHidRemoved':
        return GSimpleEvent(GEventKind.usbHidRemoved, m);
      default:
        return GSimpleEvent(GEventKind.unknown, m);
    }
  }

  static int? _asIntOrNull(Object? v) {
    if (v == null) return null;
    if (v is int) return v;
    return int.tryParse(v.toString());
  }
}

/// 未知或解析失败的事件（保留原始 JSON 字符串）。
class GUnknownEvent extends GEvent {
  final String json;
  GUnknownEvent(this.json) : super(GEventKind.unknown, const {});
}

/// EPC 盘点日志事件。
class GTagEpcLogEvent extends GEvent {
  final String epc;
  final String? tid;
  final int? ant;
  final int? rssi;
  GTagEpcLogEvent({
    required this.epc,
    this.tid,
    this.ant,
    this.rssi,
    Map<String, dynamic> raw = const {},
  }) : super(GEventKind.tagEpcLog, raw);
}

/// 仅携带类型与原始字段的简单事件。
class GSimpleEvent extends GEvent {
  const GSimpleEvent(super.kind, super.raw);
}

// Keep the original stub API minimal for now.
class GreaderPlugin {
  static Future<String?> getPlatformVersionStatic() async {
    return 'Windows-FFI';
  }

  Future<String?> getPlatformVersion() async {
    return GreaderPlugin.getPlatformVersionStatic();
  }
}

/// 纯 Dart 的高层封装，隐藏 C++/FFI 细节。
///
/// 使用方式：
/// - final reader = await GReader.openTcp('192.168.1.168:8160');
/// - await reader.registerCallbacks();
/// - final r = reader.inventoryEpcStart(antennaEnable: 1);
/// - reader.close();
class GReader {
  final GReaderFfi _ffi = GReaderFfi.instance;
  ffi.Pointer<ffi.Void> _handle;
  Future<void> _op = Future.value(); // serialize ops

  // ---- Event/Diag callback-style dispatch ----
  StreamController<String>? _eventsCtrl;
  Timer? _eventsTimer;
  static StreamController<String>? _diagCtrl;
  static Timer? _diagTimer;
  // typed events: 直接由 JSON 流映射生成，避免重复定时器
  Stream<GEvent>? _typedEvents;

  GReader._(this._handle);

  bool get isOpen => _handle.address != 0;

  bool get isClosed => _handle.address == 0;

  int get handleAddress => _handle.address; // 可选：仅用于调试显示

  // ---- Open helpers (async, 不阻塞 UI) ----
  static Future<GReader> openSerial(
    String conn, {
    int timeoutSeconds = 3,
  }) async {
    final h = await GReaderFfi.instance
        .openSerialAsync(conn, timeoutSeconds: timeoutSeconds)
        .timeout(
          Duration(seconds: timeoutSeconds + 2),
          onTimeout: () => ffi.Pointer.fromAddress(0),
        );
    final inst = GReader._(h);
    if (h.address != 0) {
      // Auto-register callbacks to ensure events/diagnostics flow.
      inst.registerCallbacks();
    }
    return inst;
  }

  static Future<GReader> openTcp(
    String hostPort, {
    int timeoutSeconds = 3,
  }) async {
    final h = await GReaderFfi.instance
        .openTcpAsync(hostPort, timeoutSeconds: timeoutSeconds)
        .timeout(
          Duration(seconds: timeoutSeconds + 2),
          onTimeout: () => ffi.Pointer.fromAddress(0),
        );
    final inst = GReader._(h);
    if (h.address != 0) {
      inst.registerCallbacks();
    }
    return inst;
  }

  static Future<GReader> openRs485(
    String conn, {
    int timeoutSeconds = 3,
  }) async {
    final h = await GReaderFfi.instance
        .openRs485Async(conn, timeoutSeconds: timeoutSeconds)
        .timeout(
          Duration(seconds: timeoutSeconds + 2),
          onTimeout: () => ffi.Pointer.fromAddress(0),
        );
    final inst = GReader._(h);
    if (h.address != 0) {
      inst.registerCallbacks();
    }
    return inst;
  }

  static Future<GReader> openUsbHid(
    String path, {
    int timeoutSeconds = 6,
  }) async {
    final h = await GReaderFfi.instance
        .openUsbHidAsync(path, timeoutSeconds: timeoutSeconds)
        .timeout(
          Duration(seconds: timeoutSeconds + 2),
          onTimeout: () => ffi.Pointer.fromAddress(0),
        );
    final inst = GReader._(h);
    if (h.address != 0) {
      // Small delay to ensure device is fully ready before registering callbacks/commands.
      await Future.delayed(const Duration(milliseconds: 150));
      inst.registerCallbacks();
    }
    return inst;
  }

  static List<String> listUsbHid() => GReaderFfi.instance.listUsbHid();

  // ---- Lifecycle ----
  void close() {
    if (!isOpen) return;
    // stop event/diag dispatchers
    stopEventDispatch();
    // Do not stop global diag dispatcher here (could be used by other instances)
    try {
      _eventsCtrl?.close();
    } catch (_) {}
    _eventsCtrl = null;
    _ffi.close(_handle);
    _handle = ffi.Pointer.fromAddress(0);
  }

  // ---- Events ----
  void registerCallbacks() {
    if (!isOpen) return;
    _ffi.registerCallbacks(_handle);
  }

  String? nextEventJson() {
    if (!isOpen) return null;
    return _ffi.eventsNextJson(_handle);
  }

  // Start periodic drain of native event queue and push to Stream listeners.
  void startEventDispatch({
    Duration interval = const Duration(milliseconds: 30),
  }) {
    if (!isOpen) return;
    _eventsCtrl ??= StreamController<String>.broadcast(
      onCancel: () {
        // If no more listeners, stop timer
        if (!(_eventsCtrl?.hasListener ?? false)) {
          _eventsTimer?.cancel();
          _eventsTimer = null;
        }
      },
    );
    _eventsTimer?.cancel();
    _eventsTimer = Timer.periodic(interval, (_) {
      if (!isOpen) return;
      // Drain all pending events in one tick
      while (true) {
        final line = _ffi.eventsNextJson(_handle);
        if (line == null || line.isEmpty) break;
        _eventsCtrl!.add(line);
      }
    });
  }

  // Stop the event timer (stream remains, can be re-used/restarted)
  void stopEventDispatch() {
    _eventsTimer?.cancel();
    _eventsTimer = null;
  }

  // Get events stream; auto-start dispatch if needed.
  Stream<String> events({
    Duration interval = const Duration(milliseconds: 30),
  }) {
    if (_eventsCtrl == null) {
      startEventDispatch(interval: interval);
    } else if (_eventsTimer == null) {
      startEventDispatch(interval: interval);
    }
    return _eventsCtrl!.stream;
  }

  // Convenience: register a callback; returns subscription to cancel later.
  StreamSubscription<String> onEvent(
    void Function(String json) listener, {
    Duration interval = const Duration(milliseconds: 30),
  }) {
    return events(interval: interval).listen(listener);
  }

  // ---- Typed events (recommended) ----
  Stream<GEvent> eventsTyped({
    Duration interval = const Duration(milliseconds: 30),
  }) {
    _typedEvents ??= events(
      interval: interval,
    ).map(GEvent.fromJson).asBroadcastStream();
    return _typedEvents!;
  }

  StreamSubscription<GEvent> onEventTyped(
    void Function(GEvent event) listener, {
    Duration interval = const Duration(milliseconds: 30),
  }) {
    return eventsTyped(interval: interval).listen(listener);
  }

  // Global diagnostics stream (library-wide), useful for debugging.
  static void startDiagDispatch({
    Duration interval = const Duration(milliseconds: 80),
  }) {
    _diagCtrl ??= StreamController<String>.broadcast(
      onCancel: () {
        if (!(_diagCtrl?.hasListener ?? false)) {
          _diagTimer?.cancel();
          _diagTimer = null;
        }
      },
    );
    _diagTimer?.cancel();
    _diagTimer = Timer.periodic(interval, (_) {
      while (true) {
        final line = GReaderFfi.instance.diagNextJson();
        if (line == null || line.isEmpty) break;
        _diagCtrl!.add(line);
      }
    });
  }

  static void stopDiagDispatch() {
    _diagTimer?.cancel();
    _diagTimer = null;
  }

  static Stream<String> diagEvents({
    Duration interval = const Duration(milliseconds: 80),
  }) {
    if (_diagCtrl == null || _diagTimer == null) {
      startDiagDispatch(interval: interval);
    }
    return _diagCtrl!.stream;
  }

  static StreamSubscription<String> onDiag(
    void Function(String json) listener, {
    Duration interval = const Duration(milliseconds: 80),
  }) {
    return diagEvents(interval: interval).listen(listener);
  }

  // ---- Base operations ----
  ({int code, String? error}) baseStop() {
    if (!isOpen) return (code: -1, error: 'not open');
    // diagnostics
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({'type': 'DartBaseStopStart'}),
      );
    } catch (_) {}
    // serialize to avoid overlapping with other commands
    final res = _serialize(() => _ffi.baseStop(_handle));
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartBaseStopResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  // Async version: run in background isolate to avoid UI jank
  Future<({int code, String? error})> baseStopAsync() async {
    if (!isOpen) return (code: -1, error: 'not open');
    final addr = _handle.address;
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({'type': 'DartBaseStopStart'}),
      );
    } catch (_) {}
    final res = await Isolate.run<({int code, String? error})>(() {
      final h = ffi.Pointer<ffi.Void>.fromAddress(addr);
      return GReaderFfi.instance.baseStop(h);
    });
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartBaseStopResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  ({int code, String? error}) setPower({
    int antennaNo = 1,
    required int power,
  }) {
    if (!isOpen) return (code: -1, error: 'not open');
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartSetPowerStart',
          'antennaNo': antennaNo,
          'power': power,
        }),
      );
    } catch (_) {}
    final res = _serialize(
      () => _ffi.baseSetPower(_handle, antennaNo: antennaNo, power: power),
    );
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartSetPowerResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  Future<({int code, String? error})> setPowerAsync({
    int antennaNo = 1,
    required int power,
  }) async {
    if (!isOpen) return (code: -1, error: 'not open');
    final addr = _handle.address;
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartSetPowerStart',
          'antennaNo': antennaNo,
          'power': power,
        }),
      );
    } catch (_) {}
    final res = await Isolate.run<({int code, String? error})>(() {
      final h = ffi.Pointer<ffi.Void>.fromAddress(addr);
      return GReaderFfi.instance.baseSetPower(
        h,
        antennaNo: antennaNo,
        power: power,
      );
    });
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartSetPowerResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  // ---- Inventories ----
  ({int code, String? error}) inventoryEpcStart({
    required int antennaEnable,
    int inventoryMode = 1,
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
    int readTidLen = 0,
    int timeoutMs = 0,
  }) {
    if (!isOpen) return (code: -1, error: 'not open');
    // Emit diagnostics before/after to trace failures like "Failed to WriteCmd".
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartInvEpcStart',
          'antennaEnable': antennaEnable,
          'inventoryMode': inventoryMode,
          'filterArea': filterArea,
          'filterLen': (filterHex ?? '').length,
          'filterBitStart': filterBitStart,
          'readTidLen': readTidLen,
          'timeoutMs': timeoutMs,
        }),
      );
    } catch (_) {}
    final res = _serialize(
      () => _ffi.inventoryEpcStart(
        _handle,
        antennaEnable: antennaEnable,
        inventoryMode: inventoryMode,
        filterArea: filterArea,
        filterHex: filterHex,
        filterBitStart: filterBitStart,
        readTidLen: readTidLen,
        timeoutMs: timeoutMs,
      ),
    );
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartInvEpcResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  Future<({int code, String? error})> inventoryEpcStartAsync({
    required int antennaEnable,
    int inventoryMode = 1,
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
    int readTidLen = 0,
    int timeoutMs = 0,
  }) async {
    if (!isOpen) return (code: -1, error: 'not open');
    final addr = _handle.address;
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartInvEpcStart',
          'antennaEnable': antennaEnable,
          'inventoryMode': inventoryMode,
          'filterArea': filterArea,
          'filterLen': (filterHex ?? '').length,
          'filterBitStart': filterBitStart,
          'readTidLen': readTidLen,
          'timeoutMs': timeoutMs,
        }),
      );
    } catch (_) {}
    final res = await Isolate.run<({int code, String? error})>(() {
      final h = ffi.Pointer<ffi.Void>.fromAddress(addr);
      return GReaderFfi.instance.inventoryEpcStart(
        h,
        antennaEnable: antennaEnable,
        inventoryMode: inventoryMode,
        filterArea: filterArea,
        filterHex: filterHex,
        filterBitStart: filterBitStart,
        readTidLen: readTidLen,
        timeoutMs: timeoutMs,
      );
    });
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartInvEpcResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  ({int code, String? error}) inventoryGbStart({
    required int antennaEnable,
    int inventoryMode = 1,
    int timeoutMs = 0,
  }) {
    if (!isOpen) return (code: -1, error: 'not open');
    return _serialize(
      () => _ffi.inventoryGbStart(
        _handle,
        antennaEnable: antennaEnable,
        inventoryMode: inventoryMode,
        timeoutMs: timeoutMs,
      ),
    );
  }

  ({int code, String? error}) inventoryGjbStart({
    required int antennaEnable,
    int inventoryMode = 1,
    int timeoutMs = 0,
  }) {
    if (!isOpen) return (code: -1, error: 'not open');
    return _serialize(
      () => _ffi.inventoryGjbStart(
        _handle,
        antennaEnable: antennaEnable,
        inventoryMode: inventoryMode,
        timeoutMs: timeoutMs,
      ),
    );
  }

  ({int code, String? error}) inventoryTlStart({
    required int antennaEnable,
    int inventoryMode = 1,
    int timeoutMs = 0,
  }) {
    if (!isOpen) return (code: -1, error: 'not open');
    return _serialize(
      () => _ffi.inventoryTlStart(
        _handle,
        antennaEnable: antennaEnable,
        inventoryMode: inventoryMode,
        timeoutMs: timeoutMs,
      ),
    );
  }

  // ---- EPC ops ----
  // Helper: compute PC (2 bytes, 4 hex chars) from EPC hex length in words, like vendor sample.
  static String computePcHexForEpc(String epcHex) {
    final clean = epcHex.replaceAll(RegExp(r"[^0-9A-Fa-f]"), '');
    if (clean.isEmpty || clean.length % 4 != 0) {
      throw ArgumentError(
        'EPC hex length must be a multiple of 4 (word-aligned)',
      );
    }
    final words = clean.length ~/ 4; // 16-bit words
    final pc = (words << 11) & 0xFFFF;
    return pc.toRadixString(16).padLeft(4, '0').toUpperCase();
  }

  // Convenience: write with PC+EPC at startWord=1 (Area=EPC), optional filter.
  ({int code, String? error}) writeEpcWithPc({
    required int antennaEnable,
    required String epcHex,
    String passwordHex = '',
    int block = 0,
    int stayCw = 0,
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
  }) {
    final cleanEpc = epcHex
        .replaceAll(RegExp(r"[^0-9A-Fa-f]"), '')
        .toUpperCase();
    // 1024 bytes = 2048 hex chars, PC+EPC = 4+N
    if (cleanEpc.isEmpty) {
      GReaderFfi.instance.diagEmit(
        '{"type":"DartWriteEpcResult","code":-2,"error":"EPC不能为空"}',
      );
      return (code: -2, error: 'EPC不能为空');
    }
    if (cleanEpc.length % 4 != 0) {
      GReaderFfi.instance.diagEmit(
        '{"type":"DartWriteEpcResult","code":-2,"error":"EPC长度必须为4的倍数(16位字对齐)"}',
      );
      return (code: -2, error: 'EPC长度必须为4的倍数(16位字对齐)');
    }
    if (cleanEpc.length > 512) {
      // 512=2048/4, 预留足够空间给PC
      GReaderFfi.instance.diagEmit(
        '{"type":"DartWriteEpcResult","code":-2,"error":"EPC过长，最大支持512个hex字符(256字节)"}',
      );
      return (code: -2, error: 'EPC过长，最大支持512个hex字符(256字节)');
    }
    final pc = computePcHexForEpc(cleanEpc);
    return writeEpc(
      antennaEnable: antennaEnable,
      area: 1,
      startWord: 1,
      hexData: pc + cleanEpc,
      passwordHex: passwordHex,
      block: block,
      stayCw: stayCw,
      filterArea: filterArea,
      filterHex: filterHex,
      filterBitStart: filterBitStart,
    );
  }

  Future<({int code, String? error})> writeEpcWithPcAsync({
    required int antennaEnable,
    required String epcHex,
    String passwordHex = '',
    int block = 0,
    int stayCw = 0,
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
  }) async {
    final cleanEpc = epcHex
        .replaceAll(RegExp(r"[^0-9A-Fa-f]"), '')
        .toUpperCase();
    if (cleanEpc.isEmpty) {
      GReaderFfi.instance.diagEmit(
        '{"type":"DartWriteEpcResult","code":-2,"error":"EPC不能为空"}',
      );
      return (code: -2, error: 'EPC不能为空');
    }
    if (cleanEpc.length % 4 != 0) {
      GReaderFfi.instance.diagEmit(
        '{"type":"DartWriteEpcResult","code":-2,"error":"EPC长度必须为4的倍数(16位字对齐)"}',
      );
      return (code: -2, error: 'EPC长度必须为4的倍数(16位字对齐)');
    }
    if (cleanEpc.length > 512) {
      GReaderFfi.instance.diagEmit(
        '{"type":"DartWriteEpcResult","code":-2,"error":"EPC过长，最大支持512个hex字符(256字节)"}',
      );
      return (code: -2, error: 'EPC过长，最大支持512个hex字符(256字节)');
    }
    final pc = computePcHexForEpc(cleanEpc);
    return writeEpcAsync(
      antennaEnable: antennaEnable,
      area: 1,
      startWord: 1,
      hexData: pc + cleanEpc,
      passwordHex: passwordHex,
      block: block,
      stayCw: stayCw,
      filterArea: filterArea,
      filterHex: filterHex,
      filterBitStart: filterBitStart,
    );
  }

  ({int code, String? error}) writeEpc({
    required int antennaEnable,
    int area = 1,
    int startWord = 2,
    required String hexData,
    String passwordHex = '',
    int block = 1,
    int stayCw = 0,
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
  }) {
    if (!isOpen) return (code: -1, error: 'not open');
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartWriteEpcStart',
          'antennaEnable': antennaEnable,
          'area': area,
          'startWord': startWord,
          'hexLen': hexData.length,
          'hasPwd': passwordHex.isNotEmpty,
          'block': block,
          'stayCw': stayCw,
          'filterArea': filterArea,
          'filterLen': (filterHex ?? '').length,
          'filterBitStart': filterBitStart,
        }),
      );
    } catch (_) {}
    final res = _serialize(
      () => _ffi.writeEpc(
        _handle,
        antennaEnable: antennaEnable,
        area: area,
        startWord: startWord,
        hexData: hexData,
        passwordHex: passwordHex,
        block: block,
        stayCw: stayCw,
        filterArea: filterArea,
        filterHex: filterHex,
        filterBitStart: filterBitStart,
      ),
    );
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartWriteEpcResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  Future<({int code, String? error})> writeEpcAsync({
    required int antennaEnable,
    int area = 1,
    int startWord = 2,
    required String hexData,
    String passwordHex = '',
    int block = 1,
    int stayCw = 0,
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
  }) async {
    if (!isOpen) return (code: -1, error: 'not open');
    final addr = _handle.address;
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartWriteEpcStart',
          'antennaEnable': antennaEnable,
          'area': area,
          'startWord': startWord,
          'hexLen': hexData.length,
          'hasPwd': passwordHex.isNotEmpty,
          'block': block,
          'stayCw': stayCw,
          'filterArea': filterArea,
          'filterLen': (filterHex ?? '').length,
          'filterBitStart': filterBitStart,
        }),
      );
    } catch (_) {}
    final res = await Isolate.run<({int code, String? error})>(() {
      final h = ffi.Pointer<ffi.Void>.fromAddress(addr);
      return GReaderFfi.instance.writeEpc(
        h,
        antennaEnable: antennaEnable,
        area: area,
        startWord: startWord,
        hexData: hexData,
        passwordHex: passwordHex,
        block: block,
        stayCw: stayCw,
        filterArea: filterArea,
        filterHex: filterHex,
        filterBitStart: filterBitStart,
      );
    });
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartWriteEpcResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  ({int code, String? error}) lockEpc({
    required int antennaEnable,
    int area = 1,
    required int mode,
    String passwordHex = '',
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
  }) {
    if (!isOpen) return (code: -1, error: 'not open');
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartLockEpcStart',
          'antennaEnable': antennaEnable,
          'area': area,
          'mode': mode,
          'hasPwd': passwordHex.isNotEmpty,
          'filterArea': filterArea,
          'filterLen': (filterHex ?? '').length,
          'filterBitStart': filterBitStart,
        }),
      );
    } catch (_) {}
    final res = _serialize(
      () => _ffi.lockEpc(
        _handle,
        antennaEnable: antennaEnable,
        area: area,
        mode: mode,
        passwordHex: passwordHex,
        filterArea: filterArea,
        filterHex: filterHex,
        filterBitStart: filterBitStart,
      ),
    );
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartLockEpcResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  Future<({int code, String? error})> lockEpcAsync({
    required int antennaEnable,
    int area = 1,
    required int mode,
    String passwordHex = '',
    int filterArea = -1,
    String? filterHex,
    int filterBitStart = 0,
  }) async {
    if (!isOpen) return (code: -1, error: 'not open');
    final addr = _handle.address;
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartLockEpcStart',
          'antennaEnable': antennaEnable,
          'area': area,
          'mode': mode,
          'hasPwd': passwordHex.isNotEmpty,
          'filterArea': filterArea,
          'filterLen': (filterHex ?? '').length,
          'filterBitStart': filterBitStart,
        }),
      );
    } catch (_) {}
    final res = await Isolate.run<({int code, String? error})>(() {
      final h = ffi.Pointer<ffi.Void>.fromAddress(addr);
      return GReaderFfi.instance.lockEpc(
        h,
        antennaEnable: antennaEnable,
        area: area,
        mode: mode,
        passwordHex: passwordHex,
        filterArea: filterArea,
        filterHex: filterHex,
        filterBitStart: filterBitStart,
      );
    });
    try {
      GReaderFfi.instance.diagEmit(
        convert.jsonEncode({
          'type': 'DartLockEpcResult',
          'code': res.code,
          'error': res.error,
        }),
      );
    } catch (_) {}
    return res;
  }

  // Simple synchronous serialization helper for short native calls.
  ({int code, String? error}) _serialize(
    ({int code, String? error}) Function() fn,
  ) {
    // chain on a completed future to serialize calls
    _op = _op.then((_) {});
    try {
      return fn();
    } finally {
      // no-op to keep the chain
    }
  }

  static void setVerboseLogging(bool enabled) =>
      GReaderFfi.instance.setVerboseLogging(enabled);

  static String? nextDiagEventJson() => GReaderFfi.instance.diagNextJson();
}
