# greader_plugin_example

示例应用展示如何在 Windows 上使用 greader_plugin：

- 列出并打开 USB HID/RS232/TCP/RS485
- 盘点 EPC（支持读取 TID）
- 基于 TID 过滤写 EPC（推荐 PC+EPC 从字地址1写入）
- 锁 EPC
- 查看实时诊断事件

> 本示例已改为“回调/Stream”模式，不再使用轮询读取事件。

## 运行

```powershell
cd example
flutter pub get
flutter build windows --release
"build\windows\x64\runner\Release\greader_plugin_example.exe"
```

## 关键代码节选（回调式）

```dart
final reader = await GReader.openUsbHid(devPath);
await reader.inventoryEpcStartAsync(antennaEnable: 0x1, readTidLen: 6);

// 业务事件
final sub = reader.onEvent((json) {
	// 例如 {"type":"TagEpcLog","epc":"...","tid":"..."}
});

// 全局诊断
final dsub = GReader.onDiag((json) {
	// 打印或保存诊断
});

// 需要时取消
await sub.cancel();
await dsub.cancel();
```

更多 API 与注意事项请查看仓库根目录的 README。
