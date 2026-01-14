"""
Action to build the project using CMake and Ninja for a given preset.
"""

from ci import log, root
from ci.actions.base.action import BaseAction
from ci.utils.preset import get_build_dir
from ci.utils.run import run_command, MODE_BY_COLOR, MODE_FOR_NINJA


class Build(BaseAction):
    """
    Action to build the project using CMake and Ninja for a given preset.
    """

    def run(self, preset: str) -> int:
        """
        Compile the project for the given preset.
        :param preset: The preset to check.
        :return: Exit code indicating success or failure.
        """
        log.info(f"Building project with preset: {preset}")
        # run cmake configure, capture output in real-time, and print it using logging
        configure = run_command(
            ["cmake", "--preset", preset, "-S", str(root), "-G", "Ninja"],
            detection_mode=MODE_BY_COLOR,
        )
        if configure != 0:
            log.error("CMake configuration failed.")
            return configure
        build_dir = get_build_dir(preset)
        if not build_dir.exists():
            log.error(f"Build directory does not exist: {build_dir}")
            return 1
        build_result = run_command(
            ["cmake", "--build", str(build_dir)], detection_mode=MODE_FOR_NINJA
        )
        if build_result != 0:
            log.error("CMake build failed.")
            return build_result
        return 0
