
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_custom_target(All_Tests)
# googletest
find_package(GTest REQUIRED)

# The GUI tests instantiate the application, so they need a display. `xvfb-run` gives
# them a virtual one and Vulkan falls back on lavapipe, which is all they need — they
# check the loop, not the pixels. Without it the whole GUI suite could only ever be run
# by hand, which is why `test_Application.cpp` sat commented out for a year.
if ((${PROJECT_PREFIX}_ENABLE_LEAK_SANITIZER OR ${PROJECT_PREFIX}_ENABLE_ADDRESS_SANITIZER) AND NOT WIN32)
    find_library(SANITIZER_KEEPS_DRIVER_LOADED NAMES vulkan_lvp)
    if (SANITIZER_KEEPS_DRIVER_LOADED)
        message(STATUS "GUI tests will keep ${SANITIZER_KEEPS_DRIVER_LOADED} loaded for the leak check")
    endif ()
endif ()

if (NOT WIN32)
    find_program(XVFB_RUN xvfb-run)
    if (XVFB_RUN)
        message(STATUS "GUI tests will run under ${XVFB_RUN}")
    else ()
        message(WARNING "xvfb-run not found: the GUI tests need a display to pass")
    endif ()
endif ()

# Each sanitizer that has known findings outside our code gets its suppression file
# here, once, rather than at every place a test is run.
set(sanitizerEnv "")
if (${PROJECT_PREFIX}_ENABLE_ADDRESS_SANITIZER OR ${PROJECT_PREFIX}_ENABLE_LEAK_SANITIZER)
    list(APPEND sanitizerEnv "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/lsan_suppressions.txt")
endif ()
if (${PROJECT_PREFIX}_ENABLE_THREAD_SANITIZER)
    list(APPEND sanitizerEnv "TSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/tsan_suppressions.txt")
endif ()

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
            set(testLauncher "")
            if (${child} MATCHES "gui" AND SANITIZER_KEEPS_DRIVER_LOADED)
                # The Vulkan ICD is dlopen'd by the loader and unloaded at
                # vkDestroyInstance, taking its allocations out of reach: LeakSanitizer
                # then reports them from an <unknown module>, which no suppression can
                # name. Keeping the software driver loaded makes them reachable again —
                # and proves they were never leaks. The signal is kept for our own code:
                # this is exactly how the surface no longer freed since ImGui 1.92.6 was
                # caught.
                set(testLauncher ${CMAKE_COMMAND} -E env LD_PRELOAD=${SANITIZER_KEEPS_DRIVER_LOADED})
            endif ()
            if (${child} MATCHES "gui" AND XVFB_RUN)
                # -a picks a free display number, so parallel builds on one agent do not
                # fight over :99.
                set(testLauncher ${XVFB_RUN} -a ${testLauncher})
            endif ()
            set(${PROJECT_PREFIX_LOWER}_${child}_test_command ${testLauncher} ${CMAKE_BINARY_DIR}/bin/${${child}_exe} --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${child}_UTest_Report.xml)
            add_test(${PROJECT_PREFIX_LOWER}_${child}_UTests ${${PROJECT_PREFIX_LOWER}_${child}_test_command})
            set_tests_properties(${PROJECT_PREFIX_LOWER}_${child}_UTests PROPERTIES TIMEOUT 3600)
            if (sanitizerEnv)
                set_tests_properties(${PROJECT_PREFIX_LOWER}_${child}_UTests PROPERTIES
                        ENVIRONMENT "${sanitizerEnv}")
            endif ()
            add_dependencies(All_Tests ${${child}_exe})
            if (sanitizerEnv)
                add_custom_command(TARGET All_Tests POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E env ${sanitizerEnv} ${testLauncher} ${${child}_exe}
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
            else ()
                add_custom_command(TARGET All_Tests POST_BUILD
                        COMMAND ${testLauncher} ${${child}_exe}
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
