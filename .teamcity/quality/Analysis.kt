import jetbrains.buildServer.configs.kotlin.*

// The id is the one the compile-time clang-tidy configuration already had: the check is
// the same, better run, and its build history is worth keeping.
val clangTidy = analysisBuild("LotoBranch_Build_LinuxX64_ClangTidy", "Clang-Tidy", "tidy",
                              onDiff = false)

val clangTidyDiff = analysisBuild("LotoBranch_Build_Quality_ClangTidyDiff", "Clang-Tidy (diff)",
                                  "tidy", onDiff = true)

val staticAnalyzer = analysisBuild("LotoBranch_Build_Quality_StaticAnalyzer", "Static Analyzer",
                                   "analyzer", onDiff = false)

val staticAnalyzerDiff = analysisBuild("LotoBranch_Build_Quality_StaticAnalyzerDiff",
                                       "Static Analyzer (diff)", "analyzer", onDiff = true)

/**
 * The two analyses, each in both scopes. Kept apart from the sanitizers: one reads the
 * code, the other runs it.
 */
val analysis = Project {
    id = RelativeId("LotoBranch_Analysis")
    name = "Analysis"

    buildType(clangTidyDiff)
    buildType(staticAnalyzerDiff)
    buildType(clangTidy)
    buildType(staticAnalyzer)

    buildTypesOrder = arrayListOf(
            clangTidyDiff,
            staticAnalyzerDiff,
            clangTidy,
            staticAnalyzer,
    )

    params {
        param("platform", "Linux")
    }
}
