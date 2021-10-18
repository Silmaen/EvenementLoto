
FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

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
            add_test(${PRJPREFIX_LOWER}_${child}_UTests ./${PRJPREFIX_LOWER}_${child}_unit_test  --gtest_output=xml:test/${child}_UTest_Report.xml)
            set_tests_properties(${PRJPREFIX_LOWER}_${child}_UTests PROPERTIES TIMEOUT 3600)
        ENDIF()
    ENDIF()
ENDFOREACH()
