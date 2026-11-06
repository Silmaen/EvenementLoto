"""
Action to build and check a preset the way the CI does.
"""

from ci import log
from ci.actions.base.action import BaseAction


class BuildAndTest(BaseAction):
    """
    Action chaining the build, the tests, the coverage and the documentation of a preset,
    then the same for its release counterpart.

    What runs is read from `ci/PresetsParameters.json` and nowhere else: the pipeline
    only says which presets exist and where they run.
    """

    def run(self, preset: str) -> int:
        """
        Run every step the preset declares, stopping at the first failure.
        :param preset: The preset to build.
        :return: Exit code indicating success or failure.
        """
        from ci.actions.build import Build
        from ci.actions.clean import Clean
        from ci.actions.coverage import Coverage
        from ci.actions.documentation import Documentation
        from ci.actions.test import Test
        from ci.utils.preset import get_preset_config

        config = get_preset_config(preset)
        steps = [(Clean, preset), (Build, preset)]
        if config.run_tests:
            steps.append((Test, preset))
            if config.run_coverage:
                steps.append((Coverage, preset))
        if config.release_preset not in [None, ""]:
            steps += [(Clean, config.release_preset), (Build, config.release_preset)]
            if config.run_tests:
                steps.append((Test, config.release_preset))
        # Built from the release tree when there is one, see Documentation.
        if config.run_documentation:
            steps.append((Documentation, preset))

        for action, target in steps:
            log.info(f"{action.__name__} {target}")
            result = action()(target)
            if result != 0:
                return result
        return 0
