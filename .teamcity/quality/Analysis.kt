import jetbrains.buildServer.configs.kotlin.*

/// What must be green before an analysis runs: the whole chain below it.
private val analysisGates = listOf(sanitizerAddress, sanitizerLeak, sanitizerThread, sanitizerUndefinedBehavior)

// The id is the one the compile-time clang-tidy configuration already had: the check is
// the same, better run, and its build history is worth keeping.
val clangTidy = analysisBuild("LotoBranch_Build_LinuxX64_ClangTidy", "Clang-Tidy", "tidy", analysisGates)

val staticAnalyzer =
        analysisBuild("LotoBranch_Build_Quality_StaticAnalyzer", "Static Analyzer", "analyzer", analysisGates)

/**
 * The two analyses. Kept apart from the sanitizers: one reads the code, the other runs
 * it — and these two are the last links of the chain, so they are the ones a merge waits
 * for.
 */
val analysis = Project {
    id = RelativeId("LotoBranch_Analysis")
    name = "Analysis"

    buildType(clangTidy)
    buildType(staticAnalyzer)

    buildTypesOrder = arrayListOf(
            clangTidy,
            staticAnalyzer,
    )

    params {
        param("platform", "Linux")
    }
}
