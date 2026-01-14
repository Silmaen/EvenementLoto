"""
Action to clean the project using CMake and Ninja for a given preset.
"""

from ci import log
from ci.actions.base.action import BaseAction
from ci.utils.preset import get_build_dir


class Clean(BaseAction):
    """
    Action to clean the project using CMake and Ninja for a given preset.
    """

    def run(self, preset: str) -> int:
        """
        Clean the project build artifacts for the given preset.
        :param preset: The preset to check.
        :return: Exit code indicating success or failure.
        """
        log.info(f"Cleaning build artifacts with preset: {preset}")
        build_dir = get_build_dir(preset)
        if build_dir.exists():
            import shutil

            shutil.rmtree(build_dir, ignore_errors=True)
            log.info(f"Removed build directory: {build_dir}")
        else:
            log.info(f"No build directory to remove: {build_dir}")
        return 0
