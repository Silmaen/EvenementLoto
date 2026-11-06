"""
GitLab Code Quality report, the channel through which a finding reaches the merge request.
"""

import re

from ci import log, root

# `path:line:col: level: message`, the shape every CI action prints its findings in.
# The optional drive letter keeps a Windows path in one piece.
FINDING = re.compile(
    r"^(?P<path>(?:[A-Za-z]:)?[^:]+):(?P<line>\d+):(?P<column>\d+): "
    r"(?P<level>warning|error): (?P<message>.*)$"
)

# clang-tidy and clang-format end a diagnostic with the check that raised it.
CHECK = re.compile(r"\[(?P<check>[^\]]+)\]$")

SEVERITY = {"error": "major", "warning": "minor"}


def report_dir():
    """
    :return: The directory the pipeline collects the reports from.
    """
    return root / "output" / "code-quality"


def write_report(name: str, tool: str, lines: list) -> None:
    """
    Write the findings as a Code Quality report.

    The report is written even when empty: GitLab compares it with the target branch's,
    and an empty one is what tells it the findings are resolved.

    :param name: The report file name, without extension.
    :param tool: The check name used when a finding does not carry its own.
    :param lines: The findings, in the shape `FINDING` matches; other lines are ignored.
    """
    import json
    from hashlib import sha256
    from pathlib import Path

    issues = []
    for line in lines:
        match = FINDING.match(line)
        if match is None:
            continue
        path = Path(match["path"])
        if path.is_absolute() and path.is_relative_to(root):
            path = path.relative_to(root)
        check = CHECK.search(match["message"])
        issues.append(
            {
                "description": match["message"],
                "check_name": check["check"] if check else tool,
                "fingerprint": sha256(f"{tool}:{line}".encode()).hexdigest(),
                "severity": SEVERITY[match["level"]],
                "location": {
                    "path": path.as_posix(),
                    "lines": {"begin": int(match["line"])},
                },
            }
        )
    output = report_dir() / f"{name}.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(issues, indent=2))
    log.info(f"Code Quality: {len(issues)} finding(s) written to {output}")
