import jetbrains.buildServer.configs.kotlin.*

val linuxGcc = presetBuild("LotoBranch_Build_LinuxX64_Gcc", "GCC", "linux-gcc-debug")

val linuxClang = presetBuild("LotoBranch_Build_LinuxX64_Clang", "Clang", "linux-clang-debug")

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
