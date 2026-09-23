
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
        if (NOT SRCS)
            message(STATUS "Test folder '${child}' is empty, skipping")
        else ()

            set(${child}_exe ${PROJECT_PREFIX_LOWER}_${child}_unit_test)
            add_executable(${${child}_exe} ${SRCS})
            set_target_properties(${${child}_exe} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
            set_target_properties(${${child}_exe} PROPERTIES FOLDER "UnitTests")
            if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_helper")
                target_include_directories(${${child}_exe} PUBLIC test_helper)
            endif ()
            # The shipped .lev files are read by the serialization tests: they are the
            # only real files in versions 3, 4 and 6 we have, and the backward
            # compatibility of the reader is exactly what they check.
            target_compile_definitions(${${child}_exe} PRIVATE
                    ${PROJECT_PREFIX}_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data"
                    ${PROJECT_PREFIX}_TEST_REFERENCE_FILE="${CMAKE_SOURCE_DIR}/test/lib_test/reference-v7.lev")
            target_link_libraries(${${child}_exe} GTest::gtest)
            target_link_libraries(${${child}_exe} ${CMAKE_PROJECT_NAME}_lib)
            if (${child} MATCHES "gui")
                target_link_libraries(${${child}_exe} ${CMAKE_PROJECT_NAME}_ui)
            endif ()
            copy_shared_libraries(${${child}_exe})
            set(${PROJECT_PREFIX_LOWER}_${child}_test_command ${CMAKE_BINARY_DIR}/bin/${${child}_exe} --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${child}_UTest_Report.xml)
            add_test(${PROJECT_PREFIX_LOWER}_${child}_UTests ${${PROJECT_PREFIX_LOWER}_${child}_test_command})
            set_tests_properties(${PROJECT_PREFIX_LOWER}_${child}_UTests PROPERTIES TIMEOUT 3600)
            if (${PROJECT_PREFIX}_ENABLE_ADDRESS_SANITIZER OR ${PROJECT_PREFIX}_ENABLE_LEAK_SANITIZER)
                set_tests_properties(${PROJECT_PREFIX_LOWER}_${child}_UTests PROPERTIES
                        ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/lsan_suppressions.txt")
            endif ()
            add_dependencies(All_Tests ${${child}_exe})
            if (${PROJECT_PREFIX}_ENABLE_ADDRESS_SANITIZER OR ${PROJECT_PREFIX}_ENABLE_LEAK_SANITIZER)
                add_custom_command(TARGET All_Tests POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E env "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/lsan_suppressions.txt" ${${child}_exe}
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
            else ()
                add_custom_command(TARGET All_Tests POST_BUILD
                        COMMAND ${${child}_exe}
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
            endif ()
        endif ()
    endif ()
endforeach ()

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
