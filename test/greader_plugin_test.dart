import 'package:flutter_test/flutter_test.dart';
import 'package:greader_plugin/greader_plugin.dart';
import 'package:greader_plugin/greader_plugin_platform_interface.dart';
import 'package:greader_plugin/greader_plugin_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockGreaderPluginPlatform
    with MockPlatformInterfaceMixin
    implements GreaderPluginPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final GreaderPluginPlatform initialPlatform = GreaderPluginPlatform.instance;

  test('$MethodChannelGreaderPlugin is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelGreaderPlugin>());
  });

  test('getPlatformVersion', () async {
    GreaderPlugin greaderPlugin = GreaderPlugin();
    MockGreaderPluginPlatform fakePlatform = MockGreaderPluginPlatform();
    GreaderPluginPlatform.instance = fakePlatform;

    expect(await greaderPlugin.getPlatformVersion(), '42');
  });
}
