import jetbrains.buildServer.configs.kotlin.*
import jetbrains.buildServer.configs.kotlin.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.triggers.vcs

/**
 * Waits for the given configurations, and does not start at all if one of them fails.
 *
 * `Code Style` gates everything: it costs seconds, it fails on a formatting slip alone,
 * and there is no point spending an agent on a full build — let alone nine — for a
 * change that will be sent back anyway. `reuseBuilds = SUCCESSFUL` means the chain
 * reuses the style build already green for that revision instead of running it again.
 *
 * @param gates The configurations that must be green first.
 */
fun Dependencies.after(vararg gates: BuildType) {
    gates.forEach { gate ->
        snapshot(gate) {
            onDependencyFailure = FailureAction.FAIL_TO_START
            onDependencyCancel = FailureAction.FAIL_TO_START
            reuseBuilds = ReuseBuilds.SUCCESSFUL
            runOnSameAgent = false
        }
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
