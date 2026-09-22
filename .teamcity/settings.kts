import jetbrains.buildServer.configs.kotlin.*

/*
Entry point of the "Evenement Loto" project.

This file holds nothing but the project itself: its parameters, the VCS root and the
templates it registers, and the sub-projects in the order the server displays them.
Everything else is split to mirror that hierarchy, and every directory below has to be
listed in `pom.xml` under `sourceDirs` or the compiler never sees it. `quality/` groups
three configurations that belong together; it is no longer a project level:

    common/Vcs.kt         the git VCS root
    common/Templates.kt   Global Build (the eleven steps) and Tool Build (two steps)
    common/Helpers.kt     the VCS trigger, the GitHub bridge feature, the dependencies
    common/Factories.kt   one function per family of configuration
    quality/CodeStyle.kt  the gate every other configuration waits for
    quality/Analysis.kt   clang-tidy and the static analyzer, both scopes
    quality/Sanitizers.kt the four sanitizers
    build/Linux.kt        Build Linux x64
    build/Windows.kt      Build Windows x64
    packaging/Package.kt  the two ready-to-run archives

A declaration cannot live in this file if another file needs it: the top-level values of
a `.kts` are members of the script's own class, invisible from a `.kt` beside it. That is
why only `project { }` remains here.

Everything a build does lives in `ci/` and is driven by `ci_action.py <Action> <preset>`:
the DSL only describes which presets exist and where they run. Whether tests, coverage
or documentation run for a given preset — and which docker image to use — is described
once in `ci/PresetsParameters.json` and pushed as TeamCity parameters at run time by the
`DefineVariables` and `DefineDockerImage` steps. It must not be duplicated here.

The root project stays managed from the server UI: it holds the shared parameters
(`docker_image`, `docker_registry`, `deploy_*`…), the GitHub App connection and the
uploaded SSH key. Nothing secret appears in this repository.

To edit in IntelliJ Idea, open `pom.xml` and choose 'Open as a project'.
*/

version = "2026.2"

project {
    description = "Loto/Bingo Application"

    vcsRoot(githubLoto)

    template(globalBuild)
    template(toolBuild)

    params {
        param("loto_git_branch", "main")
        param("branch_specification", """
            +:refs/heads/(main)
            +:refs/heads/(Feature/*)
            +:refs/heads/(Experiment/*)
        """.trimIndent())
        // Pull requests are built by the GitHub App bridge; branch pushes are not, so
        // main relies on the template's VCS trigger.
        param("teamcity.github.bridge.repo", "Silmaen/EvenementLoto")
        param("teamcity.github.bridge.connectionId", "PROJECT_EXT_5")
        param("teamcity.github.bridge.prBuildRef", "branch")
        param("teamcity.github.bridge.prTrigger.enabled", "true")
        param("teamcity.github.bridge.branchTrigger.enabled", "true")
        param("teamcity.github.bridge.annotations.enabled", "true")
    }

    // The gate, at the root: no `Quality` folder around it. That level carried nothing
    // but a `platform` parameter its two sub-projects already declared themselves.
    buildType(codeStyle)

    subProject(analysis)
    subProject(sanitizers)
    subProject(linuxX64)
    subProject(windowsX64)
    subProject(packaging)

    // Declared, because the order of the calls above is not what TeamCity displays: it
    // keeps an order of its own, which the UI lets one drag around. What reads the code
    // first, then what runs it, then the builds, then what ships.
    subProjectsOrder = arrayListOf(
            RelativeId("LotoBranch_Analysis"),
            RelativeId("LotoBranch_Sanitizers"),
            RelativeId("LotoBranch_Build_LinuxX64"),
            RelativeId("LotoBranch_Build_WindowsX64"),
            RelativeId("LotoBranch_Package"),
    )
}
