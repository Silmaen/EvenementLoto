"""
Cmake utility functions for CI scripts.
"""
from ci import log


def list_cmake_presets() -> list[str]:
    """
    List all available CMake presets for the project.
    :return: A list of available CMake presets.
    """
    from ci.utils.run import run_command_capture_output
    result, out = run_command_capture_output(["cmake", "--list-presets"])
    presets = []
    if result == 0:
        for line in out.splitlines():
            if not '"' in line:
                continue
            preset = line.split('"')[1]
            presets.append(preset)
    else:
        log.error(f"Failed to list CMake presets {result}.")
    if len(presets) == 0:
        log.warning("No CMake presets found.")
    return presets


def cmake_preset_exists(preset: str) -> bool:
    """
    Check if a given CMake preset exists.
    :param preset: The preset to check.
    :return: True if the preset exists, False otherwise.
    """
    return preset in list_cmake_presets()
