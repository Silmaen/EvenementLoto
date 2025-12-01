
if (WIN32)
    list(PREPEND CMAKE_PREFIX_PATH ${${PROJECT_PREFIX}_QT_DIR})
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
endif ()

file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_custom_target(All_Tests)
# googletest
find_package(GTest REQUIRED)

foreach (child ${children})
    if (${child} MATCHES "test_helper")
        continue()
    endif ()
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
        file(GLOB_RECURSE
                SRCS
                ${CMAKE_CURRENT_SOURCE_DIR}/${child}/*.cpp
        )
        if ("${SRCS} " STREQUAL " ")
            message(STATUS "Test folder '${child}' is empty, skipping")
        else ()
            set(${child}_exe ${PROJECT_PREFIX_LOWER}_${child}_unit_test)
            add_executable(${${child}_exe} ${SRCS})
            set_target_properties(${${child}_exe} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
            set_target_properties(${${child}_exe} PROPERTIES FOLDER "UnitTests")
            target_include_directories(${${child}_exe} PUBLIC test_helper)
            target_link_libraries(${${child}_exe} GTest::gtest)
            target_link_libraries(${${child}_exe} ${CMAKE_PROJECT_NAME}_lib)
            if (${child} MATCHES "[g]ui")
                target_link_libraries(${${child}_exe} ${CMAKE_PROJECT_NAME}_ui)
                if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
                    target_compile_options(${${child}_exe} PRIVATE
                            -Wno-reserved-identifier
                            -Wno-missing-prototypes
                    )
                endif ()
            endif ()
            set(${PROJECT_PREFIX_LOWER}_${child}_test_command ${CMAKE_BINARY_DIR}/bin/${${child}_exe} --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${child}_UTest_Report.xml)
            add_test(${PROJECT_PREFIX_LOWER}_${child}_UTests ${${PROJECT_PREFIX_LOWER}_${child}_test_command})
            set_tests_properties(${PROJECT_PREFIX_LOWER}_${child}_UTests PROPERTIES TIMEOUT 3600)
            if (${PROJECT_PREFIX}_BUILD_SHARED AND WIN32)
                add_custom_command(TARGET ${${child}_exe} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GTest_DIR}/../../../bin/libgtest$<$<CONFIG:Debug>:d>.dll" "${CMAKE_BINARY_DIR}/bin"
                        COMMAND_EXPAND_LISTS
                )
            endif ()

            add_dependencies(All_Tests ${${child}_exe})
            add_custom_command(TARGET All_Tests POST_BUILD
                    COMMAND ${${child}_exe}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

        ENDIF ()
    ENDIF ()
ENDFOREACH ()

if (${PROJECT_PREFIX}_ENABLE_COVERAGE)

    set(COVERAGE_RESULT_DIR ${CMAKE_BINARY_DIR}/Coverage)
    file(MAKE_DIRECTORY ${COVERAGE_RESULT_DIR})
    if ("d${${PROJECT_PREFIX}_GCOV}" STREQUAL "d")
        if (${PROJECT_PREFIX}_COMPILER_GCC)
            set(COVERAGE_GCOV "gcov")
        elseif (${PROJECT_PREFIX}_COMPILER_CLANG)
            set(COVERAGE_GCOV "llvm gcov")
        endif ()
    else ()
        set(COVERAGE_GCOV "${${PROJECT_PREFIX}_GCOV}")
    endif ()
    set(CMD ${GCOVR} -r ${CMAKE_SOURCE_DIR} -o ${COVERAGE_RESULT_DIR}/index.html --gcov-executable ${COVERAGE_GCOV} .)

    add_custom_command(TARGET All_Tests POST_BUILD
            COMMAND echo "command: '${CMD}'")
    add_custom_command(TARGET All_Tests POST_BUILD
            COMMAND ${CMD}
            COMMAND_EXPAND_LISTS
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            USES_TERMINAL)
    add_custom_command(TARGET All_Tests POST_BUILD
            COMMAND echo "look at the coverage result: file:///${COVERAGE_RESULT_DIR}/index.html")
endif ()
