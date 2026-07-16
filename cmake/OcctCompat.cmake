# OpenCASCADE Debian/Ubuntu link-path compatibility (unversioned .so names).

set(_openbimrl_lib_compat "${CMAKE_BINARY_DIR}/lib-compat")
set(_openbimrl_lib_compat_from "")
set(_openbimrl_lib_compat_to "")

macro(_openbimrl_add_lib_compat_symlink _link_name _real_path)
    if(EXISTS "${_real_path}" AND NOT EXISTS "/usr/lib/x86_64-linux-gnu/${_link_name}")
        file(MAKE_DIRECTORY "${_openbimrl_lib_compat}")
        set(_compat_path "${_openbimrl_lib_compat}/${_link_name}")
        if(NOT EXISTS "${_compat_path}")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E create_symlink "${_real_path}" "${_compat_path}")
        endif()
        list(APPEND _openbimrl_lib_compat_from
             "/usr/lib/x86_64-linux-gnu/${_link_name}")
        list(APPEND _openbimrl_lib_compat_to "${_compat_path}")
    endif()
endmacro()

_openbimrl_add_lib_compat_symlink("libtbb.so" "/usr/lib/x86_64-linux-gnu/libtbb.so.12")
_openbimrl_add_lib_compat_symlink("libtbbmalloc.so"
                                  "/usr/lib/x86_64-linux-gnu/libtbbmalloc.so.2")
_openbimrl_add_lib_compat_symlink("libtcl.so" "/usr/lib/x86_64-linux-gnu/libtcl8.6.so")
_openbimrl_add_lib_compat_symlink("libtk.so" "/usr/lib/x86_64-linux-gnu/libtk8.6.so")

if(_openbimrl_lib_compat_from)
    link_directories("${_openbimrl_lib_compat}")

    macro(_openbimrl_apply_lib_compat_replacements _links_str _new_links_var)
        set(_result "${_links_str}")
        list(LENGTH _openbimrl_lib_compat_from _compat_count)
        math(EXPR _last_compat_index "${_compat_count} - 1")
        foreach(_idx RANGE ${_last_compat_index})
            list(GET _openbimrl_lib_compat_from ${_idx} _from)
            list(GET _openbimrl_lib_compat_to ${_idx} _to)
            string(REPLACE "${_from}" "${_to}" _result "${_result}")
        endforeach()
        set(${_new_links_var} "${_result}")
    endmacro()

    macro(_openbimrl_patch_target_link_paths _target)
        if(NOT TARGET "${_target}")
            return()
        endif()
        foreach(_prop INTERFACE_LINK_LIBRARIES LINK_LIBRARIES)
            get_target_property(_links "${_target}" ${_prop})
            if(_links AND NOT _links STREQUAL "${_prop}-NOTFOUND")
                list(JOIN _links ";" _links_str)
                _openbimrl_apply_lib_compat_replacements("${_links_str}" _new_links)
                if(NOT _new_links STREQUAL _links_str)
                    set_target_properties("${_target}" PROPERTIES ${_prop}
                                          "${_new_links}")
                endif()
            endif()
        endforeach()
    endmacro()

    macro(_openbimrl_patch_occt_link_paths)
        file(GLOB _occt_cmake_files
             "/usr/lib/x86_64-linux-gnu/cmake/opencascade/OpenCASCADE*Targets*.cmake"
             "/usr/lib/cmake/opencascade/OpenCASCADE*Targets*.cmake")
        foreach(_occt_file IN LISTS _occt_cmake_files)
            file(READ "${_occt_file}" _occt_contents)
            string(REGEX MATCHALL "add_library\\([^ )]+" _occt_targets "${_occt_contents}")
            foreach(_occt_entry IN LISTS _occt_targets)
                string(REGEX REPLACE "add_library\\(" "" _occt_target "${_occt_entry}")
                _openbimrl_patch_target_link_paths("${_occt_target}")
            endforeach()
        endforeach()
    endmacro()

    _openbimrl_patch_occt_link_paths()
endif()
