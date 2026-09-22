import jetbrains.buildServer.configs.kotlin.*
import jetbrains.buildServer.configs.kotlin.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.buildSteps.script

/**
 * Style gate: it inspects and never rewrites, so a finding is the author's to fix.
 *
 * It sits at the root of the project rather than under a `Quality` folder: it gates
 * every other configuration, so that is where one should trip over it first.
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
