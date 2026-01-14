"""
Gathering tools for presets
"""
from ci import root, Path


def get_build_dir(preset: str) -> Path:
    """
    Get the build directory for a given preset.

    :param preset: The preset to get the build directory for.
    :return: The Path to the build directory.
    """
    return root / "output" / "build" / preset
