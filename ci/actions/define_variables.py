"""
Action to define the Docker image for a given preset in a CI environment.
"""

from ci import log
from ci.actions.base.action import BaseAction


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
        if preset_config.run_coverage is not None:
            set_teamcity_parameter(
                "run_coverage", str(preset_config.run_coverage).lower()
            )
        if preset_config.run_documentation is not None:
            set_teamcity_parameter(
                "run_documentation", str(preset_config.run_documentation).lower()
            )
        artifact_path = """+:output/build/%cmake_preset%/bin => BuildArtefact.zip!bin/debug/
+:output/build/%cmake_preset%/lib => BuildArtefact.zip!lib/debug/
+:output/build/%cmake_preset%/test/*.xml => BuildArtefact.zip!test/debug/
+:output/build/%cmake_preset%/Coverage => Coverage.zip"""
        if preset_config.release_preset not in [None, ""]:
            artifact_path += """+:output/build/%release_preset%/bin => BuildArtefact.zip!bin/release/
+:output/build/%release_preset%/lib => BuildArtefact.zip!lib/release/
+:output/build/%release_preset%/test/*.xml => BuildArtefact.zip!test/release/
+:output/build/%release_preset%/Documentation/html => Documentation.zip
+:output/build/%release_preset%/*.zip
+:output/build/%release_preset%/*.tar.gz"""
        log.info(f"Setting artifact path:\n{artifact_path}")
        set_teamcity_parameter("artifact_path", artifact_path)

        return 0
