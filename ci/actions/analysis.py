"""
Action to run clang-tidy or the Clang static analyzer, over the codebase or over a diff.
"""

import re

from ci import log, root
from ci.actions.base.action import BaseAction
from ci.utils.preset import get_build_dir
from ci.utils.run import run_command, run_command_capture_output, MODE_BY_COLOR

# The analysis scope. The tests are deliberately out: a finding in a test fixture is not
# what this gate is for, and the checks are tuned for the product code.
SOURCE_ROOTS = ("source",)

# `path:line:col: warning: message [check]` — the shape the bridge's annotation parser
# reads. Continuation lines (`note:`) belong to the diagnostic above them and are not
# matched, or a finding would be counted twice.
FINDING = re.compile(r"^\S[^:]*:\d+:\d+: (?:warning|error): ")

# GitHub accepts 50 annotations per check run, and a wall of findings makes a build page
# unreadable. The count in the summary stays exact.
MAX_REPORTED = 40

# Windows caps a command line around 32 000 characters, and a full scan would go past
# it. Batching costs nothing on Linux.
TARGETS_PER_CALL = 40


class Analysis(BaseAction):
    """
    Action to run clang-tidy, or the same binary restricted to the static analyzer
    checks, over every translation unit or only over the ones a diff touches.
    """

    def __init__(self):
        self.tool = "tidy"
        self.mode = "full"
        self.on_findings = "warn"
        self.merge_base = ""
        self.base = "main"

    def with_options(self, options: list[str]) -> BaseAction:
        """
        Parse the options the entry point handed over.

        Options rather than environment variables: the exact command then reads in the
        build log and in the TeamCity step, and argparse rejects a typo instead of the
        action discovering it half way through.

        :param options: The leftover command-line arguments.
        :return: The action, configured.
        """
        from argparse import ArgumentParser

        parser = ArgumentParser("Analysis")
        parser.add_argument(
            "--tool",
            choices=("tidy", "analyzer"),
            default="tidy",
            help="tidy: the check set in .clang-tidy. analyzer: the same binary, "
            "clang-analyzer-* checks only.",
        )
        parser.add_argument(
            "--mode",
            choices=("auto", "full", "diff"),
            default="auto",
            help="auto: diff inside a pull request, full elsewhere. full: every "
            "translation unit. diff: only the ones the change touches, a changed "
            "header pulling in every unit that includes it.",
        )
        parser.add_argument(
            "--on-findings",
            choices=("auto", "warn", "fail"),
            default="auto",
            help="auto: fail on a diff, warn on a full scan. warn: report and stay "
            "green. fail: a finding fails the build.",
        )
        parser.add_argument(
            "--pull-request",
            default="",
            metavar="NUMBER",
            help="the pull request being built, as the bridge publishes it. Empty "
            "outside a pull request, which is what `auto` reads.",
        )
        parser.add_argument(
            "--merge-base",
            default="",
            metavar="SHA",
            help="the commit the diff starts from, normally the pull request's merge "
            "base as GitHub computed it. Empty falls back to --base.",
        )
        parser.add_argument(
            "--base",
            default="main",
            metavar="BRANCH",
            help="the branch the diff is taken against, used when --merge-base is "
            "empty.",
        )
        parsed = parser.parse_args(options)
        self.tool = parsed.tool
        self.merge_base = parsed.merge_base.strip()
        self.base = parsed.base.strip() or "main"

        # One configuration, two jobs. Inside a pull request the analysis is a gate: it
        # looks at what changed and a finding is an error. Anywhere else it is a survey
        # of the whole codebase, and a finding elsewhere is not this commit's fault.
        insidePullRequest = parsed.pull_request.strip() != ""
        self.mode = parsed.mode
        if self.mode == "auto":
            self.mode = "diff" if insidePullRequest else "full"
        self.on_findings = parsed.on_findings
        if self.on_findings == "auto":
            self.on_findings = "fail" if self.mode == "diff" else "warn"
        log.info(
            f"Analyse : {self.tool}, portée {self.mode}, "
            f"constat {'bloquant' if self.on_findings == 'fail' else 'consultatif'}"
            + (
                f", pull request {parsed.pull_request.strip()}"
                if insidePullRequest
                else ""
            )
        )
        return self

    def run(self, preset: str) -> int:
        """
        Configure the analysis tree, pick the translation units and analyse them.
        :param preset: The preset producing the compilation database.
        :return: Exit code indicating success or failure.
        """
        from shutil import which

        binary = which("clang-tidy") or which("clang-tidy-22")
        if binary is None:
            log.error("clang-tidy is missing from the image.")
            return 1

        # Configure only: the compilation database is a generate-time product, and no
        # object file is needed to analyse a translation unit.
        configure = run_command(
            ["cmake", "--preset", preset, "-S", str(root), "-G", "Ninja"],
            detection_mode=MODE_BY_COLOR,
        )
        if configure != 0:
            log.error("CMake configuration failed.")
            return configure
        buildDir = get_build_dir(preset)
        if not (buildDir / "compile_commands.json").is_file():
            log.error(f"No compile_commands.json in {buildDir}")
            return 1

        targets = self._targets()
        if targets is None:
            # The range could not be established. A gate that does not know what it is
            # supposed to cover must be red, never silently green.
            message = "Unable to determine what the change touches."
            if self.on_findings == "fail":
                log.error(message)
                return 1
            log.warning(message)
            return 0
        if not targets:
            log.info("Nothing to analyse.")
            return 0

        label = "clang-tidy" if self.tool == "tidy" else "clang static analyzer"
        log.info(f"Running {label} over {len(targets)} translation unit(s)...")
        return self._report(self._analyse(binary, buildDir, targets))

    def _targets(self):
        """
        Pick the translation units to analyse, according to the mode.
        :return: The list of paths, or None when the diff range is unknown.
        """
        from pathlib import Path
        from ci.utils import changed_tus

        roots = [root / name for name in SOURCE_ROOTS]
        if self.mode == "full":
            return changed_tus.translation_units(roots)

        changed = self._changed_paths()
        if changed is None:
            return None
        if not changed:
            log.info("The change touches no file under the analysis scope.")
            return []
        selected = changed_tus.select(roots, [Path(p) for p in changed])
        log.info(f"{len(changed)} changed file(s) select {len(selected)} unit(s).")
        return selected

    def _changed_paths(self):
        """
        Ask git which files the change touched.

        The merge base GitHub itself computed is preferred over the branch name, and for
        a better reason than saving a fetch: it is the range the bridge places its
        annotations against, so the analysed units and the lines GitHub is willing to
        pin a finding on cannot disagree.

        An empty merge base falls back to the branch name, which needs that branch in
        the checkout — so the plugin's server setting `mergeBase.enabled` being on is
        what keeps this path from being taken.

        :return: The changed paths relative to the repository root, or None when the
                 range could not be established at all.
        """
        start = self.merge_base
        if start == "":
            log.info(f"No merge base published, diffing against '{self.base}'.")
            code, out = run_command_capture_output(
                ["git", "merge-base", "HEAD", f"origin/{self.base}"]
            )
            start = (
                out.strip().splitlines()[-1].strip()
                if code == 0 and out.strip()
                else ""
            )
            if start == "":
                log.error(
                    f"No merge base with 'origin/{self.base}': the agent has no such "
                    "branch, and the bridge published none."
                )
                return None
        log.info(f"Diffing from {start}")
        code, out = run_command_capture_output(
            ["git", "diff", "--name-only", "--diff-filter=d", start, "HEAD"]
        )
        if code != 0:
            log.error("git diff failed.")
            return None
        return [line.strip() for line in out.splitlines() if line.strip()]

    def _analyse(self, binary: str, buildDir, targets: list) -> list:
        """
        Run the tool over the targets and collect the diagnostics.
        :param binary: The clang-tidy executable.
        :param buildDir: The directory holding compile_commands.json.
        :param targets: The translation units to analyse.
        :return: The diagnostic lines, in order, without duplicates.
        """
        from subprocess import run

        command = [binary, "-p", str(buildDir), "--quiet"]
        if self.tool == "analyzer":
            command.append("--checks=-*,clang-analyzer-*")

        findings: list[str] = []
        seen: set[str] = set()
        for start in range(0, len(targets), TARGETS_PER_CALL):
            batch = [str(path) for path in targets[start : start + TARGETS_PER_CALL]]
            result = run(command + batch, capture_output=True, text=True, cwd=root)
            for line in (result.stdout + result.stderr).splitlines():
                if FINDING.match(line) and line not in seen:
                    seen.add(line)
                    findings.append(line)
        return findings

    def _report(self, findings: list) -> int:
        """
        Log the findings and decide the outcome.
        :param findings: The diagnostic lines collected.
        :return: Exit code indicating success or failure.
        """
        if not findings:
            log.info("Analysis: nothing to report.")
            return 0
        for line in findings[:MAX_REPORTED]:
            if self.on_findings == "fail":
                log.error(line)
            else:
                log.warning(line)
        hidden = len(findings) - MAX_REPORTED
        if hidden > 0:
            log.info(f"{hidden} further finding(s) not reported.")
        if self.on_findings == "fail":
            log.error(f"Analysis: {len(findings)} finding(s).")
            return 1
        log.warning(f"Analysis: {len(findings)} finding(s), not failing the build.")
        return 0
