import jetbrains.buildServer.configs.kotlin.*
import jetbrains.buildServer.configs.kotlin.buildFeatures.XmlReport
import jetbrains.buildServer.configs.kotlin.buildFeatures.investigationsAutoAssigner
import jetbrains.buildServer.configs.kotlin.buildFeatures.xmlReport
import jetbrains.buildServer.configs.kotlin.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.buildSteps.script
import jetbrains.buildServer.configs.kotlin.triggers.vcs

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
