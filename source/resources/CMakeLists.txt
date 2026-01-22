
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

install(DIRECTORY $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/
        DESTINATION resources
)
