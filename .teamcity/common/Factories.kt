import jetbrains.buildServer.configs.kotlin.*
import jetbrains.buildServer.configs.kotlin.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.buildSteps.script

/**
 * One build configuration per CMake preset: only the preset changes.
 *
 * The ids are set explicitly to the names the configurations already have on the
 * server: the portable DSL identifies a configuration by its id, and a new id would
 * lose its build history.
 */
fun presetBuild(idValue: String, buildName: String, cmakePreset: String,
                onDraft: Boolean = false, gates: List<BuildType> = listOf(codeStyle)) = BuildType {
    id = RelativeId(idValue)
    name = buildName
    templates(globalBuild)

    params {
        param("cmake_preset", cmakePreset)
    }

    if (onDraft) {
        features {
            githubBridge(triggerOnPrDraft = true)
        }
    }

    dependencies {
        after(*gates.toTypedArray())
    }
}

/**
 * clang-tidy, and the same binary restricted to the static analyzer checks.
 *
 * **One configuration per tool**, not one per scope: the scope is decided at run time by
 * `ci/actions/analysis.py`, from the pull request number the bridge publishes. Inside a
 * pull request the analysis is a gate — it looks at what changed, a changed header
 * pulling in every translation unit that includes it, and a finding fails the build so
 * the bridge pins it on the diff line it belongs to. Anywhere else it surveys the whole
 * codebase and only warns, because a finding elsewhere is not this commit's fault.
 *
 * These two configurations sit at the **end of the dependency chain**, which is what
 * makes them the only ones worth requiring before a merge: nothing reaches them unless
 * the style, every build and every sanitizer went green first.
 *
 * The base of the diff is the pull request's own merge base, which the bridge publishes
 * — GitHub's answer to "where did this branch start", not ours, and the very range the
 * bridge places its annotations against. The target branch comes along as the fallback.
 * These references live here and never in the template: a parameter reference TeamCity
 * cannot resolve turns into an implicit agent requirement, and the build would then
 * never start.
 *
 * @param idValue The configuration id, kept explicit like every other one.
 * @param buildName The configuration name, as it reads on the server.
 * @param tool `tidy` or `analyzer`.
 * @param gates What must be green first.
 */
fun analysisBuild(idValue: String, buildName: String, tool: String, gates: List<BuildType>) = BuildType {
    id = RelativeId(idValue)
    name = buildName
    templates(toolBuild)

    triggers {
        mainBranchOnly()
    }

    params {
        param("cmake_preset", "linux-analysis")
        param("analysis.tool", tool)
        param("analysis.pullRequest", "%teamcity.github.bridge.pullRequest.number%")
        param("analysis.merge-base", "%teamcity.github.bridge.pullRequest.mergeBase%")
        param("analysis.base.branch", "%teamcity.github.bridge.pullRequest.targetBranch%")
    }

    steps {
        script {
            name = "Analyse"
            id = "Analyse"
            scriptContent = "poetry run python3 -u ci_action.py Analysis %cmake_preset% " +
                            "--tool %analysis.tool% " +
                            "--pull-request '%analysis.pullRequest%' " +
                            "--merge-base '%analysis.merge-base%' " +
                            "--base '%analysis.base.branch%'"
            dockerImage = "%docker_image%"
            dockerImagePlatform = ScriptBuildStep.ImagePlatform.Linux
            dockerPull = true
            dockerRunParameters = "%docker_parameters%"
        }
    }

    features {
        githubBridge()
    }

    dependencies {
        after(*gates.toTypedArray())
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
 * @param tested The configuration that built and tested that same release preset. Owl
 *        packages in parallel with its tests; here it waits, because an archive built
 *        from code whose tests fail has no business existing.
 */
fun packageBuild(idValue: String, buildName: String, platformName: String,
                 cmakePreset: String, extension: String, tested: BuildType) = BuildType {
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

    dependencies {
        after(codeStyle, tested)
    }
}
