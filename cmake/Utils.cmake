# cmake/Utils.cmake

function(check_required_targets)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs TARGETS)
    cmake_parse_arguments(PARSE_ARGV 0 _CHECK "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT _CHECK_TARGETS)
        message(FATAL_ERROR "check_required_targets() called without TARGETS argument")
    endif()

    foreach(_target_var IN LISTS _CHECK_TARGETS)
        if(NOT DEFINED ${_target_var})
            message(FATAL_ERROR "Variable '${_target_var}' is not defined.")
        elseif("${${_target_var}}" STREQUAL "")
            string(REGEX REPLACE "^_(.*)_target$" "\\1" _lib_name "${_target_var}")
            if(_lib_name STREQUAL "${_target_var}")
                # 正则未匹配，说明命名不符合约定，直接用变量名
                set(_lib_name "${_target_var}")
            endif()
            string(TOUPPER "${_lib_name}" _LIB_NAME)
            message(FATAL_ERROR
                "Required dependency '${_lib_name}' not found.\n"
                "Please ensure the following:\n"
                "  - The library is installed (e.g., via vcpkg)\n"
                "  - find_package(${_LIB_NAME} ...) succeeded\n"
                "  - A valid target was assigned to ${_target_var}"
            )
        endif()
    endforeach()
endfunction()