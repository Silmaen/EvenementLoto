import jetbrains.buildServer.configs.kotlin.*

val linuxGcc = presetBuild("LotoBranch_Build_LinuxX64_Gcc", "GCC", "linux-gcc-debug")

// The only Linux configuration a draft pull request triggers, with its Windows
// counterpart: enough to know the change compiles and passes its tests on both
// platforms, without spending nine agents on work its author calls unfinished.
val linuxClang = presetBuild("LotoBranch_Build_LinuxX64_Clang", "Clang", "linux-clang-debug", onDraft = true)

val linuxX64 = Project {
    id = RelativeId("LotoBranch_Build_LinuxX64")
    name = "Build Linux x64"

    buildType(linuxGcc)
    buildType(linuxClang)

    buildTypesOrder = arrayListOf(
            linuxGcc,
            linuxClang,
    )

    params {
        param("platform", "Linux")
    }
}
