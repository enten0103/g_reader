# The Flutter tooling requires that developers have a version of Visual Studio
# installed that includes CMake 3.14 or later. You should not increase this
# version, as doing so will cause the plugin to fail to compile for some
# customers of the plugin.
cmake_minimum_required(VERSION 3.14)

# Project-level configuration.
set(PROJECT_NAME "greader_plugin")
project(${PROJECT_NAME} LANGUAGES CXX)

# Explicitly opt in to modern CMake behaviors to avoid warnings with recent
# versions of CMake.
cmake_policy(VERSION 3.14...3.25)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "greader_plugin_plugin")

# Any new source files that you add to the plugin should be added here.
list(APPEND PLUGIN_SOURCES
  "greader_plugin.cpp"
  "greader_plugin.h"
  "src/greader_api.cpp"
)

# Define the plugin library target. Its name must not be changed (see comment
# on PLUGIN_NAME above).
add_library(${PLUGIN_NAME} SHARED
  "include/greader_plugin/greader_plugin_c_api.h"
  "greader_plugin_c_api.cpp"
  ${PLUGIN_SOURCES}
)

# Apply a standard set of build settings that are configured in the
# application-level CMakeLists.txt. This can be removed for plugins that want
# full control over build settings.
apply_standard_settings(${PLUGIN_NAME})

# Symbols are hidden by default to reduce the chance of accidental conflicts
# between plugins. This should not be removed; any symbols that should be
# exported should be explicitly exported with the FLUTTER_PLUGIN_EXPORT macro.
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)

# Source include directories and library dependencies. Add any plugin-specific
# dependencies here.
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)

# === GReader SDK wiring ===
# Prefer a vendored SDK inside the plugin so the plugin is self-contained.
set(_VENDORED_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/greader_sdk")

# Allow override via environment or CMake option
set(GREADER_SDK_DIR "" CACHE PATH "Path to the GReader Windows SDK root (contains include/, lib_x64/, lib_x86/)")
if(NOT GREADER_SDK_DIR AND EXISTS "${_VENDORED_SDK_DIR}/include/GClient.h")
  set(GREADER_SDK_DIR "${_VENDORED_SDK_DIR}")
endif()
if(NOT GREADER_SDK_DIR AND DEFINED ENV{GREADER_SDK_DIR})
  file(TO_CMAKE_PATH "$ENV{GREADER_SDK_DIR}" GREADER_SDK_DIR)
endif()

# Auto-detect by walking up directories to find a 'windows/include/GClient.h'
if(NOT GREADER_SDK_DIR)
  set(_probe_dir "${CMAKE_CURRENT_LIST_DIR}")
  set(_found FALSE)
  foreach(_i RANGE 1 8)
    get_filename_component(_probe_dir "${_probe_dir}/.." REALPATH)
    if(EXISTS "${_probe_dir}/windows/include/GClient.h")
      set(GREADER_SDK_DIR "${_probe_dir}/windows")
      set(_found TRUE)
      break()
    endif()
  endforeach()
  if(NOT _found)
    message(FATAL_ERROR "GReader SDK not found. Provide SDK via vendor/greader_sdk inside the plugin, or set GREADER_SDK_DIR to the SDK 'windows' folder path (e.g., c:/path/to/c_c++-api/windows)")
  endif()
endif()

set(SDK_ROOT "${GREADER_SDK_DIR}")
set(SDK_INCLUDE_DIR "${SDK_ROOT}/include")

# Select arch-specific lib/dll folder
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(SDK_LIB_DIR "${SDK_ROOT}/lib_x64")
else()
  set(SDK_LIB_DIR "${SDK_ROOT}/lib_x86")
endif()

# Add include paths for wrapper and SDK headers
target_include_directories(${PLUGIN_NAME} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${SDK_INCLUDE_DIR}"
)

# Fallback: also add SDK include dir globally to avoid symlink include issues
include_directories("${SDK_INCLUDE_DIR}")

# Link against prebuilt GReader import library
target_link_libraries(${PLUGIN_NAME} PRIVATE
  "${SDK_LIB_DIR}/GReader.lib"
)

# MSVC: Inject include directory explicitly as a compile option for reliability
if (MSVC)
  target_compile_options(${PLUGIN_NAME} PRIVATE 
  "/I${SDK_INCLUDE_DIR}"
  "/utf-8"
  "/wd4828"  # suppress invalid char warnings from non-UTF8 vendor headers
  )
  # Align with Flutter's wrapper library in Debug to avoid LNK2038.
  # Use _ITERATOR_DEBUG_LEVEL=2 in Debug, 0 in Release.
  target_compile_definitions(${PLUGIN_NAME} PRIVATE $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=2;_HAS_ITERATOR_DEBUGGING=1> $<$<CONFIG:RelWithDebInfo>:_ITERATOR_DEBUG_LEVEL=0;_HAS_ITERATOR_DEBUGGING=0> $<$<CONFIG:Release>:_ITERATOR_DEBUG_LEVEL=0;_HAS_ITERATOR_DEBUGGING=0>)
endif()

# List of absolute paths to libraries that should be bundled with the plugin.
# This list could contain prebuilt libraries, or libraries created by an
# external build triggered from this build file.
set(greader_plugin_bundled_libraries
  # Bundle the vendor DLL so the app can load it at runtime
  "${SDK_LIB_DIR}/GReader.dll"
  PARENT_SCOPE
)

# === Tests ===
# These unit tests can be run from a terminal after building the example, or
# from Visual Studio after opening the generated solution file.

# Only enable test builds when building the example (which sets this variable)
# so that plugin clients aren't building the tests.
if (${include_${PROJECT_NAME}_tests})
set(TEST_RUNNER "${PROJECT_NAME}_test")
enable_testing()

# Add the Google Test dependency.
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.11.0.zip
)
# Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Disable install commands for gtest so it doesn't end up in the bundle.
set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of googletest" FORCE)
FetchContent_MakeAvailable(googletest)

# The plugin's C API is not very useful for unit testing, so build the sources
# directly into the test binary rather than using the DLL.
add_executable(${TEST_RUNNER}
  test/greader_plugin_test.cpp
  ${PLUGIN_SOURCES}
)
apply_standard_settings(${TEST_RUNNER})
target_include_directories(${TEST_RUNNER} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${TEST_RUNNER} PRIVATE "${SDK_INCLUDE_DIR}")
target_link_libraries(${TEST_RUNNER} PRIVATE flutter_wrapper_plugin)
target_link_libraries(${TEST_RUNNER} PRIVATE gtest_main gmock)
# Link vendor import library so references like OpenRS232/Close resolve
target_link_libraries(${TEST_RUNNER} PRIVATE "${SDK_LIB_DIR}/GReader.lib")

 # MSVC: Ensure UTF-8 source and do not treat warnings as errors for tests.
 if (MSVC)
   target_compile_options(${TEST_RUNNER} PRIVATE
     "/utf-8"   # interpret sources as UTF-8
     "/WX-"     # do not treat warnings as errors in tests
  "/wd4819"  # suppress codepage warning from vendor headers
  "/wd4828"  # suppress invalid char warnings from non-UTF8 vendor headers
   )
 endif()
# flutter_wrapper_plugin has link dependencies on the Flutter DLL.
add_custom_command(TARGET ${TEST_RUNNER} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${FLUTTER_LIBRARY}" $<TARGET_FILE_DIR:${TEST_RUNNER}>
)

# Also copy the vendor runtime DLL for executing tests locally
if (EXISTS "${SDK_LIB_DIR}/GReader.dll")
  add_custom_command(TARGET ${TEST_RUNNER} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${SDK_LIB_DIR}/GReader.dll" $<TARGET_FILE_DIR:${TEST_RUNNER}>
  )
endif()

# Enable automatic test discovery.
include(GoogleTest)
gtest_discover_tests(${TEST_RUNNER})
endif()
