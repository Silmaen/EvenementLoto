"""
Action to check the code style, without ever rewriting a file.
"""

from ci import log
from ci.actions.base.action import BaseAction

# Directories walked for C++ sources. `source/gui/fonts` is excluded: it holds generated
# `.embed` arrays, not code anyone formats.
CXX_ROOTS = ("source", "test")
CXX_SUFFIXES = (".h", ".cpp")
CXX_EXCLUDED = ("source/gui/fonts",)

# cmake-format is deliberately not run: with .cmake-format.json as it stands it
# explodes `target_link_libraries(X PUBLIC Y)` over four lines and mangles the
# file(GLOB_RECURSE) calls, which is less readable than the hand formatting in place.
# Tuning that file is its own job; until then the CMake sources are not gated.

PYTHON_ROOTS = ("ci", "ci_action.py")


def diagnostic(path, line: int, check: str, message: str, column: int = 1) -> None:
    """
    Report one finding as a compiler-style diagnostic.

    The shape matters beyond the log: the teamcity-github-bridge plugin scans a failed
    build for `path:line:column: level: message` and turns each one into a GitHub check
    run annotation pinned to that line of the pull request's diff. A finding printed in
    any other shape never leaves the build log.

    :param path: The file the finding is about, relative to the repository root.
    :param line: The 1-based line number, 1 for a whole-file finding.
    :param check: The sub-check name, kept as the first word of the message.
    :param message: What is wrong, on a single line.
    :param column: The 1-based column, when the tool reports one.
    """
    log.error(f"{path}:{line}:{column}: error: {check}: {message}")


class CodeStyle(BaseAction):
    """
    Action to check the code style of the C++ sources and of the CI scripts.
    """

    def run(self, preset: str) -> int:
        """
        Executes every style check and reports the findings.
        :param preset: Unused, kept for the common action signature.
        :return: Exit code indicating success or failure.
        """
        log.info("Starting code style checks...")
        findings = 0
        findings += self._check_clang_format()
        findings += self._check_black()
        if findings > 0:
            log.error(f"Code style: {findings} file(s) to fix.")
            return 1
        log.info("Code style: nothing to report.")
        return 0

    @staticmethod
    def _cxx_files() -> list:
        """
        Collect the C++ sources and headers to inspect.
        :return: The list of paths, relative to the repository root.
        """
        from pathlib import Path

        files = []
        for rootName in CXX_ROOTS:
            root = Path(rootName)
            if not root.is_dir():
                continue
            for path in sorted(root.rglob("*")):
                if path.suffix not in CXX_SUFFIXES:
                    continue
                if any(
                    path.as_posix().startswith(excluded) for excluded in CXX_EXCLUDED
                ):
                    continue
                files.append(path)
        return files

    def _check_clang_format(self) -> int:
        """
        Run clang-format in dry-run mode: it already reports its own findings in the
        diagnostic shape the bridge reads, so its output is relayed as is.
        :return: The number of files that would be reformatted.
        """
        from shutil import which
        from subprocess import run

        files = self._cxx_files()
        if not files:
            log.warning("clang-format: no C++ source found.")
            return 0
        tool = which("clang-format")
        if tool is None:
            diagnostic(
                ".clang-format", 1, "format", "clang-format is missing from the image"
            )
            return 1
        log.info(f"clang-format: checking {len(files)} file(s)...")
        # One call per file: clang-format stops at the first badly formatted file when
        # given several, so a single call would only ever report one finding.
        offenders = 0
        for path in files:
            result = run(
                [tool, "--dry-run", "--Werror", str(path)],
                capture_output=True,
                text=True,
            )
            if result.returncode == 0:
                continue
            offenders += 1
            for line in result.stderr.splitlines():
                if line.strip():
                    log.error(line)
        return offenders

    @staticmethod
    def _check_black() -> int:
        """
        Run black in check mode over the CI scripts.
        :return: The number of files that would be reformatted.
        """
        from pathlib import Path
        from shutil import which
        from subprocess import run

        targets = [name for name in PYTHON_ROOTS if Path(name).exists()]
        if not targets:
            log.warning("black: no Python source found.")
            return 0
        tool = which("black")
        if tool is None:
            diagnostic(
                "pyproject.toml", 1, "black", "black is missing from the environment"
            )
            return 1
        log.info(f"black: checking {', '.join(targets)}...")
        result = run(
            [tool, "--check", "--quiet", *targets], capture_output=True, text=True
        )
        if result.returncode == 0:
            return 0
        # Black reports "would reformat <path>" on stderr, one line per file.
        offenders = 0
        for line in result.stderr.splitlines():
            if line.startswith("would reformat "):
                offenders += 1
                diagnostic(
                    line.removeprefix("would reformat ").strip(),
                    1,
                    "black",
                    "file is not formatted",
                )
            elif line.strip():
                log.error(line)
        return max(offenders, 1)
