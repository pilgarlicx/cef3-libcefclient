# CEF3 library import helper

if(MSVC)
    # static runtime linking('/MT') required to link with CEF3
    include(${CMAKE_CURRENT_LIST_DIR}/../configure_msvc_runtime.cmake)
    set(MSVC_RUNTIME "static")
    configure_msvc_runtime()
endif()

if(NOT CEF3DIR AND NOT "$ENV{CEF3DIR}")
    get_filename_component(CEF3DIR ${CMAKE_CURRENT_LIST_DIR}/../../CEF3 ABSOLUTE)
    set(CEF3_INCLUDE_DIRS ${CEF3DIR}/include)
    if(WIN32)
        set(CEF3_LIBRARIES
            libcef.lib
            debug ${CEF3DIR}/lib/libcef_dll_wrapperd.lib
            optimized ${CEF3DIR}/lib/libcef_dll_wrapper.lib)
        set(CEF3_LIBRARY_DIRS ${CEF3DIR}/lib)
    endif()
else()
    find_path(CEF3_INCLUDE_DIR include/cef_app.h
        HINTS CEF3DIR ENV CEF3DIR)
    find_library(LIBCEF_LIBRARY libcef
        HINTS CEF3DIR ENV CEF3DIR)
    find_library(LIBCEF_DLL_WRAPPER_LIBRARY libcef_dll_wrapper
        HINTS CEF3DIR ENV CEF3DIR)
    if(CEF3_INCLUDE_DIR AND LIBCEF_LIBRARY AND LIBCEF_DLL_WRAPPER_LIBRARY)
        set(CEF3_INCLUDE_DIRS "${CEF3_INCLUDE_DIR}")
        set(CEF3_LIBRARIES "${LIBCEF_LIBRARY}" "${LIBCEF_DLL_WRAPPER_LIBRARY}")
    endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(CEF3 DEFAULT_MSG
    CEF3_INCLUDE_DIRS CEF3_LIBRARIES)
