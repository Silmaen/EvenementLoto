"""
Select the translation units an analysis restricted to a diff must cover.
"""

import re
from pathlib import Path

# `#include "core/Log.h"` — the quoted form only. Every project header is included that
# way here, and resolving those needs no compiler, no compilation database and no tool
# that might be missing from the builder image. An angle-bracket include of a project
# header would go unseen; this codebase has none, and the full scan on `main` is the
# backstop anyway.
QUOTED_INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.MULTILINE)

SOURCE_SUFFIXES = (".cpp",)
HEADER_SUFFIXES = (".h",)


def _read(path: Path) -> str:
    """
    Read a file, tolerating anything unreadable or badly encoded.
    :param path: The file to read.
    :return: Its content, empty when it cannot be read.
    """
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return ""


def direct_includes(path: Path, roots: list[Path]) -> set[Path]:
    """
    Project headers a file includes directly, resolved to real files.

    A quoted include resolves against the including file's directory first — the C++
    rule — then against each source root. Anything resolving to no file in the
    repository is a system or dependency header, and is dropped.

    :param path: The file to inspect.
    :param roots: The source roots includes may be relative to.
    :return: The set of resolved header paths.
    """
    found: set[Path] = set()
    for target in QUOTED_INCLUDE.findall(_read(path)):
        for candidate in [path.parent / target] + [root / target for root in roots]:
            resolved = candidate.resolve()
            if resolved.is_file():
                found.add(resolved)
                break
    return found


def include_closure(path: Path, roots: list[Path]) -> set[Path]:
    """
    Every project header reachable from a file, transitively.

    Cycles are normal in C++ — include guards make them harmless — so the walk tracks
    what it has already seen instead of assuming a tree.

    :param path: The file to start from.
    :param roots: The source roots includes may be relative to.
    :return: The set of reachable header paths.
    """
    seen: set[Path] = set()
    pending = [path]
    while pending:
        for header in direct_includes(pending.pop(), roots):
            if header not in seen:
                seen.add(header)
                pending.append(header)
    return seen


def translation_units(roots: list[Path]) -> list[Path]:
    """
    Every translation unit under the source roots.
    :param roots: The source roots to scan.
    :return: The sorted list of resolved source paths.
    """
    units: list[Path] = []
    for root in roots:
        if not root.is_dir():
            continue
        units.extend(
            path.resolve()
            for path in sorted(root.rglob("*"))
            if path.is_file() and path.suffix in SOURCE_SUFFIXES
        )
    return units


def _under_roots(path: Path, roots: list[Path]) -> bool:
    """
    Tell whether a path lives under one of the source roots.
    :param path: The path to test.
    :param roots: The resolved source roots.
    :return: True when the path is inside the analysis scope.
    """
    return any(path.is_relative_to(root) for root in roots)


def select(roots: list[Path], changed: list[Path]) -> list[Path]:
    """
    Pick the translation units a diff requires analysing.

    A changed source is analysed directly. A changed **header** pulls in every
    translation unit that includes it, directly or transitively: analysing only the
    changed files would miss the diagnostics a header change causes in the sources
    using it, which is exactly where a gate restricted to a diff would let a real
    regression through.

    :param roots: The source roots forming the analysis scope.
    :param changed: The paths the pull request changed.
    :return: The sorted list of translation units to analyse.
    """
    roots = [root.resolve() for root in roots]
    changedPaths = {path.resolve() for path in changed}
    changedHeaders = {
        path
        for path in changedPaths
        if path.suffix in HEADER_SUFFIXES and _under_roots(path, roots)
    }

    selected = {
        path
        for path in changedPaths
        if path.suffix in SOURCE_SUFFIXES
        and path.is_file()
        and _under_roots(path, roots)
    }
    if changedHeaders:
        selected.update(
            unit
            for unit in translation_units(roots)
            if include_closure(unit, roots) & changedHeaders
        )
    return sorted(selected)
