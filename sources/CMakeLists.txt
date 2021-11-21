
#
#  Base Library
#
file(GLOB_RECURSE
        SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/internal/*.cpp
        )
add_library(${CMAKE_PROJECT_NAME}_lib ${SRCS})
target_include_directories(${CMAKE_PROJECT_NAME}_lib PUBLIC ${${PRJPREFIX}_ROOT_DIR}/include)
# JSON
target_include_directories(${CMAKE_PROJECT_NAME}_lib PUBLIC ${${PRJPREFIX}_ROOT_DIR}/third_party/nlohmann/include)
#
#  Base Library
#

#
#  UI Library
#  ==========
#
#  QT Package
#
message(STATUS "${PRJPREFIX}_QT_VERSION ${${PRJPREFIX}_QT_VERSION}")
message(STATUS "${PRJPREFIX}_QT_DIR ${${PRJPREFIX}_QT_DIR}")

if (${${PRJPREFIX}_QT_VERSION} LESS 5)
    message(FATAL_ERROR "QT version 5 minimum is required version 6 is recommended")
endif ()
if (NOT EXISTS ${${PRJPREFIX}_QT_DIR})
    message(FATAL_ERROR "Please give an existing Folder To find QT${${PRJPREFIX}_QT_VERSION} by setting ${PRJPREFIX}_QT_DIR")
endif ()
set(CMAKE_PREFIX_PATH ${${PRJPREFIX}_QT_DIR})
set(${PRJPREFIX}_QT_COMPONENTS_LIST Core Gui Widgets)
find_package(Qt${${PRJPREFIX}_QT_VERSION} COMPONENTS Core Gui Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTORCC_OPTIONS "--compress;9")
#
#  QT Package
#
file(GLOB_RECURSE
        SRCS_UI
        ${CMAKE_CURRENT_SOURCE_DIR}/gui/*
        )
add_library(${CMAKE_PROJECT_NAME}_ui ${SRCS_UI})
target_link_libraries(${CMAKE_PROJECT_NAME}_ui
        Qt::Core
        Qt::Gui
        Qt::Widgets
        ${CMAKE_PROJECT_NAME}_lib
        )
target_include_directories(${CMAKE_PROJECT_NAME}_ui PUBLIC ${${PRJPREFIX}_ROOT_DIR}/include)
target_include_directories(${CMAKE_PROJECT_NAME}_ui PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}_ui_autogen/include)
#  ==========
#  UI Library
#

#
#  Main Executable
#
file(GLOB_RECURSE
        SRCS_RESOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/*.qrc
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/*.rc
        )
add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRCS_RESOURCE})
target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_PROJECT_NAME}_lib
        ${CMAKE_PROJECT_NAME}_ui
        )


# base includes
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${${PRJPREFIX}_ROOT_DIR}/include)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows.dll")
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
#
#  Main Executable
#
