#
# Conan provides the third parties, but CMake drives it: the profile, the compiler and
# its version are pushed to Conan, never detected behind our back. The actual
# `conan install` is run by cmake/conan_provider.cmake on the first find_package().
#
find_program(CONAN_COMMAND conan REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(conanCompiler "gcc")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(conanCompiler "clang")
else ()
    message(FATAL_ERROR "No Conan profile for compiler '${CMAKE_CXX_COMPILER_ID}'")
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CONAN_HOST_PROFILE "windows-mingw-${conanCompiler}")
    set(CONAN_BUILD_PROFILE "${CONAN_HOST_PROFILE}")
else ()
    set(CONAN_HOST_PROFILE "linux-${conanCompiler}")
    # The build tools Conan has to compile (flex, m4, libiconv...) are autotools based
    # and expect a native toolchain; building them with gcc also matches the prebuilt
    # tool packages, so they are shared between the gcc and clang host profiles.
    set(CONAN_BUILD_PROFILE "linux-build")
    find_program(CONAN_BUILD_C_COMPILER gcc REQUIRED)
    find_program(CONAN_BUILD_CXX_COMPILER g++ REQUIRED)
    execute_process(COMMAND ${CONAN_BUILD_C_COMPILER} -dumpversion
            OUTPUT_VARIABLE buildCompilerVersion
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE conanResult)
    if (NOT conanResult EQUAL 0)
        message(FATAL_ERROR "Unable to read the version of ${CONAN_BUILD_C_COMPILER}")
    endif ()
    string(REGEX MATCH "^[0-9]+" buildCompilerVersion "${buildCompilerVersion}")
    set(ENV{EVL_BUILD_COMPILER_VERSION} "${buildCompilerVersion}")
    set(ENV{EVL_BUILD_C_COMPILER} "${CONAN_BUILD_C_COMPILER}")
    set(ENV{EVL_BUILD_CXX_COMPILER} "${CONAN_BUILD_CXX_COMPILER}")
endif ()

# Read back by the profiles: they hold the policy, CMake holds the toolchain.
string(REGEX MATCH "^[0-9]+" conanCompilerVersion "${CMAKE_CXX_COMPILER_VERSION}")
set(ENV{EVL_COMPILER_VERSION} "${conanCompilerVersion}")
set(ENV{EVL_CXX_COMPILER} "${CMAKE_CXX_COMPILER}")
set(ENV{EVL_C_COMPILER} "${CMAKE_C_COMPILER}")

# X11 is declared as provided by the platform (see the Linux profiles): Conan runs its
# own isolated pkgconf, so it must be told where the system .pc files live.
if (NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_program(PKG_CONFIG_EXECUTABLE pkg-config REQUIRED)
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable pc_path pkg-config
            OUTPUT_VARIABLE systemPkgConfigPath
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(ENV{EVL_SYSTEM_PKG_CONFIG_PATH} "${systemPkgConfigPath}")
endif ()

execute_process(COMMAND ${CONAN_COMMAND} config install "${PROJECT_SOURCE_DIR}/conan/config"
        RESULT_VARIABLE conanResult
        OUTPUT_VARIABLE conanOutput
        ERROR_VARIABLE conanOutput)
if (NOT conanResult EQUAL 0)
    message(FATAL_ERROR "Conan config install failed:\n${conanOutput}")
endif ()

# nativefiledialog-extended is not on ConanCenter, it comes from an in-tree recipe.
execute_process(COMMAND ${CONAN_COMMAND} remote add --force --type=local-recipes-index
        evl-local "${PROJECT_SOURCE_DIR}/conan/local-recipes"
        RESULT_VARIABLE conanResult
        OUTPUT_VARIABLE conanOutput
        ERROR_VARIABLE conanOutput)
if (NOT conanResult EQUAL 0)
    message(FATAL_ERROR "Unable to register the local recipe index:\n${conanOutput}")
endif ()

# A local-recipes-index revision does not follow the recipe content, so an edited
# recipe would be served stale from the cache. Treat it as the source file it is.
file(GLOB_RECURSE localRecipeFiles "${PROJECT_SOURCE_DIR}/conan/local-recipes/*")
set(localRecipesHash "")
foreach (recipeFile IN LISTS localRecipeFiles)
    file(SHA256 "${recipeFile}" recipeFileHash)
    string(APPEND localRecipesHash "${recipeFileHash}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${recipeFile}")
endforeach ()
string(SHA256 localRecipesHash "${localRecipesHash}")

# The stamp belongs to the Conan cache, not to a build directory: a fresh build
# directory must not purge a recipe that the other ones still point at.
execute_process(COMMAND ${CONAN_COMMAND} config home
        OUTPUT_VARIABLE conanHome
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE conanResult)
if (NOT conanResult EQUAL 0)
    message(FATAL_ERROR "Unable to locate the Conan home.")
endif ()
set(localRecipesStamp "${conanHome}/evl-local-recipes.sha256")
set(previousRecipesHash "")
if (EXISTS "${localRecipesStamp}")
    file(READ "${localRecipesStamp}" previousRecipesHash)
endif ()
if (NOT previousRecipesHash STREQUAL localRecipesHash)
    message(STATUS "Local Conan recipes changed: dropping them from the cache")
    execute_process(COMMAND ${CONAN_COMMAND} remove "nfd/*" --confirm
            RESULT_VARIABLE conanResult
            OUTPUT_VARIABLE conanOutput
            ERROR_VARIABLE conanOutput)
    if (NOT conanResult EQUAL 0)
        message(FATAL_ERROR "Unable to drop the local recipes from the Conan cache:\n${conanOutput}")
    endif ()
    file(WRITE "${localRecipesStamp}" "${localRecipesHash}")
endif ()

message(STATUS "Conan profiles: host=${CONAN_HOST_PROFILE} (${conanCompiler} ${conanCompilerVersion}) build=${CONAN_BUILD_PROFILE}")
