function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list(SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif ()
        endif ()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach ()
endfunction()

# Make the build tree runnable as is.
#
# On Windows there is no rpath, so every DLL must sit next to the executable;
# TARGET_RUNTIME_DLLS walks the dependencies transitively. On Linux the build rpath
# already points at the packages, and the install step gathers the shared objects
# through GET_RUNTIME_DEPENDENCIES, so there is nothing to copy.
function(copy_shared_libraries iTarget)
    if (NOT WIN32)
        return()
    endif ()
    # TARGET_RUNTIME_DLLS only accepts what the DLLs can sit beside; a static library
    # has no runtime directory of its own.
    get_target_property(targetType ${iTarget} TYPE)
    if (NOT targetType MATCHES "^(EXECUTABLE|SHARED_LIBRARY|MODULE_LIBRARY)$")
        return()
    endif ()
    # `true` when the list is empty: since everything but the Vulkan loader is linked
    # statically, a target may legitimately have no DLL to copy, and
    # `copy_if_different` without a single source file is a usage error.
    add_custom_command(TARGET ${iTarget} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E
            "$<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:${iTarget}>>,copy_if_different,true>"
            "$<TARGET_RUNTIME_DLLS:${iTarget}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMAND_EXPAND_LISTS
            COMMENT "Copying runtime DLLs next to ${iTarget}"
    )
endfunction()
