
#
# ----==== Base Library ====----
#
add_subdirectory(core)
#
# ----====  UI Library ====----
#
#  ImGUI UI lib
#
add_subdirectory(resources)
add_subdirectory(gui)

#  ImGUI UI Library
#
#
# ----==== Main Executable ====----
#
file(GLOB_RECURSE
        SRCS_RESOURCE
        resources/*.qrc
        resources/*.rc
)
add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRCS_RESOURCE})
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
            -Wno-reserved-identifier
            -Wno-missing-prototypes
    )
endif ()
target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_PROJECT_NAME}_lib
        ${CMAKE_PROJECT_NAME}_ui
)
if (CMAKE_SYSTEM_NAME MATCHES "Windows" AND CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif ()
#  Main Executable
#
# install functions
install(TARGETS ${CMAKE_PROJECT_NAME}
        RUNTIME DESTINATION .
        LIBRARY DESTINATION .
)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    install(CODE [[
		file(GET_RUNTIME_DEPENDENCIES
			EXECUTABLES $<TARGET_FILE:EvenementLoto>
			RESOLVED_DEPENDENCIES_VAR _r_deps
			UNRESOLVED_DEPENDENCIES_VAR _u_deps
			DIRECTORIES ${CMAKE_BINARY_DIR}
			PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
			POST_EXCLUDE_REGEXES ".*[Ss]ystem32/.*\\.dll"
		)
		foreach(_file ${_r_deps})
			file(INSTALL
				DESTINATION "${CMAKE_INSTALL_PREFIX}"
				TYPE SHARED_LIBRARY
				FILES "${_file}"
			)
		endforeach()
	]])
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    install(CODE [[
		file(GET_RUNTIME_DEPENDENCIES
			EXECUTABLES $<TARGET_FILE:EvenementLoto>
			RESOLVED_DEPENDENCIES_VAR _r_deps
			UNRESOLVED_DEPENDENCIES_VAR _u_deps
			DIRECTORIES ${CMAKE_BINARY_DIR}
			PRE_EXCLUDE_REGEXES "^/lib" "^/usr/lib" "^/lib64" "^/usr/lib64"
			POST_EXCLUDE_REGEXES "^/lib" "^/usr/lib" "^/lib64" "^/usr/lib64"
		)
		foreach(_file ${_r_deps})
			file(INSTALL
				DESTINATION "${CMAKE_INSTALL_PREFIX}"
				TYPE SHARED_LIBRARY
				FOLLOW_SYMLINK_CHAIN
				FILES "${_file}"
			)
		endforeach()
	]])
endif ()

if (EXISTS "${CMAKE_SOURCE_DIR}/data")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/"
            DESTINATION data
            PATTERN ".git" EXCLUDE
    )
endif ()
# When Documetation will be ready, just include it into the build process
#install(DIRECTORY "${CMAKE_BINARY_DIR}/Documentation/html/"
#        DESTINATION doc
#        OPTIONAL
#)
