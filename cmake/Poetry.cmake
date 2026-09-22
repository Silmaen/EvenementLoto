#
# Poetry owns the Python virtual environment and every build tool it contains
# (conan, gcovr). Its bin directory is prepended to PATH so that the later
# find_program() calls resolve to the pinned versions.
#
option(${PROJECT_PREFIX}_USE_POETRY "Let Poetry provide the build tooling" ON)

if (NOT ${PROJECT_PREFIX}_USE_POETRY)
    message(STATUS "Poetry disabled: build tools are expected to be in PATH.")
    return()
endif ()

# Keep the virtual environment out of the source tree: an in-project .venv is not
# portable between the host and the build containers, and CI wipes untracked files.
set(ENV{POETRY_VIRTUALENVS_IN_PROJECT} false)

find_program(POETRY_EXECUTABLE poetry REQUIRED)

execute_process(COMMAND ${POETRY_EXECUTABLE} --version
        OUTPUT_VARIABLE poetryVersion
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE poetryResult)
if (NOT poetryResult EQUAL 0)
    message(FATAL_ERROR "Poetry is not usable: ${POETRY_EXECUTABLE}")
endif ()
message(STATUS "Using ${poetryVersion} @ ${POETRY_EXECUTABLE}")

execute_process(COMMAND ${POETRY_EXECUTABLE} sync --no-root
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE poetryResult
        OUTPUT_VARIABLE poetryOutput
        ERROR_VARIABLE poetryError)
if (NOT poetryResult EQUAL 0)
    message(FATAL_ERROR "Poetry sync failed:\n${poetryOutput}${poetryError}")
endif ()

execute_process(COMMAND ${POETRY_EXECUTABLE} env info --path
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE ${PROJECT_PREFIX}_VENV_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE poetryResult)
if (NOT poetryResult EQUAL 0 OR NOT EXISTS "${${PROJECT_PREFIX}_VENV_DIR}")
    message(FATAL_ERROR "Unable to locate the Poetry virtual environment.")
endif ()

if (WIN32)
    set(venvBinDir "${${PROJECT_PREFIX}_VENV_DIR}/Scripts")
    set(pathSeparator ";")
else ()
    set(venvBinDir "${${PROJECT_PREFIX}_VENV_DIR}/bin")
    set(pathSeparator ":")
endif ()

set(ENV{VIRTUAL_ENV} "${${PROJECT_PREFIX}_VENV_DIR}")
set(ENV{PATH} "${venvBinDir}${pathSeparator}$ENV{PATH}")
message(STATUS "Python tooling from: ${venvBinDir}")
