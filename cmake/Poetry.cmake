option(${PROJECT_PREFIX}_USE_POETRY "Use Poetry for Python dependency management" OFF)

if (${PROJECT_PREFIX}_USE_POETRY)
    # Check if Poetry is installed
    execute_process(
            COMMAND poetry --version
            RESULT_VARIABLE POETRY_CHECK_RESULT
            OUTPUT_VARIABLE POETRY_VERSION_OUTPUT
            ERROR_VARIABLE POETRY_ERROR_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
    )

    if (POETRY_CHECK_RESULT EQUAL 0)
        message(STATUS "Poetry is installed: ${POETRY_VERSION_OUTPUT}")

        execute_process(
                COMMAND poetry sync --no-root
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE POETRY_INSTALL_RESULT
                OUTPUT_VARIABLE POETRY_INSTALL_OUTPUT
                ERROR_VARIABLE POETRY_INSTALL_ERROR
        )

        if (NOT POETRY_INSTALL_RESULT EQUAL 0)
            message(FATAL_ERROR "Poetry sync failed: ${POETRY_INSTALL_ERROR}")
        endif ()

    else ()
        message(WARNING "Poetry is not installed. Please install Poetry to manage Building Tools dependencies.
see https://python-poetry.org/docs/#installing-with-the-official-installer")
    endif ()
else ()
    message(STATUS "Poetry usage is disabled. Skipping Python dependency management.")
endif ()
