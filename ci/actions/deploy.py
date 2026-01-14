"""
Action to deploy the project using CPack for a given preset.
"""

from ci import log, root
from ci.actions.base.action import BaseAction
from ci.utils.preset import get_build_dir
from ci.utils.run import run_command, MODE_BY_COLOR, MODE_FOR_NINJA


class Deploy(BaseAction):
    """
    Action to deploy the project using CPack for a given preset.
    """

    def run(self, preset: str) -> int:
        """
        Deploy the project using CPack for the given preset.
        :param preset: The preset to check.
        :return: Exit code indicating success or failure.
        """
        log.info(f"Deploying project with preset: {preset}")
        build_dir = get_build_dir(preset)
        # clean build directory first
        if build_dir.exists():
            import shutil

            shutil.rmtree(build_dir, ignore_errors=True)
            log.info(f"Removed existing build directory: {build_dir}")

        configure = run_command(
            ["cmake", "--preset", preset, "-S", str(root), "-G", "Ninja"],
            detection_mode=MODE_BY_COLOR,
        )
        if configure != 0:
            log.error("CMake configuration failed.")
            return configure
        if not build_dir.exists():
            log.error(f"Build directory does not exist: {build_dir}")
            return 1
        build_result = run_command(
            ["cmake", "--build", str(build_dir), "--target", "Package"], detection_mode=MODE_FOR_NINJA
        )
        if build_result != 0:
            log.error("CMake build failed.")
            return build_result
        return 0
