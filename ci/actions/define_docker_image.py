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
        )
        from ci.utils.teamcity import set_teamcity_parameter

        result = determine_docker_image(preset)
        if result == "":
            log.info(f"Docker image not found for preset: {preset}, assuming run on native host.")
            return 0
        log.info(f"Docker image for preset '{preset}': {result}")
        set_teamcity_parameter("docker_image", result)
        docker_parameters = "-u %env.BUILDER_UID%:%env.BUILDER_GID%"
        docker_parameters += " -v %teamcity.agent.home.dir%/user/cache_dir:/tmp/cache_dir"
        docker_parameters += " --network host"
        docker_parameters += " -v %teamcity.agent.home.dir%/user:/home/user"
        docker_parameters += " -e HOME=/home/user"
        docker_parameters += " --cap-add=SYS_PTRACE --security-opt seccomp=unconfined"

        set_teamcity_parameter("docker_parameters", docker_parameters)
        return 0
