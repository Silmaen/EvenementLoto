"""
Action to test the project using CTest for a given preset.
"""

from ci import log
from ci.actions.base.action import BaseAction
from ci.utils.preset import get_build_dir
from ci.utils.run import run_command


class Test(BaseAction):
    """
    Action to test the project using CTest for a given preset.
    """

    def run(self, preset: str) -> int:
        """
        Test the project using CTest for the given preset.
        :param preset: The preset to check.
        :return: Exit code indicating success or failure.
        """
        log.info(f"Running tests with preset: {preset}")
        build_dir = get_build_dir(preset)
        if not build_dir.exists():
            log.error(f"Build directory does not exist: {build_dir}")
            return 1
        test = run_command(
            ["ctest", "--test-dir", str(build_dir), "--output-on-failure"]
        )
        if test != 0:
            log.error("Tests failed.")
            return test
        return 0
