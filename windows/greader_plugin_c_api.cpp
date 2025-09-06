#include "include/greader_plugin/greader_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "greader_plugin.h"

void GreaderPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  greader_plugin::GreaderPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
