
add_custom_target(${CMAKE_PROJECT_NAME}_resource ALL
        COMMENT "Building Resource target..."
)

# Resources construction
add_custom_command(TARGET ${CMAKE_PROJECT_NAME}_resource POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/darkicons"
        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/darkicons"
        COMMENT "Copying Icon resources..."
        VERBATIM
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}_resource POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/documentation"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/document/Utilisation.md"
        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/documentation/Utilisation.md"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/document/images"
        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/documentation/images"
        COMMENT "Copying documentation resources..."
        VERBATIM
)

# The desktop entry and its icon: on Wayland a client cannot ask for a window icon, the
# compositor takes it from the `.desktop` entry it matches by application id. Shipped
# beside the program rather than installed system wide, the archive being portable.
if (${PROJECT_PREFIX}_PLATFORM_LINUX)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME}_resource POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/desktop"
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/EvenementLoto.desktop"
            "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/desktop/EvenementLoto.desktop"
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/mainIcon/mainIcon_256.png"
            "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/desktop/EvenementLoto.png"
            COMMENT "Copying desktop entry..."
            VERBATIM
    )
endif ()

install(DIRECTORY $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/
        DESTINATION resources
)
