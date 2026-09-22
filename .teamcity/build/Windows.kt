import jetbrains.buildServer.configs.kotlin.*

val windowsGcc = presetBuild("LotoBranch_Build_WindowsX64_Gcc", "GCC", "windows-gcc-debug")

val windowsClang = presetBuild("LotoBranch_Build_WindowsX64_Clang", "Clang", "windows-clang-debug")

val windowsX64 = Project {
    id = RelativeId("LotoBranch_Build_WindowsX64")
    name = "Build Windows x64"

    buildType(windowsGcc)
    buildType(windowsClang)

    buildTypesOrder = arrayListOf(
            windowsGcc,
            windowsClang,
    )

    params {
        param("platform", "Windows")
    }
}
