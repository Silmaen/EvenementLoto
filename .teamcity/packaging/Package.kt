import jetbrains.buildServer.configs.kotlin.*

// The Clang configurations are the ones that build and test the packaged release
// preset: `linux-clang-debug` declares `linux-clang-release`, and likewise on Windows.
// See ci/PresetsParameters.json.
val packageLinux = packageBuild("LotoBranch_Package_LinuxX64", "Linux x64", "Linux",
                                "linux-clang-release", "tgz", linuxClang)

val packageWindows = packageBuild("LotoBranch_Package_WindowsX64", "Windows x64", "Windows",
                                  "windows-clang-release", "zip", windowsClang)

val packaging = Project {
    id = RelativeId("LotoBranch_Package")
    name = "Package"
    description = "Delivering the Project"

    buildType(packageLinux)
    buildType(packageWindows)

    buildTypesOrder = arrayListOf(
            packageLinux,
            packageWindows,
    )
}
