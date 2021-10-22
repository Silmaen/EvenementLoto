
FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

if (${PRJPREFIX}_COVERAGE)
    add_custom_target(Generate_test_coverage_report)
endif ()

FOREACH(child ${children})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
        file(GLOB
             SRCS
             ${CMAKE_CURRENT_SOURCE_DIR}/${child}/*.cpp
             )
        IF("${SRCS} " STREQUAL " ")
            message(STATUS "Test folder '${child}' is empty, skipping")
        ELSE()
            add_executable(${PRJPREFIX_LOWER}_${child}_unit_test ${SRCS})
            target_include_directories(${PRJPREFIX_LOWER}_${child}_unit_test PUBLIC ${${PRJPREFIX}_ROOT_DIR}/include)
            target_link_libraries(${PRJPREFIX_LOWER}_${child}_unit_test gtest gtest_main)
            target_link_libraries(${PRJPREFIX_LOWER}_${child}_unit_test ${CMAKE_PROJECT_NAME}_lib)
            add_test(${PRJPREFIX_LOWER}_${child}_UTests ./${PRJPREFIX_LOWER}_${child}_unit_test --gtest_output=xml:test/${child}_UTest_Report.xml)
            set_tests_properties(${PRJPREFIX_LOWER}_${child}_UTests PROPERTIES TIMEOUT 3600)
            if (${PRJPREFIX}_COVERAGE)
                add_dependencies(Generate_test_coverage_report ${PRJPREFIX_LOWER}_${child}_unit_test)
                add_custom_command(TARGET Generate_test_coverage_report POST_BUILD
                        COMMAND ${PRJPREFIX_LOWER}_${child}_unit_test
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
            endif ()
        ENDIF ()
    ENDIF ()
ENDFOREACH ()

if (${PRJPREFIX}_COVERAGE)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Coverage)
    set(COVERAGE_TITLE "Evenement Loto: couverture de tests")
    set(COVERAGE_HTML --html --html-details --html-details-syntax-highlighting --html-title ${COVERAGE_TITLE} --source-encoding utf-8)
    set(COVERAGE_FILTER -e ".*third_party/.*" -e ".*test/.*")

    if ("d${${PRJPREFIX}_GCOV}" STREQUAL "d")
        set(COVERAGE_GCOV)
    else ()
        set(COVERAGE_GCOV "--gcov-executable \"llvm-cov gcov\"" --gcov-ignore-parse-errors)
    endif ()
    add_custom_command(TARGET Generate_test_coverage_report POST_BUILD
            COMMAND ${GCOVR} -kbp -r ${CMAKE_SOURCE_DIR} -o ${CMAKE_BINARY_DIR}/Coverage/index.html ${COVERAGE_GCOV} ${COVERAGE_HTML} ${COVERAGE_FILTER} .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            USES_TERMINAL)
endif ()

