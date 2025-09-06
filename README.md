# greader_plugin (Windows)

高性能的 Flutter Windows 插件，封装厂商 GReader SDK，通过 Dart FFI 提供稳定的 HID/串口/TCP 连接、盘点、写/锁 EPC 等能力，并内置丰富诊断日志与异常场景处理。

> 平台与依赖
>
> - 仅支持 Windows（x64）。
> - 需要 Visual Studio 2022 C++ 工具链（编译示例时）。
> - 插件内已内置厂商 SDK（windows/vendor/greader_sdk），构建时自动链接并拷贝运行库。
> - 分发/上传时只需提交 `greader_plugin/` 文件夹即可。

## 快速开始

- 列出 HID 并打开：
	```dart
	final devices = GReader.listUsbHid();
	final reader = await GReader.openUsbHid(devices.first);
	```
- 盘点 EPC（同时读取 TID，便于后续过滤写入）：
	```dart
	final reader = await GReader.openUsbHid(devPath);
	final inv = await reader.inventoryEpcStartAsync(
		antennaEnable: 0x1,
		readTidLen: 6, // 读取6字(96bit) TID，事件里才会包含 tid
	);
	// 推荐：通过回调(流)接收事件，而非手动轮询
	// 取消订阅
	await sub.cancel();
		// 典型事件: {"type":"TagEpcLog","epc":"...","tid":"...","ant":1,"rssi":-40}

	#### 强类型事件进阶示例：提取 TID 并定向写 EPC

	```dart
	final reader = await GReader.openUsbHid(devPath);
	await reader.inventoryEpcStartAsync(antennaEnable: 0x1, readTidLen: 6);

	String? lastTid;
	final sub = reader.onEventTyped((ev) {
		if (ev is GTagEpcLogEvent && (ev.tid?.isNotEmpty ?? false)) {
			lastTid = ev.tid; // 缓存最近一次盘点到的 TID
		}
	});

	// 需要写入时（推荐 PC+EPC，从 startWord=1，并用 TID 过滤只写目标标签）
	final epcHex = '300833B2DDD901400000000D';
	if (lastTid == null) {
		print('尚未读取到 TID，先进行盘点');
	} else {
		final r = await reader.writeEpcWithPcAsync(
			antennaEnable: 0x1,
			epcHex: epcHex,
			passwordHex: '00000000',
			block: 0,            // 建议整段写
			filterArea: 2,       // 2 = TID
			filterHex: lastTid,  // 仅写入匹配该 TID 的标签
			filterBitStart: 0,
		);
		print('WriteEPC code=${r.code} ${r.error ?? ''}');
	}

	await sub.cancel();
	```
	});

	// ...需要时取消订阅
	await sub.cancel();
	```
- 停止：
	```dart
	await reader.baseStopAsync();
	```
- 写 EPC（推荐：PC+EPC，从字地址1开始，使用 TID 过滤只写目标标签）：
	```dart
	// 假设最近一次盘点缓存到的 tidHex
	final r = await reader.writeEpcWithPcAsync(
		antennaEnable: 0x1,
		epcHex: '300833B2DDD901400000000D',
		passwordHex: '00000000', // 可空字符串，表示无密码
		block: 0,                 // 建议0，避免分块写失败
		filterArea: 2,            // 2 = TID
		filterHex: tidHex,
		filterBitStart: 0,
	);
	if (r.code != 0) {
		// r.error 为底层返回的失败原因
	}
	```
- 锁 EPC：
	```dart
	final r = await reader.lockEpcAsync(
		antennaEnable: 0x1,
		area: 1,        // EPC 区
		mode: 0,        // 厂商定义的锁定模式
		passwordHex: '00000000',
		// 可选 TID 过滤
	);
	```

## 事件与诊断

- `TagEpcLog`：携带 `epc`, `tid`, `ant`, `rssi` 等。
- `TagEpcOver`：一轮 EPC 盘点结束。
- `TagGbLog`/`TagGbOver`、`TagGjbLog`/`TagGjbOver`、`TagTLLog`/`TagTLOver`、`Tag6bLog`/`Tag6bOver`、`Tag6DLog`/`Tag6DOver`。
- `GpiStart`/`GpiOver`。
- `TcpDisconnected` / `UsbHidRemoved`：连接断开/设备拔出。
- 通过 `GReader.setVerboseLogging(true)` 打开/关闭详细日志。

### 回调式（Stream）用法

无需手动轮询，可注册监听：

```dart
final reader = await GReader.openUsbHid(devPath);

// 业务事件
final sub = reader.onEvent((json) {
	// 处理 TagEpcLog / TagEpcOver / 断开等事件
});

// 诊断事件（全局）
final diagSub = GReader.onDiag((json) {
	// 打印或保存诊断
});

// 停止监听
await sub.cancel();
await diagSub.cancel();
```

### 强类型事件（推荐）

无需自己解析 JSON，使用内置类型模型：

```dart
final reader = await GReader.openUsbHid(devPath);
await reader.inventoryEpcStartAsync(antennaEnable: 0x1, readTidLen: 6);

final sub = reader.onEventTyped((ev) {
	switch (ev.kind) {
		case GEventKind.tagEpcLog:
			final e = ev as GTagEpcLogEvent;
			print('EPC=${e.epc} TID=${e.tid} ant=${e.ant} rssi=${e.rssi}');
			break;
		case GEventKind.tcpDisconnected:
		case GEventKind.usbHidRemoved:
			// 断开/拔出
			break;
		default:
			// 其它事件可使用 ev.raw 读取原始字段
			break;
	}
});

// 取消订阅	nawait sub.cancel();
```

## API 速览（高层 GReader）

- 打开/关闭（参数说明）
	- `static Future<GReader> openUsbHid(String path, {int timeoutSeconds=6})`
		- path: USBHID 设备路径（可通过 `listUsbHid()` 获取）；
		- timeoutSeconds: 连接确认超时（秒）。
	- `static Future<GReader> openSerial(String conn, {int timeoutSeconds=3})`
		- conn: 串口连接字符串，如 `COM3:115200`；
		- timeoutSeconds: 秒。
	- `static Future<GReader> openTcp(String hostPort, {int timeoutSeconds=3})`
		- hostPort: `ip:port`（默认 8160）；
		- timeoutSeconds: 秒。
	- `static Future<GReader> openRs485(String conn, {int timeoutSeconds=3})`
		- conn: `COMx:baud:addr`；
		- timeoutSeconds: 秒。
	- `static List<String> listUsbHid()` 返回 HID 设备路径列表。
	- `void close()` 关闭连接并释放资源。
	- 状态：`isOpen`, `isClosed`, `handleAddress`

- 事件
	- `void registerCallbacks()`（通常在打开成功后立即调用）
	- `String? nextEventJson()`（兼容用途，已不推荐）
	- `StreamSubscription<String> onEvent(void Function(String) listener)` / `Stream<String> events()`
	- `StreamSubscription<GEvent> onEventTyped(void Function(GEvent) listener)` / `Stream<GEvent> eventsTyped()`
	- 事件模型：`GEventKind`、`GEvent`、`GTagEpcLogEvent`、`GSimpleEvent`

- 基础操作
	- `({int code, String? error}) baseStop()` / `Future<...> baseStopAsync()`
	- `({int code, String? error}) setPower({int antennaNo=1, required int power})` / `Future<...> setPowerAsync(...)`

- 盘点（参数说明）
		- `({int code, String? error}) inventoryEpcStart({
				required int antennaEnable,
				int inventoryMode=1,
				int filterArea=-1,
				String? filterHex,
				int filterBitStart=0,
				int readTidLen=0,
				int timeoutMs=0,
			})`
				- antennaEnable: 天线位图（0x1=天线1，0x3=1+2 ...）；
				- inventoryMode: 0=单次；1=连续；
				- filterArea: -1=不使用；0=保留；1=EPC；2=TID；3=用户；
				- filterHex: 选择内容（HEX），为空表示不使用过滤；
				- filterBitStart: 匹配起始位（匹配 EPC 常用 32）；
				- readTidLen: TID 读取字数（0=不读；>0 指定，常用 6）；
				- timeoutMs: 指令超时（毫秒，0=默认）。
		- `Future<...> inventoryEpcStartAsync({...})` 异步版本。
		- `inventoryGbStart`, `inventoryGjbStart`, `inventoryTlStart`（国标/军标/TL），参数为：天线位图/模式/超时。

- EPC 写/锁（参数说明与建议）
	- `static String computePcHexForEpc(String epcHex)`：根据 EPC 字数计算 PC（4 个 HEX）。
	- `writeEpcWithPc({required int antennaEnable, required String epcHex, String passwordHex='', int block=0, int stayCw=0, int filterArea=-1, String? filterHex, int filterBitStart=0})`
		- 从 `startWord=1` 写入 PC+EPC（推荐）；
		- 建议过滤：`filterArea=2, filterHex=tid, filterBitStart=0`（按 TID 精确选中目标标签）。
	- `writeEpc({required int antennaEnable, int area=1, int startWord=2, required String hexData, String passwordHex='', int block=1, int stayCw=0, int filterArea=-1, String? filterHex, int filterBitStart=0})`
		- 直写接口：area=EPC(1)；写 PC+EPC 时从 `startWord=1`；
		- block: 0=整段写（建议）；非 0=分块写；
		- passwordHex: 8 个 HEX，可空。
	- `lockEpc({required int antennaEnable, int area=1, required int mode, String passwordHex='', int filterArea=-1, String? filterHex, int filterBitStart=0})`
		- area: 0=灭活密码；1=访问密码；2=EPC；3=TID；4=用户；
		- mode: 0=解锁；1=锁定；2=永久解锁；3=永久锁定；
		- 建议：按 TID 过滤，避免误锁。

- 诊断
	- `static void setVerboseLogging(bool enabled)`
	- `static String? nextDiagEventJson()`（兼容用途，已不推荐）
	- `static StreamSubscription<String> onDiag(void Function(String) listener)` / `static Stream<String> diagEvents()`

## 状态与实时信息（Status / Realtime）

提供轻量的状态查询与丰富的实时快照，便于做 UI 展示与自检。

### 基础状态 getStatus()

返回当前句柄状态与连接信息（不会阻塞）：

```dart
final st = await reader.getStatus();
// 形如：
// {
//   "connected": true,
//   "readerName": "COM3:115200" | "\\\\?\\hid#vid_...", // 打开时的标识
//   "transport": "rs232" | "tcp" | "rs485" | "usbhid",
//   "isUsbHid": true/false
// }
```

### 实时快照 getRealtime()

在串行工作线程上依次查询多项信息并合并为 JSON（小体量、数百字节量级）：

```dart
final rt = await reader.getRealtime();
// 示例返回（字段会随固件而异）：
// {
//   "connected": true,
//   "readerName": "...",
//   "transport": "usbhid",
//   "isUsbHid": true,
//   "capabilities": { "maxPower": 30, "minPower": 5, "antennaCount": 4 },
//   "power": [ { "antenna": 1, "read": 20 }, { "antenna": 2, "read": 20 } ],
//   "freqRangeIndex": 2,
//   "baseband": { "baseSpeed": 1, "qValue": 4, "session": 1, "inventoryFlag": 2 },
//   "gpi": [ { "port": 1, "level": 0 } ],
//   "readerInfo": { "serial": "SN...", "appVersion": "0.1.0.0", "powerOnTime": "2025-09-06 ..." },
//   "pendingEvents": 0
// }
```

提示与排错：
- 必须先成功打开设备；对 HID 设备建议在 open 后稍作延时（插件已在原生侧做最小等待）。
- 若只看到 `{"connected": true}`，多半是 JSON 被上层兜底（例如路径中含反斜杠/非 UTF-8 导致解析失败，现已在原生侧做了 UTF‑8 转换与转义）。请更新为当前版本并重试。
- 少数字段取决于固件能力，旧固件可能缺省或为默认值。


## 速查卡（参数对照）📌

与开发指南一致的核心参数映射，适用于 Dart 高层与 FFI 层 API：

- 天线位图 antennaEnable
	- 按位启用天线（0x1=1 号，0x2=2 号，0x3=1+2，类推）。

- 过滤参数（选择器）
	- filterArea：-1 不使用；0 保留；1 EPC；2 TID；3 用户。
	- filterHex：匹配内容（HEX 字符串）。
	- filterBitStart：起始位，匹配 EPC 时常用 32。

- TID 读取（盘点可选项）
	- readTidLen：单位为字（word=16bit）；0 不读，>0 指定字数（常用 6）。

- 写 EPC
	- area：1=EPC 区（默认）。
	- startWord：1=PC，2=EPC 首字；写 PC+EPC 时推荐从 1 开始。
	- passwordHex：访问密码（8 个 HEX，32bit），空串为无密码。
	- block：0 整段写（建议）；非 0 分块写。
	- stayCw：0 正常收尾；1 保持载波（通常不需要）。

- 锁 EPC
	- area：0 灭活密码；1 访问密码；2 EPC；3 TID；4 用户。
	- mode：0 解锁；1 锁定；2 永久解锁；3 永久锁定。

提示：为避免多标签误写/误锁，强烈建议按 TID 精确过滤（filterArea=2, filterHex=tid, filterBitStart=0）。

### 生命周期钩子（Hooks）

无需解析事件即可感知连接/断开：

```dart
reader.onConnect().listen((_) {
	// 已连接，可刷新状态/实时信息
});
reader.onDisconnect().listen((_) {
	// 已断开（含 TCP 断线 / HID 拔出）
});
```

## 关键注意事项（必读）

- 仅 Windows 支持；HID 打开在主 Isolate 上执行以满足驱动线程约束。
- 写 EPC 前建议执行 `baseStop` 停止盘点，避免“设备忙”。
- 标签选择：为避免多标签误写，强烈建议用 TID 过滤（`filterArea=2, filterHex=tid, filterBitStart=0`）。
- EPC 写入规范：
	- 使用 `writeEpcWithPc*`（会自动计算 PC，并从 `startWord=1` 写入）。
	- EPC 仅限十六进制；会自动清洗空格、分隔符并转为大写。
	- 长度要求：必须为 4 的倍数（16 位字对齐）。
	- 长度上限：不超过 512 个 hex 字符（256 字节）。超限时立即返回错误并产生日志。
	- `block` 建议为 `0`（整段写入），`stayCw` 一般为 `0`。
- 密码：`passwordHex` 为访问密码（8 个 hex，32bit）；空串表示无密码。
- 天线：`antennaEnable` 为位图（如 `0x1` 表示天线1，`0x1|0x2` 表示 1+2）。
- 事件消费：推荐使用回调（Stream）订阅；`nextEventJson/nextDiagEventJson` 仅作兼容备用，不再推荐轮询。
- 错误码：
	- `-1`：读写器未打开；
	- `-2`：Dart 侧参数校验失败（如 EPC 非法/超长/为空）；
	- 其他：底层 SDK 的 `RtCode`；具体原因见 `error` 文本（如 `Other error`）。

## 常见问题

- 首次连接 HID 即收到 `UsbHidRemoved`？
	- 插件已在原生侧做存在性多次采样与宽限时间过滤；若仍出现，请检查 USB 线缆/集线器与供电。

- 写 EPC 返回 `Other error`？
	- 确认已 `baseStop` 后再写；
	- 使用 `writeEpcWithPc*` + `block=0`；
	- 单标签场景或使用 TID 过滤；
	- 确认 `passwordHex` 正确；
	- 观察诊断 `DartWriteEpcStart/Result` 与 `[GReaderDiag] Send/Result`。

## 示例工程

本仓库内含 `example/` Flutter 桌面程序，演示 HID/TCP 打开、盘点、写/锁 EPC 与诊断日志查看。示例已采用回调（Stream）实现事件与诊断展示。直接构建运行：

```powershell
# 在 example 目录构建 Windows Release
flutter build windows --release
# 运行（或直接在 VS Code/IDE 启动）
"c:\\Users\\renren\\Desktop\\c_c++-api\\greader_plugin\\example\\build\\windows\\x64\\runner\\Release\\greader_plugin_example.exe"
```

> 提示：构建前请关闭正在运行的示例，避免 GReader.dll 被占用导致复制失败。

## 版本与兼容性

- Flutter 3.35+，Dart 3.9+（示例环境）。
- 如需其它平台支持，请提交 Issue 以评估。

