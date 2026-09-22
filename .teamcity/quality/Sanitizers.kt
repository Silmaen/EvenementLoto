import jetbrains.buildServer.configs.kotlin.*

val sanitizerAddress =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerAdress", "Sanitizer Address", "linux-sanitizer-address")

val sanitizerLeak =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerLeak", "Sanitizer Leak", "linux-sanitizer-leak")

val sanitizerThread =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerThread", "Sanitizer Thread", "linux-sanitizer-thread")

val sanitizerUndefinedBehavior = presetBuild("LotoBranch_Build_LinuxX64_SanitizerUndefinedBehavior",
                                             "Sanitizer Undefined Behavior", "linux-sanitizer-undefined-behavior")

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
