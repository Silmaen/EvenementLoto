import jetbrains.buildServer.configs.kotlin.*
import jetbrains.buildServer.configs.kotlin.buildFeatures.XmlReport
import jetbrains.buildServer.configs.kotlin.buildFeatures.investigationsAutoAssigner
import jetbrains.buildServer.configs.kotlin.buildFeatures.xmlReport
import jetbrains.buildServer.configs.kotlin.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.buildSteps.script
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
        checkbox("run_deploy", "true", checked = "true", unchecked = "false")
    }

    vcs {
        root(githubLoto)
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
        script {
            name = "Deploy"
            id = "Deploy"

            conditions {
                doesNotMatch("release_preset", "^${'$'}")
                equals("run_deploy", "true")
            }
            scriptContent = "poetry run python3 -u ci_action.py Deploy %release_preset%"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
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
 * GitHub App bridge settings, kept explicit and identical to the current server state
 * rather than relying on the plugin defaults.
 *
 * @param triggerOnPrDraft also build pull requests still marked as draft.
 */
fun BuildFeatures.githubBridge(triggerOnPrDraft: Boolean = false) {
    feature {
        id = "github-bridge"
        type = "github-bridge"
        param("annotateDiff", "true")
        param("publishChecks", "true")
        param("runOnApproval", "true")
        param("triggerOnBranch", "true")
        param("triggerOnPrReady", "true")
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
fun presetBuild(idValue: String, buildName: String, cmakePreset: String,
                triggerOnPrDraft: Boolean = false) = BuildType {
    id = RelativeId(idValue)
    name = buildName
    templates(globalBuild)

    params {
        param("cmake_preset", cmakePreset)
    }

    if (triggerOnPrDraft) {
        features {
            githubBridge(triggerOnPrDraft = true)
        }
    }
}

val linuxGcc = presetBuild("LotoBranch_Build_LinuxX64_Gcc", "GCC", "linux-gcc-debug")
val linuxClang = presetBuild("LotoBranch_Build_LinuxX64_Clang", "Clang", "linux-clang-debug",
                             triggerOnPrDraft = true)

val clangTidy = presetBuild("LotoBranch_Build_LinuxX64_ClangTidy", "Clang-Tidy", "linux-clang-tidy")
val sanitizerAddress =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerAdress", "Sanitizer Address", "linux-sanitizer-address")
val sanitizerLeak =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerLeak", "Sanitizer Leak", "linux-sanitizer-leak")
val sanitizerThread =
        presetBuild("LotoBranch_Build_LinuxX64_SanitizerThread", "Sanitizer Thread", "linux-sanitizer-thread")
val sanitizerUndefinedBehavior = presetBuild("LotoBranch_Build_LinuxX64_SanitizerUndefinedBehavior",
                                             "Sanitizer Undefined Behavior", "linux-sanitizer-undefined-behavior")

val windowsGcc = presetBuild("LotoBranch_Build_WindowsX64_Gcc", "GCC", "windows-gcc-debug")
val windowsClang = presetBuild("LotoBranch_Build_WindowsX64_Clang", "Clang", "windows-clang-debug",
                               triggerOnPrDraft = true)

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

    buildType(clangTidy)
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
}

project {
    description = "Loto/Bingo Application"

    vcsRoot(githubLoto)

    template(globalBuild)

    params {
        param("loto_git_branch", "main")
        param("branch_specification", """
            +:refs/heads/(main)
            +:refs/heads/(Feature/*)
            +:refs/heads/(Experiment/*)
        """.trimIndent())
        // Builds are triggered by the GitHub App bridge, on branch pushes and on pull
        // requests marked ready, instead of a VCS trigger.
        param("teamcity.github.bridge.repo", "Silmaen/EvenementLoto")
        param("teamcity.github.bridge.connectionId", "PROJECT_EXT_5")
        param("teamcity.github.bridge.prBuildRef", "branch")
        param("teamcity.github.bridge.prTrigger.enabled", "true")
        param("teamcity.github.bridge.branchTrigger.enabled", "true")
        param("teamcity.github.bridge.annotations.enabled", "true")
    }

    subProject(build)
}
