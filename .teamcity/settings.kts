import jetbrains.buildServer.configs.kotlin.*
import jetbrains.buildServer.configs.kotlin.buildFeatures.XmlReport
import jetbrains.buildServer.configs.kotlin.buildFeatures.investigationsAutoAssigner
import jetbrains.buildServer.configs.kotlin.buildFeatures.xmlReport
import jetbrains.buildServer.configs.kotlin.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.buildSteps.script
import jetbrains.buildServer.configs.kotlin.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.triggers.vcs
import jetbrains.buildServer.configs.kotlin.vcs.GitVcsRoot

/*
Settings of the "Evenement Loto" project.

Everything a build does lives in `ci/` and is driven by `ci_action.py <Action> <preset>`:
this file only describes which presets exist and where they run. Whether tests,
coverage, documentation or deployment run for a given preset — and which docker image
to use — is described once in `ci/PresetsParameters.json` and pushed as TeamCity
parameters at run time by the `DefineVariables` and `DefineDockerImage` steps. It must
not be duplicated here.

The root project stays managed from the server UI: it holds the shared parameters
(`docker_image`, `docker_registry`, `deploy_*`…), the GitHub App connection and the
uploaded SSH key. Nothing secret appears in this file.

To edit in IntelliJ Idea, open `pom.xml` and choose 'Open as a project'.
*/

version = "2026.2"

val githubLoto = GitVcsRoot {
    id = RelativeId("GithubLoto")
    name = "github loto"
    url = "git@github.com:Silmaen/EvenementLoto.git"
    branch = "refs/heads/%loto_git_branch%"
    branchSpec = "%branch_specification%"
    userNameStyle = GitVcsRoot.UserNameStyle.NAME
    agentCleanPolicy = GitVcsRoot.AgentCleanPolicy.ALWAYS
    agentCleanFilesPolicy = GitVcsRoot.AgentCleanFilesPolicy.ALL_UNTRACKED
    checkoutPolicy = GitVcsRoot.AgentCheckoutPolicy.USE_MIRRORS
    checkoutSubmodules = GitVcsRoot.CheckoutSubmodules.SUBMODULES_CHECKOUT
    authMethod = uploadedKey {
        uploadedKey = "github connexion"
    }
    // Kept explicit: the generated export dropped it, and without it the agent would
    // refuse the GitHub host key.
    param("ignoreKnownHosts", "true")
}

val globalBuild = Template {
    id = RelativeId("GlobalBuild")
    name = "Global Build"
    description = "build in a docker"

    artifactRules = "%artifact_path%"

    params {
        param("cmake_preset", "")
        // Filled at run time by the DefineVariables and DefineDockerImage steps, from
        // ci/PresetsParameters.json. Declared here only so they can be overwritten.
        param("release_preset", "")
        param("artifact_path", "")
        param("docker_parameters", "")
        checkbox("run_tests", "true", checked = "true", unchecked = "false")
        checkbox("run_coverage", "false", checked = "true", unchecked = "false")
        checkbox("run_documentation", "false", checked = "true", unchecked = "false")
    }

    vcs {
        root(githubLoto)
    }

    triggers {
        mainBranchOnly()
    }

    steps {
        // Runs on the agent itself: it is the step that decides which image the next
        // ones run in.
        script {
            name = "Determine docker"
            id = "RUNNER_24"
            scriptContent = "python3 -u ci_action.py DefineDockerImage %cmake_preset%"
        }
        script {
            name = "Tool Dependencies"
            id = "Update_remote"
            scriptContent = """
                python3 -u ci_action.py PythonRequirements %cmake_preset%
                poetry run python3 -u ci_action.py DefineVariables %cmake_preset%
            """.trimIndent()
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
        script {
            name = "Clean"
            id = "Clean_Output_Folder"
            scriptContent = "poetry run python3 -u ci_action.py Clean %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
        script {
            name = "Build"
            id = "Build_Release"
            scriptContent = "poetry run python3 -u ci_action.py Build %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
        script {
            name = "Test"
            id = "Test_Release"

            conditions {
                equals("run_tests", "true")
            }
            scriptContent = "poetry run python3 -u ci_action.py Test %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
        script {
            name = "Code Coverage"
            id = "Code_Coverage"

            conditions {
                equals("run_tests", "true")
                equals("run_coverage", "true")
            }
            scriptContent = "poetry run python3 -u ci_action.py Coverage %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerRunParameters = "%docker_parameters%"
        }
        // The steps below build the release preset of the same toolchain, when the
        // preset declares one. Their ids say "Debug" for historical reasons.
        script {
            name = "Clean Release"
            id = "Clean_Release"

            conditions {
                doesNotMatch("release_preset", "^${'$'}")
            }
            scriptContent = "poetry run python3 -u ci_action.py Clean %release_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
        script {
            name = "Build Release"
            id = "Build_Debug"

            conditions {
                doesNotMatch("release_preset", "^${'$'}")
            }
            scriptContent = "poetry run python3 -u ci_action.py Build %release_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
        script {
            name = "Test Release"
            id = "Test_Debug"

            conditions {
                doesNotMatch("release_preset", "^${'$'}")
                equals("run_tests", "true")
            }
            scriptContent = "poetry run python3 -u ci_action.py Test %release_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
        script {
            name = "Documentation"
            id = "Documentation"

            conditions {
                equals("run_documentation", "true")
            }
            scriptContent = "poetry run python3 -u ci_action.py Documentation %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerRunParameters = "%docker_parameters%"
        }
    }

    features {
        investigationsAutoAssigner {
            id = "InvestigationsAutoAssigner"
        }
        xmlReport {
            id = "BUILD_EXT_8"
            reportType = XmlReport.XmlReportType.GOOGLE_TEST
            rules = "output/build/**/test/*_Report.xml"
            verbose = true
        }
        githubBridge()
    }

    requirements {
        contains("teamcity.agent.jvm.os.name", "%platform%", "RQ_18")
        contains("teamcity.agent.jvm.os.arch", "amd64", "RQ_4")
    }
}

/**
 * Shared skeleton for the configurations that do not build the project.
 *
 * `globalBuild` chains eleven steps — build, test, coverage, documentation, deploy —
 * and a style check needs none of them. Only the two that every containerised action
 * needs are kept: the one that decides the image, on the agent, then the one that
 * provisions the Python environment inside it.
 */
val toolBuild = Template {
    id = RelativeId("ToolBuild")
    name = "Tool Build"
    description = "run one ci_action in a docker"

    params {
        param("cmake_preset", "")
        // Filled at run time by the Determine docker step.
        param("docker_parameters", "")
        // Overridden by the sub-project, as in globalBuild.
        param("platform", "Linux")
    }

    vcs {
        root(githubLoto)
    }

    steps {
        script {
            name = "Determine docker"
            id = "Tool_Determine_Docker"
            scriptContent = "python3 -u ci_action.py DefineDockerImage %cmake_preset%"
        }
        script {
            name = "Tool Dependencies"
            id = "Tool_Dependencies"
            scriptContent = "python3 -u ci_action.py PythonRequirements %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
    }

    features {
        investigationsAutoAssigner {
            id = "Tool_InvestigationsAutoAssigner"
        }
    }

    requirements {
        contains("teamcity.agent.jvm.os.name", "%platform%", "RQ_TOOL_OS")
        contains("teamcity.agent.jvm.os.arch", "amd64", "RQ_TOOL_ARCH")
    }
}

/**
 * Builds a push to `main` — and nothing else.
 *
 * The bridge plugin enqueues builds from pull request events only: its webhook
 * controller ignores `push`, and `branchTrigger.enabled` has no runtime consumer in
 * 1.10.0. Without this trigger a push to main would build nothing at all. Scoped to
 * main, so pull requests stay the bridge's job and nothing is built twice.
 */
fun Triggers.mainBranchOnly() {
    vcs {
        id = "vcsTrigger"
        branchFilter = """
            +:main
            +:refs/heads/main
        """.trimIndent()
        enableQueueOptimization = true
        quietPeriodMode = VcsTrigger.QuietPeriodMode.DO_NOT_USE
    }
}

/**
 * GitHub App bridge settings, kept explicit and identical to the current server state
 * rather than relying on the plugin defaults.
 *
 * @param triggerOnPrDraft also build pull requests still marked as draft. Reserved for
 *        the style gate: a full build on every push to a draft is a waste.
 * @param triggerOnPrReady build pull requests at all. Off for the Package
 *        configurations, which have no business running before a merge.
 * @param annotateDiff write the findings on the diff. Off for the full analyses: a
 *        finding in code two years old does not belong on this pull request's lines.
 */
fun BuildFeatures.githubBridge(triggerOnPrDraft: Boolean = false,
                              triggerOnPrReady: Boolean = true,
                              annotateDiff: Boolean = true) {
    feature {
        id = "github-bridge"
        type = "github-bridge"
        param("annotateDiff", annotateDiff.toString())
        param("publishChecks", "true")
        param("runOnApproval", "true")
        param("triggerOnBranch", "true")
        param("triggerOnPrReady", triggerOnPrReady.toString())
        if (triggerOnPrDraft)
            param("triggerOnPrDraft", "true")
    }
}

/**
 * One build configuration per CMake preset: only the preset changes.
 *
 * The ids are set explicitly to the names the configurations already have on the
 * server: the portable DSL identifies a configuration by its id, and a new id would
 * lose its build history.
 */
fun presetBuild(idValue: String, buildName: String, cmakePreset: String) = BuildType {
    id = RelativeId(idValue)
    name = buildName
    templates(globalBuild)

    params {
        param("cmake_preset", cmakePreset)
    }
}

val linuxGcc = presetBuild("LotoBranch_Build_LinuxX64_Gcc", "GCC", "linux-gcc-debug")
val linuxClang = presetBuild("LotoBranch_Build_LinuxX64_Clang", "Clang", "linux-clang-debug")

/**
 * clang-tidy, and the same binary restricted to the static analyzer checks.
 *
 * Two knobs, both parameters, so one factory covers the four configurations — tool ×
 * scope — and a manual run can widen a gate to the whole codebase without a second
 * configuration existing for it. See `ci/actions/analysis.py` for what they do.
 *
 * The two scopes differ in kind, not in degree:
 *
 *  - **on diff** is the gate. It analyses only what the pull request changed, a changed
 *    header pulling in every translation unit that includes it, and a finding fails the
 *    build so the bridge pins it on the diff line it belongs to. Ready pull requests
 *    only, and no push: a diff against `main` taken on `main` is empty.
 *  - **full** scans everything on `main`, and only warns. A finding elsewhere in the
 *    codebase is not this pull request's problem, and failing over it would teach
 *    everyone to ignore the gate. It does not annotate either.
 *
 * The base of the diff is the pull request's own merge base, which the bridge publishes
 * — GitHub's answer to "where did this branch start", not ours, and the very range the
 * bridge places its annotations against. The target branch comes along as the fallback.
 * These overrides live here and never in the template: a parameter reference TeamCity
 * cannot resolve turns into an implicit agent requirement, and the build would then
 * never start.
 *
 * @param idValue The configuration id, kept explicit like every other one.
 * @param buildName The configuration name, as it reads on the server.
 * @param tool `tidy` or `analyzer`.
 * @param onDiff True for the pull request gate, false for the full scan on main.
 */
fun analysisBuild(idValue: String, buildName: String, tool: String, onDiff: Boolean) = BuildType {
    id = RelativeId(idValue)
    name = buildName
    templates(toolBuild)

    if (!onDiff) {
        triggers {
            mainBranchOnly()
        }
    }

    params {
        param("cmake_preset", "linux-analysis")
        param("analysis.tool", tool)
        param("analysis.mode", if (onDiff) "diff" else "full")
        param("analysis.onFindings", if (onDiff) "fail" else "warn")
        param("analysis.merge-base",
              if (onDiff) "%teamcity.github.bridge.pullRequest.mergeBase%" else "")
        param("analysis.base.branch",
              if (onDiff) "%teamcity.github.bridge.pullRequest.targetBranch%" else "main")
    }

    steps {
        script {
            name = "Analyse"
            id = "Analyse"
            scriptContent = "poetry run python3 -u ci_action.py Analysis %cmake_preset% " +
                            "--tool %analysis.tool% " +
                            "--mode %analysis.mode% " +
                            "--on-findings %analysis.onFindings% " +
                            "--merge-base '%analysis.merge-base%' " +
                            "--base '%analysis.base.branch%'"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
    }

    features {
        githubBridge(triggerOnPrReady = onDiff, annotateDiff = onDiff)
    }
}

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
val sanitizerAddress =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerAdress", "Sanitizer Address", "linux-sanitizer-address")
val sanitizerLeak =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerLeak", "Sanitizer Leak", "linux-sanitizer-leak")
val sanitizerThread =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerThread", "Sanitizer Thread", "linux-sanitizer-thread")
val sanitizerUndefinedBehavior = presetBuild("LotoBranch_Build_LinuxX64_SanitizerUndefinedBehavior",
                                             "Sanitizer Undefined Behavior", "linux-sanitizer-undefined-behavior")

val windowsGcc = presetBuild("LotoBranch_Build_WindowsX64_Gcc", "GCC", "windows-gcc-debug")
val windowsClang = presetBuild("LotoBranch_Build_WindowsX64_Clang", "Clang", "windows-clang-debug")

/**
 * Style gate: it inspects and never rewrites, so a finding is the author's to fix.
 *
 * The only configuration that also runs on **draft** pull requests. It costs seconds
 * and a formatting slip is precisely what one wants to hear about early, while the
 * heavy builds wait for the pull request to be marked ready.
 *
 * `code-style` is not a CMake preset — nothing is configured here. It is the key
 * `ci/PresetsParameters.json` uses to hand out the docker image, and without an entry
 * there the action would run on the agent itself instead of in the container.
 */
val codeStyle = BuildType {
    id = RelativeId("LotoBranch_Build_Quality_CodeStyle")
    name = "Code Style"
    templates(toolBuild)

    triggers {
        mainBranchOnly()
    }

    params {
        param("cmake_preset", "code-style")
    }

    steps {
        script {
            name = "Checking Code"
            id = "Checking_Code"
            scriptContent = "poetry run python3 -u ci_action.py CodeStyle %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
    }

    features {
        githubBridge(triggerOnPrDraft = true)
    }
}

/**
 * Produces the single ready-to-run archive, and nothing else.
 *
 * Never triggered by a pull request: `triggerOnPrReady = false` and no draft either, so
 * only a push to `main` — through the template's VCS trigger — or a manual run packages
 * anything. `publishChecks` stays on, so the row still appears on the commit it built.
 *
 * It configures and builds the release preset itself instead of borrowing another
 * configuration's tree: the archive is CPack's output and CPack needs the build tree it
 * was configured in.
 *
 * @param idValue The configuration id, kept explicit like every other one.
 * @param buildName The configuration name, as it reads on the server.
 * @param platformName The agent platform the requirement matches, `Linux` or `Windows`.
 * @param cmakePreset The release preset to build and pack.
 * @param extension The archive extension CPack leaves behind, `tgz` or `zip`.
 */
fun packageBuild(idValue: String, buildName: String, platformName: String,
                 cmakePreset: String, extension: String) = BuildType {
    id = RelativeId(idValue)
    name = buildName
    templates(toolBuild)

    // Only the archive: no BuildArtefact.zip, no Coverage.zip, no documentation.
    artifactRules = "+:output/build/$cmakePreset/EvenementLoto-*.$extension"

    triggers {
        mainBranchOnly()
    }

    params {
        param("cmake_preset", cmakePreset)
        param("platform", platformName)
    }

    steps {
        script {
            name = "Clean"
            id = "Package_Clean"
            scriptContent = "poetry run python3 -u ci_action.py Clean %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
        script {
            name = "Package"
            id = "Package_Archive"
            scriptContent = "poetry run python3 -u ci_action.py Package %cmake_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
    }

    features {
        githubBridge(triggerOnPrReady = false)
    }
}

val packageLinux = packageBuild("LotoBranch_Package_LinuxX64", "Linux x64", "Linux",
                                "linux-clang-release", "tgz")
val packageWindows = packageBuild("LotoBranch_Package_WindowsX64", "Windows x64", "Windows",
                                  "windows-clang-release", "zip")

val packaging = Project {
    id = RelativeId("LotoBranch_Package")
    name = "Package"
    description = "Delivering the Project"

    buildType(packageLinux)
    buildType(packageWindows)
}

val linuxX64 = Project {
    id = RelativeId("LotoBranch_Build_LinuxX64")
    name = "Linux x64 ubuntu24.04"

    buildType(linuxGcc)
    buildType(linuxClang)

    params {
        param("platform", "Linux")
    }
}

val quality = Project {
    id = RelativeId("LotoBranch_Build_Quality")
    name = "Quality"

    buildType(codeStyle)
    buildType(clangTidyDiff)
    buildType(staticAnalyzerDiff)
    buildType(clangTidy)
    buildType(staticAnalyzer)
    buildType(sanitizerAddress)
    buildType(sanitizerLeak)
    buildType(sanitizerThread)
    buildType(sanitizerUndefinedBehavior)

    params {
        param("platform", "Linux")
    }
}

val windowsX64 = Project {
    id = RelativeId("LotoBranch_Build_WindowsX64")
    name = "Windows x64"

    buildType(windowsGcc)
    buildType(windowsClang)

    params {
        param("platform", "Windows")
    }
}

val build = Project {
    id = RelativeId("LotoBranch_Build")
    name = "Build"
    description = "Building the Project"

    subProject(linuxX64)
    subProject(quality)
    subProject(windowsX64)
    subProject(packaging)
}

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

    subProject(build)
}
