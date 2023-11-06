
find_program(EDEPMANAGER depmanager)
if (${EDEPMANAGER} STREQUAL EDEPMANAGER-NOTFOUND)
    message(FATAL_ERROR "Dependency manager not found.")
else ()
    execute_process(COMMAND ${EDEPMANAGER} info cmakedir
            OUTPUT_VARIABLE depmanager_path)
    string(STRIP ${depmanager_path} depmanager_path)
    string(REPLACE "\\" "/" depmanager_path ${depmanager_path})
    list(PREPEND CMAKE_MODULE_PATH ${depmanager_path})
    include(DepManager)
endif ()

dm_load_package(magic_enum)
dm_load_package(fmt KIND static)
dm_load_package(spdlog KIND static)
if (${${PRJPREFIX}_TESTING})
    dm_load_package(googletest KIND static)
endif ()
