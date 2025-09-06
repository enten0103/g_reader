This folder vendors the Windows GReader SDK to make the plugin self-contained.

Structure:
- include/: vendor headers (GClient.h, message.h, etc.)
- lib_x64/: x64 import library and runtime DLL (GReader.lib/GReader.dll)
- lib_x86/: x86 import library and runtime DLL (GReader.lib/GReader.dll)

Update:
- Replace files here with the official SDK ones when upgrading.
- Or set CMake option GREADER_SDK_DIR to override and use an external SDK.
