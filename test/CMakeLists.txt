
if (WIN32)
    list(PREPEND CMAKE_PREFIX_PATH ${${PRJPREFIX}_QT_DIR})
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
endif ()

FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_custom_target(All_Tests)
# googletest
find_package(GTest 1.13 REQUIRED)

FOREACH (child ${children})
    if (${child} MATCHES "test_helper")
        continue()
    endif ()
    IF (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
        file(GLOB_RECURSE
                SRCS
                ${CMAKE_CURRENT_SOURCE_DIR}/${child}/*.cpp
        )
        IF ("${SRCS} " STREQUAL " ")
            message(STATUS "Test folder '${child}' is empty, skipping")
        ELSE ()
            set(${child}_exe ${PRJPREFIX_LOWER}_${child}_unit_test)
            add_executable(${${child}_exe} ${SRCS})
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
            if (CMAKE_SYSTEM_NAME MATCHES "Linux")
                set(${PRJPREFIX_LOWER}_${child}_test_command export DISPLAY=':0' && ./${${child}_exe} --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${child}_UTest_Report.xml)
            else ()
                set(${PRJPREFIX_LOWER}_${child}_test_command ./${${child}_exe} --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${child}_UTest_Report.xml)
            endif ()
            add_test(${PRJPREFIX_LOWER}_${child}_UTests ${${PRJPREFIX_LOWER}_${child}_test_command})
            set_tests_properties(${PRJPREFIX_LOWER}_${child}_UTests PROPERTIES TIMEOUT 3600)
            add_dependencies(All_Tests ${${child}_exe})
            add_custom_command(TARGET All_Tests POST_BUILD
                    COMMAND ${${PRJPREFIX_LOWER}_${child}_test_command}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

        ENDIF ()
    ENDIF ()
ENDFOREACH ()

if (${PRJPREFIX}_COVERAGE)
    set(COVERAGE_RESULT_DIR ${CMAKE_BINARY_DIR}/Coverage)
    file(MAKE_DIRECTORY ${COVERAGE_RESULT_DIR})
    set(COVERAGE_TITLE \"Evenement Loto: couverture de tests\")
    set(COVERAGE_HTML --html --html-details --html-details-syntax-highlighting --html-title ${COVERAGE_TITLE} --source-encoding utf-8)
    set(COVERAGE_FILTER -e \".*third_party/.*\" -e \".*test/.*\" -e \".*autogen.*\" -e \".*main\.cpp\")
    set(COVERAGE_OPTION -buds -j 4 --exclude-throw-branches --exclude-unreachable-branches --exclude-lines-by-pattern \".*---UNCOVER---.*\")
    if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        #        set(COVERAGE_GCOV --gcov-executable \"llvm-cov gcov\" --gcov-ignore-parse-errors)
    else ()
        set(COVERAGE_GCOV)
    endif ()
    add_custom_command(TARGET All_Tests POST_BUILD
            COMMAND ${GCOVR} ${COVERAGE_OPTION} -r ${CMAKE_SOURCE_DIR} -o ${COVERAGE_RESULT_DIR}/index.html ${COVERAGE_GCOV} ${COVERAGE_HTML} ${COVERAGE_FILTER} .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            USES_TERMINAL)
    add_custom_command(TARGET All_Tests POST_BUILD
            COMMAND echo \"look at the coverage result: file:///${COVERAGE_RESULT_DIR}/index.html\")
endif ()
