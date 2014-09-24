cmake_minimum_required(VERSION 2.6)
get_filename_component(DEPENDS_DIR depends ABSOLUTE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${DEPENDS_DIR}/cmake/Modules)
include(${DEPENDS_DIR}/cmake/import_custom_library.cmake)
include(${DEPENDS_DIR}/cmake/configure_msvc_runtime.cmake)

project(cef3-libcefclient)

# Configure MSVC runtime library ('/MD' or '/MT'), '/MT' required here
set(MSVC_RUNTIME "static")
configure_msvc_runtime()

#
include_directories(
    .
    ./include
)
add_definitions(
    -DUNICODE
    -D_UNICODE
    -DOS_WIN
    -DUSE_CEF_SHARED
)

# main target
set(target cefclient)
set(${target}_headers
    include/offscreen_render_handler.h
    include/client_app.h
    include/client_handler.h
    include/client_renderer.h
    include/client_switches.h
    include/client_resource.h
    include/string_util.h
    include/util.h
    include/v8_util.h
)
set(${target}_sources
    src/offscreen_render_handler.cpp
    src/client_app.cpp
    src/client_app_delegates.cpp
    src/client_handler.cpp
    src/client_handler_win.cpp
    src/client_renderer.cpp
    src/client_switches.cpp
    src/string_util.cpp
    src/v8_util.cpp
)
add_library(${target} ${${target}_headers} ${${target}_sources})

import_custom_library(${target} CEF3)
