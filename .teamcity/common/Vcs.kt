import jetbrains.buildServer.configs.kotlin.*
import jetbrains.buildServer.configs.kotlin.vcs.GitVcsRoot

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
