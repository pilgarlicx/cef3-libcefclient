# @file configure_msvc_runtime.cmake
#
# @breif
#
macro(configure_msvc_runtime)
  if(MSVC)
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    # Set compiler options.
    set(msvc_flags
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${MSVC_RUNTIME} STREQUAL "static")
      message(STATUS
        "MSVC -> use statically-linked runtime."
      )
      foreach(flag ${msvc_flags})
        if(${flag} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> use dynamically-linked runtime."
      )
      foreach(flag ${msvc_flags})
        if(${flag} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${flag} "${${flag}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()
