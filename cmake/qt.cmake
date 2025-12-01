#
#  QT Package
#
set(${PROJECT_PREFIX}_QT_COMPONENTS_LIST Core Gui Widgets OpenGLWidgets Svg)
set(${PROJECT_PREFIX}_QT_COMPONENTS_LIST_LIB Qt::Core Qt::Gui Qt::Widgets Qt::Svg)
#
# Base initialization
#
if ("${${PROJECT_PREFIX}_QT_VERSION}" MATCHES "" OR "${${PROJECT_PREFIX}_QT_DIR}" MATCHES "")
    if ("${${PROJECT_PREFIX}_QT_VERSION}" MATCHES "")
        set(${PROJECT_PREFIX}_QT_VERSION 6)
    endif ()
else ()
    if (${${PROJECT_PREFIX}_QT_VERSION} LESS 5)
        message(FATAL_ERROR "QT version 5 minimum is required version 6 is recommended")
    endif ()
    if (NOT EXISTS ${${PROJECT_PREFIX}_QT_DIR})
        message(FATAL_ERROR "Please give an existing Folder To find QT${${PROJECT_PREFIX}_QT_VERSION} by setting ${PROJECT_PREFIX}_QT_DIR")
    endif ()
    set(CMAKE_PREFIX_PATH ${${PROJECT_PREFIX}_QT_DIR})

endif ()

find_package(Qt${${PROJECT_PREFIX}_QT_VERSION} COMPONENTS ${${PROJECT_PREFIX}_QT_COMPONENTS_LIST} REQUIRED)

message(STATUS "Found QT version ${${PROJECT_PREFIX}_QT_VERSION} in ${Qt${${PROJECT_PREFIX}_QT_VERSION}_DIR}")
if ("${${PROJECT_PREFIX}_QT_DIR}" MATCHES "")
    set(${PROJECT_PREFIX}_QT_DIR ${Qt${${PROJECT_PREFIX}_QT_VERSION}_DIR})
endif ()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTORCC_OPTIONS "--compress;9")

function(qt_attach_to_target TARGET_ATTACH)
    target_link_libraries(${TARGET_ATTACH}
            ${${PROJECT_PREFIX}_QT_COMPONENTS_LIST_LIB})
    if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        target_compile_options(${TARGET_ATTACH} PRIVATE
                -Wno-reserved-identifier
                -Wno-missing-prototypes
        )
    endif ()
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(QT_INSTALL_PATH "${${PROJECT_PREFIX}_QT_DIR}")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
            if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
                set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
                if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
                    set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
                endif ()
            endif ()
        endif ()
        set(QT_PLUG_PATH ${QT_INSTALL_PATH})
        if (NOT EXISTS "${QT_PLUG_PATH}/plugins/platforms")
            set(QT_PLUG_PATH "${QT_PLUG_PATH}/share/qt${${PROJECT_PREFIX}_QT_VERSION}")
            if (NOT EXISTS "${QT_PLUG_PATH}/plugins/platforms")
                message(FATAL_ERROR "Cannot find qt plugins at ${QT_PLUG_PATH}/plugins/platforms")
            endif ()
        endif ()
        if (EXISTS "${QT_PLUG_PATH}/plugins/platforms/qwindows.dll")
            add_custom_command(TARGET ${TARGET_ATTACH} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/")
            add_custom_command(TARGET ${TARGET_ATTACH} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT_PLUG_PATH}/plugins/platforms/qwindows.dll"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/")
        else ()
            message(WARNING "Cannot find the platform dll at ${QT_PLUG_PATH}/plugins/platforms/qwindows.dll")
        endif ()
        foreach (QT_LIB ${${PROJECT_PREFIX}_QT_COMPONENTS_LIST})
            add_custom_command(TARGET ${TARGET_ATTACH} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT_INSTALL_PATH}/bin/Qt${${PROJECT_PREFIX}_QT_VERSION}${QT_LIB}.dll"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
        endforeach (QT_LIB)
    endif ()
endfunction()
