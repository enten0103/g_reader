import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:greader_plugin/greader_plugin.dart';
import 'dart:convert' as convert;
// 使用高层封装 GReader，避免直接操作 FFI。

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';
  final _greaderPlugin = GreaderPlugin();
  GReader? _reader;

  final TextEditingController _connCtrl = TextEditingController(
    text: 'COM3:115200',
  );
  final TextEditingController _tcpCtrl = TextEditingController(
    text: '192.168.1.168:8160',
  );
  String _status = 'Idle';
  final List<String> _events = [];
  StreamSubscription<dynamic>? _evtSub; // String 或 GEvent 订阅皆可
  StreamSubscription<String>? _diagSub;
  List<String> _hidList = const [];
  String? _selectedHid;
  bool _openingHid = false;
  bool _verbose = true;
  bool _busy = false; // 简单的操作互斥，避免重复点击造成卡顿感觉
  // Simple inputs for write/lock
  final _epcHexCtrl = TextEditingController(text: '300833B2DDD9014000000000');
  final _pwdHexCtrl = TextEditingController(text: '00000000');
  String? _lastTid;

  @override
  void initState() {
    super.initState();
    // 开启全局轮询，确保即使未成功打开句柄也能看到诊断事件
    GReader.setVerboseLogging(_verbose);
    // 订阅全局诊断（无需手动轮询）
    _diagSub = GReader.onDiag((json) {
      _pushEventUnique(json);
      if (mounted) setState(() {});
    });
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    try {
      platformVersion =
          await _greaderPlugin.getPlatformVersion() ??
          'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  void dispose() {
    _evtSub?.cancel();
    _diagSub?.cancel();
    _reader?.close();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Plugin example app')),
        body: Padding(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                children: [
                  Expanded(child: Text('Running on: $_platformVersion')),
                  const SizedBox(width: 12),
                  Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      const Text('Verbose'),
                      const SizedBox(width: 6),
                      Switch(
                        value: _verbose,
                        onChanged: (v) {
                          setState(() => _verbose = v);
                          GReader.setVerboseLogging(v);
                        },
                      ),
                    ],
                  ),
                ],
              ),
              const SizedBox(height: 16),
              const Text('Serial (e.g., COM3:115200):'),
              TextField(
                controller: _connCtrl,
                decoration: const InputDecoration(
                  border: OutlineInputBorder(),
                  hintText: 'COMx:baud',
                ),
              ),
              const SizedBox(height: 12),
              const Text('TCP (e.g., 192.168.1.168:8160):'),
              TextField(
                controller: _tcpCtrl,
                decoration: const InputDecoration(
                  border: OutlineInputBorder(),
                  hintText: 'host:port',
                ),
              ),
              const SizedBox(height: 12),
              Row(
                children: [
                  ElevatedButton(
                    onPressed: () {
                      _hidList = GReader.listUsbHid();
                      _selectedHid = _hidList.isNotEmpty
                          ? _hidList.first
                          : null;
                      setState(() => _status = 'HID count: ${_hidList.length}');
                    },
                    child: const Text('List USB HID'),
                  ),
                  const SizedBox(width: 12),
                  Expanded(
                    child: DropdownButton<String>(
                      isExpanded: true,
                      value: _selectedHid,
                      hint: const Text('Select HID path'),
                      items: _hidList
                          .map(
                            (e) => DropdownMenuItem(value: e, child: Text(e)),
                          )
                          .toList(),
                      onChanged: (v) => setState(() => _selectedHid = v),
                    ),
                  ),
                  const SizedBox(width: 12),
                  ElevatedButton(
                    onPressed: _openingHid
                        ? null
                        : () async {
                            if (_selectedHid == null) return;
                            setState(() {
                              _openingHid = true;
                              _status = 'Opening USB HID...';
                            });
                            Timer? watchdog;
                            try {
                              final completer = Completer<GReader?>();
                              // start open
                              GReader.openUsbHid(_selectedHid!)
                                  .then((r) {
                                    if (!completer.isCompleted) {
                                      completer.complete(r);
                                    }
                                  })
                                  .catchError((e, st) {
                                    if (!completer.isCompleted) {
                                      completer.complete(null);
                                    }
                                  });
                              // timeout fallback (8s)
                              watchdog = Timer(const Duration(seconds: 8), () {
                                if (!completer.isCompleted) {
                                  completer.complete(null);
                                }
                              });
                              final r = await completer.future;
                              watchdog.cancel();
                              if (!mounted) return;
                              if (r != null && r.isOpen) {
                                _reader = r;
                                _reader!.registerCallbacks();
                                _listenReaderEvents();
                                setState(() {
                                  _status = 'USB HID Open OK';
                                });
                              } else {
                                _reader = null;
                                setState(() {
                                  _status = 'USB HID Open FAILED';
                                });
                              }
                            } finally {
                              if (mounted) setState(() => _openingHid = false);
                            }
                          },
                    child: const Text('Open HID'),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              Row(
                children: [
                  ElevatedButton(
                    onPressed: () async {
                      setState(() => _status = 'Opening RS232...');
                      try {
                        _reader = await GReader.openSerial(
                          _connCtrl.text,
                          timeoutSeconds: 3,
                        );
                        final ok = _reader!.isOpen;
                        if (!mounted) return;
                        setState(
                          () => _status = ok
                              ? 'RS232 Open OK (0x${_reader!.handleAddress.toRadixString(16)})'
                              : 'RS232 Open FAILED',
                        );
                        if (ok) {
                          _reader!.registerCallbacks();
                          _listenReaderEvents();
                        }
                      } catch (e) {
                        if (!mounted) return;
                        setState(() => _status = 'RS232 Open error: $e');
                      }
                    },
                    child: const Text('Open RS232'),
                  ),
                  const SizedBox(width: 12),
                  ElevatedButton(
                    onPressed: () async {
                      setState(() => _status = 'Opening TCP...');
                      try {
                        _reader = await GReader.openTcp(
                          _tcpCtrl.text,
                          timeoutSeconds: 3,
                        );
                        final ok = _reader!.isOpen;
                        if (!mounted) return;
                        setState(
                          () => _status = ok
                              ? 'TCP Open OK (0x${_reader!.handleAddress.toRadixString(16)})'
                              : 'TCP Open FAILED',
                        );
                        if (ok) {
                          _reader!.registerCallbacks();
                          _listenReaderEvents();
                        }
                      } catch (e) {
                        if (!mounted) return;
                        setState(() => _status = 'TCP Open error: $e');
                      }
                    },
                    child: const Text('Open TCP'),
                  ),
                  const SizedBox(width: 12),
                  ElevatedButton(
                    onPressed: () async {
                      setState(() => _status = 'Opening RS485...');
                      try {
                        _reader = await GReader.openRs485(
                          _connCtrl.text,
                          timeoutSeconds: 3,
                        );
                        final ok = _reader!.isOpen;
                        if (!mounted) return;
                        setState(
                          () => _status = ok
                              ? 'RS485 Open OK (0x${_reader!.handleAddress.toRadixString(16)})'
                              : 'RS485 Open FAILED',
                        );
                        if (ok) {
                          _reader!.registerCallbacks();
                          _listenReaderEvents();
                        }
                      } catch (e) {
                        if (!mounted) return;
                        setState(() => _status = 'RS485 Open error: $e');
                      }
                    },
                    child: const Text('Open RS485'),
                  ),
                  const SizedBox(width: 12),
                  ElevatedButton(
                    onPressed: _openingHid
                        ? null
                        : () async {
                            if (_selectedHid == null) return;
                            setState(() {
                              _openingHid = true;
                              _status = 'Opening USB HID...';
                            });
                            Timer? watchdog;
                            try {
                              final completer = Completer<GReader?>();
                              // start open
                              GReader.openUsbHid(_selectedHid!)
                                  .then((r) {
                                    if (!completer.isCompleted) {
                                      completer.complete(r);
                                    }
                                  })
                                  .catchError((e, st) {
                                    if (!completer.isCompleted) {
                                      completer.complete(null);
                                    }
                                  });
                              // timeout fallback (8s)
                              watchdog = Timer(const Duration(seconds: 8), () {
                                if (!completer.isCompleted) {
                                  completer.complete(null);
                                }
                              });
                              final r = await completer.future;
                              watchdog.cancel();
                              if (!mounted) return;
                              if (r != null && r.isOpen) {
                                _reader = r;
                                _reader!.registerCallbacks();
                                _listenReaderEvents();
                                setState(() {
                                  _status = 'USB HID Open OK';
                                });
                              } else {
                                _reader = null;
                                setState(() {
                                  _status = 'USB HID Open FAILED';
                                });
                              }
                            } finally {
                              if (mounted) setState(() => _openingHid = false);
                            }
                          },
                    child: const Text('Open HID (safe)'),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              Wrap(
                spacing: 8,
                runSpacing: 8,
                children: [
                  ElevatedButton(
                    onPressed: () async {
                      if (_reader?.isOpen != true) return;
                      if (_busy) return;
                      setState(() {
                        _busy = true;
                        _status = 'Closing...';
                      });
                      try {
                        // 发一条诊断，标记手动断开
                        try {
                          GReaderFfi.instance.diagEmit(
                            '{"type":"DartManualDisconnect","ts":${DateTime.now().millisecondsSinceEpoch}}',
                          );
                        } catch (_) {}
                        // 尝试快速 BaseStop，避免设备保持在盘点态
                        try {
                          await _reader!.baseStopAsync().timeout(
                            const Duration(milliseconds: 800),
                          );
                        } catch (_) {}
                        await _evtSub?.cancel();
                        _evtSub = null;
                        // 关闭并清理
                        _reader!.close();
                        _reader = null;
                        if (!mounted) return;
                        setState(() => _status = 'Disconnected');
                      } finally {
                        if (mounted) setState(() => _busy = false);
                      }
                    },
                    child: const Text('Disconnect'),
                  ),
                  ElevatedButton(
                    onPressed: () {
                      if (_reader?.isOpen != true) return;
                      final r = _reader!.baseStop();
                      setState(
                        () => _status =
                            'BaseStop code=${r.code} ${r.error ?? ''}',
                      );
                    },
                    child: const Text('BaseStop'),
                  ),
                  ElevatedButton(
                    onPressed: _busy
                        ? null
                        : () async {
                            if (_reader?.isOpen != true) return;
                            setState(() => _busy = true);
                            try {
                              final r = await _reader!.setPowerAsync(
                                antennaNo: 1,
                                power: 20,
                              );
                              if (!mounted) return;
                              setState(
                                () => _status =
                                    'SetPower code=${r.code} ${r.error ?? ''}',
                              );
                            } finally {
                              if (mounted) setState(() => _busy = false);
                            }
                          },
                    child: const Text('SetPower(1,20)'),
                  ),
                  ElevatedButton(
                    onPressed: _busy
                        ? null
                        : () async {
                            if (_reader?.isOpen != true) return;
                            setState(() => _busy = true);
                            try {
                              final maskAnt1 = 0x1; // AntennaNo_1
                              final r = await _reader!.inventoryEpcStartAsync(
                                antennaEnable: maskAnt1,
                                inventoryMode: 1,
                                filterArea: -1,
                                readTidLen: 6, // 读取6个word（12字节）的TID，事件里才会有 tid
                              );
                              if (!mounted) return;
                              setState(
                                () => _status =
                                    'InvEPC code=${r.code} ${r.error ?? ''}',
                              );
                            } finally {
                              if (mounted) setState(() => _busy = false);
                            }
                          },
                    child: const Text('Inventory EPC'),
                  ),
                  ElevatedButton(
                    onPressed: () {
                      if (_reader?.isOpen != true) return;
                      final maskAnt1 = 0x1;
                      final r = _reader!.inventoryGbStart(
                        antennaEnable: maskAnt1,
                        inventoryMode: 1,
                      );
                      setState(
                        () => _status = 'InvGB code=${r.code} ${r.error ?? ''}',
                      );
                    },
                    child: const Text('Inventory GB'),
                  ),
                  ElevatedButton(
                    onPressed: () {
                      if (_reader?.isOpen != true) return;
                      final maskAnt1 = 0x1;
                      final r = _reader!.inventoryGjbStart(
                        antennaEnable: maskAnt1,
                        inventoryMode: 1,
                      );
                      setState(
                        () =>
                            _status = 'InvGJB code=${r.code} ${r.error ?? ''}',
                      );
                    },
                    child: const Text('Inventory GJB'),
                  ),
                  ElevatedButton(
                    onPressed: () {
                      if (_reader?.isOpen != true) return;
                      final maskAnt1 = 0x1;
                      final r = _reader!.inventoryTlStart(
                        antennaEnable: maskAnt1,
                        inventoryMode: 1,
                      );
                      setState(
                        () => _status = 'InvTL code=${r.code} ${r.error ?? ''}',
                      );
                    },
                    child: const Text('Inventory TL'),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              const Text('Write/Lock EPC'),
              Row(
                children: [
                  Expanded(
                    child: TextField(
                      controller: _epcHexCtrl,
                      decoration: const InputDecoration(
                        border: OutlineInputBorder(),
                        labelText: 'EPC Hex',
                      ),
                    ),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: () {
                      if (_reader?.isOpen != true) return;
                      final tid = _lastTid;
                      final epc = _epcHexCtrl.text.trim();
                      final cleanEpc = epc.replaceAll(
                        RegExp(r"[^0-9A-Fa-f]"),
                        '',
                      );
                      if (cleanEpc.isEmpty) {
                        setState(() => _status = 'EPC不能为空');
                        return;
                      }
                      if (cleanEpc.length % 4 != 0) {
                        setState(() => _status = 'EPC长度必须为4的倍数(16位字对齐)');
                        return;
                      }
                      if (cleanEpc.length > 512) {
                        setState(() => _status = 'EPC过长，最大支持512个hex字符(256字节)');
                        return;
                      }
                      final r = _reader!.writeEpcWithPc(
                        antennaEnable: 0x1,
                        epcHex: epc,
                        passwordHex: _pwdHexCtrl.text.trim(),
                        block: 0,
                        filterArea: 2,
                        filterHex: tid,
                        filterBitStart: 0,
                      );
                      setState(() {
                        _status =
                            'WriteEPC (Filter TID:${tid ?? '-'}) code=${r.code} ${r.error ?? ''}';
                      });
                    },
                    child: const Text('WriteEPC (Filter TID)'),
                  ),
                  const SizedBox(width: 8),
                  SizedBox(
                    width: 150,
                    child: TextField(
                      controller: _pwdHexCtrl,
                      decoration: const InputDecoration(
                        border: OutlineInputBorder(),
                        labelText: 'Pwd (hex)',
                      ),
                    ),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: () {
                      if (_reader?.isOpen != true) return;
                      final epc = _epcHexCtrl.text.trim();
                      final cleanEpc = epc.replaceAll(
                        RegExp(r"[^0-9A-Fa-f]"),
                        '',
                      );
                      if (cleanEpc.isEmpty) {
                        setState(() => _status = 'EPC不能为空');
                        return;
                      }
                      if (cleanEpc.length % 4 != 0) {
                        setState(() => _status = 'EPC长度必须为4的倍数(16位字对齐)');
                        return;
                      }
                      if (cleanEpc.length > 512) {
                        setState(() => _status = 'EPC过长，最大支持512个hex字符(256字节)');
                        return;
                      }
                      final r = _reader!.writeEpcWithPc(
                        antennaEnable: 0x1,
                        epcHex: epc,
                        passwordHex: _pwdHexCtrl.text.trim(),
                        block: 0,
                      );
                      setState(
                        () => _status =
                            'WriteEPC code=${r.code} ${r.error ?? ''}',
                      );
                    },
                    child: const Text('WriteEPC'),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: () {
                      if (_reader?.isOpen != true) return;
                      // mode per SDK: simple example use 0
                      final r = _reader!.lockEpc(
                        antennaEnable: 0x1,
                        mode: 0,
                        passwordHex: _pwdHexCtrl.text.trim(),
                      );
                      setState(
                        () =>
                            _status = 'LockEPC code=${r.code} ${r.error ?? ''}',
                      );
                    },
                    child: const Text('LockEPC'),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              Text('Status: $_status'),
              const SizedBox(height: 12),
              const Text('Events:'),
              Row(
                children: [
                  ElevatedButton(
                    onPressed: () {
                      GReaderFfi.instance.diagEmit(
                        '{"type":"TestDiag","ts":${DateTime.now().millisecondsSinceEpoch}}',
                      );
                      setState(() => _status = 'Emit TestDiag');
                    },
                    child: const Text('Emit Test Diag'),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: _events.isEmpty
                        ? null
                        : () {
                            Clipboard.setData(
                              ClipboardData(text: _events.join('\n')),
                            );
                            ScaffoldMessenger.of(context).hideCurrentSnackBar();
                            ScaffoldMessenger.of(context).showSnackBar(
                              SnackBar(
                                content: Text('已复制全部日志（${_events.length} 条）'),
                              ),
                            );
                          },
                    child: const Text('复制全部日志'),
                  ),
                ],
              ),
              Expanded(
                child: Container(
                  decoration: BoxDecoration(
                    border: Border.all(color: Colors.black12),
                  ),
                  child: ListView.builder(
                    itemCount: _events.length,
                    itemBuilder: (_, i) {
                      final e = _events[i];
                      // 简单高亮：
                      // - EPC 日志: 绿色
                      // - 诊断事件 HidOpenDiag: 橙色并加粗
                      // - 断开/移除: 红色
                      Color color = Colors.black87;
                      FontWeight weight = FontWeight.normal;
                      if (e.contains('"type":"TagEpcLog"')) {
                        color = Colors.green[800]!;
                      } else if (e.contains('"type":"HidOpenDiag"')) {
                        color = Colors.orange[800]!;
                        weight = FontWeight.w600;
                      } else if (e.contains('Disconnected') ||
                          e.contains('Removed')) {
                        color = Colors.red[800]!;
                        weight = FontWeight.w600;
                      }
                      return Padding(
                        padding: const EdgeInsets.symmetric(
                          horizontal: 8,
                          vertical: 4,
                        ),
                        child: Row(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Expanded(
                              child: SelectableText(
                                e,
                                style: TextStyle(
                                  color: color,
                                  fontWeight: weight,
                                ),
                              ),
                            ),
                            IconButton(
                              icon: const Icon(Icons.copy, size: 16),
                              tooltip: '复制此行',
                              onPressed: () {
                                Clipboard.setData(ClipboardData(text: e));
                                ScaffoldMessenger.of(
                                  context,
                                ).hideCurrentSnackBar();
                                ScaffoldMessenger.of(context).showSnackBar(
                                  const SnackBar(content: Text('已复制日志行')),
                                );
                              },
                            ),
                          ],
                        ),
                      );
                    },
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  void _listenReaderEvents() {
    _evtSub?.cancel();
    if (_reader?.isOpen == true) {
      // 使用强类型事件
      _evtSub = _reader!.onEventTyped((ev) {
        // 仍保留原始 JSON 展示
        _pushEventUnique(
          convert.jsonEncode(
            ev.raw.isEmpty ? {'type': ev.kind.toString()} : ev.raw,
          ),
        );
        if (ev is GTagEpcLogEvent && ev.tid != null && ev.tid!.isNotEmpty) {
          _lastTid = ev.tid;
        }
        if (mounted) setState(() {});
      });
    }
  }

  void _pushEventUnique(String e) {
    if (_events.isNotEmpty && _events.first == e) return;
    _events.insert(0, e);
    if (_events.length > 200) _events.removeLast();
  }
}
