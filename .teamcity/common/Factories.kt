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
fun presetBuild(idValue: String, buildName: String, cmakePreset: String) = BuildType {
    id = RelativeId(idValue)
    name = buildName
    templates(globalBuild)

    params {
        param("cmake_preset", cmakePreset)
    }

    dependencies {
        after(codeStyle)
    }
}

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

    dependencies {
        after(codeStyle)
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
