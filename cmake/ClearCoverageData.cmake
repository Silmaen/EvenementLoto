# Remove the coverage data a previous run left in a build tree.
#
# A `.gcda` no longer matches a translation unit recompiled since it was written. The
# gcov runtime does not ignore it: it fails to merge it and takes the process down,
# so a whole test suite reports SEGFAULT for a reason that has nothing to do with the
# code under test. Clearing them costs one run of coverage history, which is what a
# rebuild costs anyway.
#
# Invoked with -P and -DDIR=<build directory>.
file(GLOB_RECURSE coverageData "${DIR}/*.gcda")
if (coverageData)
    file(REMOVE ${coverageData})
    list(LENGTH coverageData coverageDataCount)
    message(STATUS "Cleared ${coverageDataCount} stale coverage data file(s)")
endif ()
