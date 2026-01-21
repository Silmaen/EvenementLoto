"""
Action to run Documentation generation using doxygen.
"""

from ci import log
from ci.actions.base.action import BaseAction
from ci.utils.preset import get_preset_config, get_build_dir
from ci.utils.run import run_command


class Documentation(BaseAction):
    """
    Action to run Documentation generation using doxygen.
    """

    def run(self, preset: str) -> int:
        """
        Executes the documentation generation process.
        :param preset: The preset to use for the action.
        :return: Exit code indicating success or failure.
        """
        log.info("Starting documentation generation using Doxygen...")
        try:
            config = get_preset_config(preset)
            if config.release_preset not in [None, ""]:
                preset = config.release_preset
            exit_code = run_command(["cmake", "--build", f"{get_build_dir(preset)}", "--target", "documentation"])
            if exit_code != 0:
                log.error("Documentation generation failed.")
                return exit_code
            log.info("Documentation generated successfully.")
            return 0
        except Exception as e:
            log.error(f"Documentation generation failed: {e}")
            return 1
