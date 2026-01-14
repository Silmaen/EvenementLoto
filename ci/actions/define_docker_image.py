"""
Action to define the Docker image for a given preset in a CI environment.
"""

from ci import log
from ci.actions.base.action import BaseAction


class DefineDockerImage(BaseAction):
    """
    Action to determine and set the Docker image for a given preset.
    """

    def run(self, preset: str) -> int:
        """
        Determine the Docker image for the given preset and set it as a TeamCity parameter.
        :param preset: The preset to check.
        :return: Exit code indicating success or failure.
        """
        from ci.utils.docker import (
            determine_docker_image,
            list_docker_presets,
        )
        from ci.utils.teamcity import set_teamcity_parameter

        result = determine_docker_image(preset)
        if result == "notfound":
            log.error(f"Docker image not found for preset: {preset}")
            log.info("Available presets are:")

            for p in list_docker_presets():
                log.info(f" - {p}")
            return 1
        if result == "":
            log.info(f"No Docker image needed for preset '{preset}' on this platform.")

        log.info(f"Docker image for preset '{preset}': {result}")
        set_teamcity_parameter("docker_image", result)
        return 0
