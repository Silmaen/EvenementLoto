import jetbrains.buildServer.configs.kotlin.*

/// A sanitizer runs the program, so there is no point in one before the program builds.
/// The **Clang** builds only, on both platforms: they are the ones these are built from,
/// and the sanitizers are Clang-only anyway. The GCC builds stay off the chain — they
/// still run on every pull request and report their own result, but making four waves
/// wait on a second compiler would lengthen every pull request for nothing.
private val buildGates = listOf(linuxClang, windowsClang)

val sanitizerAddress = presetBuild("LotoBranch_Build_LinuxX64_SanitizerAdress", "Sanitizer Address",
                                   "linux-sanitizer-address", gates = buildGates)

val sanitizerLeak = presetBuild("LotoBranch_Build_LinuxX64_SanitizerLeak", "Sanitizer Leak",
                                "linux-sanitizer-leak", gates = buildGates)

val sanitizerThread = presetBuild("LotoBranch_Build_LinuxX64_SanitizerThread", "Sanitizer Thread",
                                  "linux-sanitizer-thread", gates = buildGates)

val sanitizerUndefinedBehavior = presetBuild("LotoBranch_Build_LinuxX64_SanitizerUndefinedBehavior",
                                             "Sanitizer Undefined Behavior",
                                             "linux-sanitizer-undefined-behavior", gates = buildGates)

val sanitizers = Project {
    id = RelativeId("LotoBranch_Sanitizers")
    name = "Sanitizers"

    buildType(sanitizerAddress)
    buildType(sanitizerLeak)
    buildType(sanitizerThread)
    buildType(sanitizerUndefinedBehavior)

    buildTypesOrder = arrayListOf(
            sanitizerAddress,
            sanitizerLeak,
            sanitizerThread,
            sanitizerUndefinedBehavior,
    )

    params {
        param("platform", "Linux")
    }
}
