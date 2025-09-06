import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'greader_plugin_method_channel.dart';

abstract class GreaderPluginPlatform extends PlatformInterface {
  /// Constructs a GreaderPluginPlatform.
  GreaderPluginPlatform() : super(token: _token);

  static final Object _token = Object();

  static GreaderPluginPlatform _instance = MethodChannelGreaderPlugin();

  /// The default instance of [GreaderPluginPlatform] to use.
  ///
  /// Defaults to [MethodChannelGreaderPlugin].
  static GreaderPluginPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [GreaderPluginPlatform] when
  /// they register themselves.
  static set instance(GreaderPluginPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
