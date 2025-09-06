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
  // Antenna mask state
  int _antennaCount = 0;
  int _antennaMask = 0x1; // 默认至少启用天线1
  // Simple inputs for write/lock
  final _epcHexCtrl = TextEditingController(text: '300833B2DDD9014000000000');
  final _pwdHexCtrl = TextEditingController(text: '00000000');
  String? _lastTid;

  // 将 Map/String JSON 美化为多行缩进文本
  String _prettyJson(dynamic data) {
    try {
      final encoder = convert.JsonEncoder.withIndent('  ');
      if (data is String) {
        final obj = convert.jsonDecode(data);
        return encoder.convert(obj);
      }
      return encoder.convert(data);
    } catch (_) {
      return data?.toString() ?? '';
    }
  }

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

  Future<void> _refreshAntennaCount() async {
    if (_reader?.isOpen == true) {
      final n = await _reader!.getAntennaCount();
      if (!mounted) return;
      setState(() {
        _antennaCount = n > 0 ? n.clamp(1, 16) : 0;
        if (_antennaCount > 0) {
          final limit = (1 << _antennaCount) - 1;
          _antennaMask &= limit;
          if (_antennaMask == 0) _antennaMask = 0x1;
        } else {
          _antennaMask = 0x1;
        }
      });
    }
  }

  Widget _buildAntennaMask() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          children: [
            const Text('Antenna Mask'),
            const SizedBox(width: 8),
            OutlinedButton(
              onPressed: _reader?.isOpen == true ? _refreshAntennaCount : null,
              child: const Text('Refresh'),
            ),
            const SizedBox(width: 8),
            OutlinedButton(
              onPressed: _antennaCount > 0
                  ? () {
                      setState(() {
                        _antennaMask = (1 << _antennaCount) - 1;
                      });
                    }
                  : null,
              child: const Text('All'),
            ),
            const SizedBox(width: 8),
            OutlinedButton(
              onPressed: _antennaCount > 0
                  ? () {
                      setState(() {
                        _antennaMask = 0;
                      });
                    }
                  : null,
              child: const Text('None'),
            ),
            const SizedBox(width: 12),
            Text(
              'count: $_antennaCount, mask: 0x${_antennaMask.toRadixString(16)}',
            ),
          ],
        ),
        const SizedBox(height: 8),
        if (_antennaCount > 0)
          Wrap(
            spacing: 8,
            runSpacing: 4,
            children: List.generate(_antennaCount, (i) {
              final bit = 1 << i;
              final checked = (_antennaMask & bit) != 0;
              return Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Checkbox(
                    value: checked,
                    onChanged: (v) {
                      setState(() {
                        if (v == true) {
                          _antennaMask |= bit;
                        } else {
                          _antennaMask &= ~bit;
                        }
                        if (_antennaMask == 0) _antennaMask = bit; // 至少保留一个
                      });
                    },
                  ),
                  Text('ANT${i + 1}'),
                ],
              );
            }),
          )
        else
          const Text('Click Refresh to load antenna count.'),
      ],
    );
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
                          unawaited(_refreshAntennaCount());
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
                          unawaited(_refreshAntennaCount());
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
                          unawaited(_refreshAntennaCount());
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
                                unawaited(_refreshAntennaCount());
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
              // Antenna selection UI
              _buildAntennaMask(),
              const SizedBox(height: 12),
              Wrap(
                spacing: 8,
                runSpacing: 8,
                children: [
                  ElevatedButton(
                    onPressed: _busy || _reader?.isOpen != true
                        ? null
                        : () async {
                            setState(() => _busy = true);
                            try {
                              final st = await _reader!.getStatus();
                              if (!mounted) return;
                              setState(() {
                                _status =
                                    'Status: ${convert.jsonEncode(st ?? {'connected': false})}';
                              });
                              _pushEventUnique(
                                convert.jsonEncode({
                                  'type': 'Status',
                                  'data': st ?? {'connected': false},
                                }),
                              );
                            } finally {
                              if (mounted) setState(() => _busy = false);
                            }
                          },
                    child: const Text('Query Status'),
                  ),
                  ElevatedButton(
                    onPressed: _busy || _reader?.isOpen != true
                        ? null
                        : () async {
                            setState(() => _busy = true);
                            try {
                              final n = await _reader!.getAntennaCount();
                              if (!mounted) return;
                              setState(() {
                                _status = 'AntennaCount: $n';
                              });
                              _pushEventUnique(
                                convert.jsonEncode({
                                  'type': 'AntennaCount',
                                  'data': n,
                                }),
                              );
                            } finally {
                              if (mounted) setState(() => _busy = false);
                            }
                          },
                    child: const Text('Get AntennaCount'),
                  ),
                  ElevatedButton(
                    onPressed: _busy || _reader?.isOpen != true
                        ? null
                        : () async {
                            setState(() => _busy = true);
                            try {
                              final rt = await _reader!.getRealtime();
                              if (!mounted) return;
                              setState(() {
                                _status =
                                    'Realtime:\n${_prettyJson(rt ?? {'connected': false})}';
                              });
                              _pushEventUnique(
                                convert.jsonEncode({
                                  'type': 'Realtime',
                                  'data': rt ?? {'connected': false},
                                }),
                              );
                            } finally {
                              if (mounted) setState(() => _busy = false);
                            }
                          },
                    child: const Text('Query Realtime'),
                  ),
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
                              final n = await _reader!.getAntennaCount();
                              // Use selected mask; bound by count if known
                              int mask = _antennaMask;
                              if (n > 0) {
                                final capped = n.clamp(1, 16);
                                mask &= (1 << capped) - 1;
                                if (mask == 0) mask = 0x1;
                              } else if (mask == 0) {
                                mask = 0x1;
                              }
                              final r = await _reader!.inventoryEpcStartAsync(
                                antennaEnable: mask,
                                inventoryMode: 1,
                                filterArea: -1,
                                readTidLen: 6, // 读取6个word（12字节）的TID，事件里才会有 tid
                              );
                              if (!mounted) return;
                              setState(
                                () => _status =
                                    'InvEPC code=${r.code} ${r.error ?? ''} (mask=0x${mask.toRadixString(16)})',
                              );
                            } finally {
                              if (mounted) setState(() => _busy = false);
                            }
                          },
                    child: const Text('Inventory EPC'),
                  ),
                  ElevatedButton(
                    onPressed: () async {
                      if (_reader?.isOpen != true) return;
                      final n = await _reader!.getAntennaCount();
                      int mask = _antennaMask;
                      if (n > 0) {
                        final capped = n.clamp(1, 16);
                        mask &= (1 << capped) - 1;
                        if (mask == 0) mask = 0x1;
                      } else if (mask == 0) {
                        mask = 0x1;
                      }
                      final r = _reader!.inventoryGbStart(
                        antennaEnable: mask,
                        inventoryMode: 1,
                      );
                      setState(
                        () => _status =
                            'InvGB code=${r.code} ${r.error ?? ''} (mask=0x${mask.toRadixString(16)})',
                      );
                    },
                    child: const Text('Inventory GB'),
                  ),
                  ElevatedButton(
                    onPressed: () async {
                      if (_reader?.isOpen != true) return;
                      final n = await _reader!.getAntennaCount();
                      int mask = _antennaMask;
                      if (n > 0) {
                        final capped = n.clamp(1, 16);
                        mask &= (1 << capped) - 1;
                        if (mask == 0) mask = 0x1;
                      } else if (mask == 0) {
                        mask = 0x1;
                      }
                      final r = _reader!.inventoryGjbStart(
                        antennaEnable: mask,
                        inventoryMode: 1,
                      );
                      setState(
                        () => _status =
                            'InvGJB code=${r.code} ${r.error ?? ''} (mask=0x${mask.toRadixString(16)})',
                      );
                    },
                    child: const Text('Inventory GJB'),
                  ),
                  ElevatedButton(
                    onPressed: () async {
                      if (_reader?.isOpen != true) return;
                      final n = await _reader!.getAntennaCount();
                      int mask = _antennaMask;
                      if (n > 0) {
                        final capped = n.clamp(1, 16);
                        mask &= (1 << capped) - 1;
                        if (mask == 0) mask = 0x1;
                      } else if (mask == 0) {
                        mask = 0x1;
                      }
                      final r = _reader!.inventoryTlStart(
                        antennaEnable: mask,
                        inventoryMode: 1,
                      );
                      setState(
                        () => _status =
                            'InvTL code=${r.code} ${r.error ?? ''} (mask=0x${mask.toRadixString(16)})',
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
                      // 使用 UI 选中的天线掩码
                      int mask = _antennaMask;
                      if (_antennaCount > 0) {
                        final capped = _antennaCount.clamp(1, 16);
                        mask &= (1 << capped) - 1;
                        if (mask == 0) mask = 0x1;
                      } else if (mask == 0) {
                        mask = 0x1;
                      }
                      final r = _reader!.writeEpcWithPc(
                        antennaEnable: mask,
                        epcHex: epc,
                        passwordHex: _pwdHexCtrl.text.trim(),
                        block: 0,
                        filterArea: 2,
                        filterHex: tid,
                        filterBitStart: 0,
                      );
                      setState(() {
                        _status = 'WriteEPC (Filter TID:${tid ?? '-'}) code=${r.code} ${r.error ?? ''} (mask=0x${mask.toRadixString(16)})';
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
                      // 使用 UI 选中的天线掩码
                      int mask = _antennaMask;
                      if (_antennaCount > 0) {
                        final capped = _antennaCount.clamp(1, 16);
                        mask &= (1 << capped) - 1;
                        if (mask == 0) mask = 0x1;
                      } else if (mask == 0) {
                        mask = 0x1;
                      }
                      final r = _reader!.writeEpcWithPc(
                        antennaEnable: mask,
                        epcHex: epc,
                        passwordHex: _pwdHexCtrl.text.trim(),
                        block: 0,
                      );
                      setState(
                        () => _status =
                            'WriteEPC code=${r.code} ${r.error ?? ''} (mask=0x${mask.toRadixString(16)})',
                      );
                    },
                    child: const Text('WriteEPC'),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: () {
                      if (_reader?.isOpen != true) return;
                      // mode per SDK: simple example use 0
                      // 使用 UI 选中的天线掩码
                      int mask = _antennaMask;
                      if (_antennaCount > 0) {
                        final capped = _antennaCount.clamp(1, 16);
                        mask &= (1 << capped) - 1;
                        if (mask == 0) mask = 0x1;
                      } else if (mask == 0) {
                        mask = 0x1;
                      }
                      final r = _reader!.lockEpc(
                        antennaEnable: mask,
                        mode: 0,
                        passwordHex: _pwdHexCtrl.text.trim(),
                      );
                      setState(
                        () => _status =
                            'LockEPC code=${r.code} ${r.error ?? ''} (mask=0x${mask.toRadixString(16)})',
                      );
                    },
                    child: const Text('LockEPC'),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              const Text('Status:'),
              const SizedBox(height: 6),
              Container(
                width: double.infinity,
                padding: const EdgeInsets.all(8),
                decoration: BoxDecoration(
                  border: Border.all(color: Colors.black12),
                  color: const Color(0xFFF8F8F8),
                ),
                child: SelectableText(
                  _status,
                  style: const TextStyle(fontFamily: 'monospace', height: 1.25),
                ),
              ),
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
      // Lifecycle hooks (demo):
      _reader!.onConnect().listen((_) {
        _pushEventUnique('{"type":"onConnect"}');
        if (mounted) setState(() {});
      });
      _reader!.onDisconnect().listen((_) {
        _pushEventUnique('{"type":"onDisconnect"}');
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
