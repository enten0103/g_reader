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

- 打开/关闭
	- `static Future<GReader> openUsbHid(String path, {int timeoutSeconds=6})`
	- `static Future<GReader> openSerial(String conn, {int timeoutSeconds=3})`
	- `static Future<GReader> openTcp(String hostPort, {int timeoutSeconds=3})`
	- `static Future<GReader> openRs485(String conn, {int timeoutSeconds=3})`
	- `static List<String> listUsbHid()`
	- `void close()`
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

- 盘点
	- `({int code, String? error}) inventoryEpcStart({required int antennaEnable, int inventoryMode=1, int filterArea=-1, String? filterHex, int filterBitStart=0, int readTidLen=0, int timeoutMs=0})`
	- `Future<...> inventoryEpcStartAsync({...})`
	- `inventoryGbStart`, `inventoryGjbStart`, `inventoryTlStart`（国标/军标/TL）

- EPC 写/锁
	- `static String computePcHexForEpc(String epcHex)`：根据 EPC 字数计算 PC（4个hex）。
	- `writeEpcWithPc(...)` / `writeEpcWithPcAsync(...)`：推荐入口，自动拼 PC+EPC，从 `startWord=1` 写入。
	- `writeEpc(...)` / `writeEpcAsync(...)`：底层直写接口（高级用法）。
	- `lockEpc(...)` / `lockEpcAsync(...)`：锁 EPC。

- 诊断
	- `static void setVerboseLogging(bool enabled)`
	- `static String? nextDiagEventJson()`（兼容用途，已不推荐）
	- `static StreamSubscription<String> onDiag(void Function(String) listener)` / `static Stream<String> diagEvents()`

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

