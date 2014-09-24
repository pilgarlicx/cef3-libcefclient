# CEF3 library import helper
function(FindCEF3 package)
    set(${package}_FOUND ON PARENT_SCOPE)
    get_filename_component(${package}_HOME
        ${CMAKE_CURRENT_LIST_DIR}/../../CEF3 ABSOLUTE)
    set(${package}_INCLUDE_DIRS ${${package}_HOME}/include PARENT_SCOPE)
    set(${package}_LIBRARIES
        libcef.lib
        debug ${${package}_HOME}/lib/libcef_dll_wrapperd.lib
        optimized ${${package}_HOME}/lib/libcef_dll_wrapper.lib
        PARENT_SCOPE)
    set(${package}_LIBRARY_DIRS ${${package}_HOME}/lib PARENT_SCOPE)
endfunction(FindCEF3)

FindCEF3(CEF3)
