"""
Action to define the Docker image for a given preset in a CI environment.
"""

from ci import log, root
from ci.actions.base.action import BaseAction

config_json_path = root / "ci" / "PresetsParameters.json"


class Variables:
    """
    Container for variable names.
    """

    def __init__(self):
        self.registry = ""  # docker registry
        self.namespace = ""  # docker namespace
        self.docker_parameters = ""  # docker parameters
        self.docker_image = ""  # empty means no docker
        self.run_tests = True
        self.release_preset = ""
        self.run_deploy = True  # deploy only if release

    def __repr__(self):
        return (f"registry =        {self.registry}\n"
                f"namespace =       {self.namespace}\n"
                f"docker_parameters={self.docker_parameters}\n"
                f"docker_image =    {self.docker_image}\n"
                f"run_tests =       {self.run_tests}\n"
                f"release_preset =  {self.release_preset}\n"
                f"run_deploy =      {self.run_deploy}")

    def from_json(self, json_data: dict):
        """
        Load variables from a JSON dictionary.
        :param json_data: The JSON data as a dictionary.
        """
        if isinstance(json_data, dict):
            for key, value in json_data.items():
                if hasattr(self, key):
                    setattr(self, key, value)
        else:
            log.warning("JSON data is not a dictionary.")

    def export_teamcity_parameters(self):
        """
        Export variables as TeamCity parameters.
        """
        from ci.utils.teamcity import set_teamcity_parameter

        set_teamcity_parameter("run_tests", str(self.run_tests).lower())
        if self.release_preset not in [None, ""]:
            set_teamcity_parameter("release_preset", self.release_preset)
        set_teamcity_parameter("run_deploy", str(self.run_deploy).lower())


class DefineVariables(BaseAction):
    """
    Action to define various variables for a given preset.
    """

    def run(self, preset: str) -> int:
        """
        Define variables for the given preset and set them as TeamCity parameters.
        :param preset: The preset to check.
        :return: Exit code indicating success or failure.
        """
        log.info(f"Defining variables for preset '{preset}'")

        from ci.utils.preset import get_preset_config
        from ci.utils.teamcity import set_teamcity_parameter
        preset_config = get_preset_config(preset)
        if preset_config.release_preset not in [None, ""]:
            set_teamcity_parameter("release_preset", preset_config.release_preset)
        if preset_config.run_tests is not None:
            set_teamcity_parameter("run_tests", str(preset_config.run_tests).lower())
        if preset_config.run_deploy is not None:
            set_teamcity_parameter("run_deploy", str(preset_config.run_deploy).lower())
        return 0
