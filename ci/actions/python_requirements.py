"""
Action to install Python requirements from a given preset.
"""

from ci import log
from ci.actions.base.action import BaseAction
from ci.utils.python import install_python_requirements


class PythonRequirements(BaseAction):
    """
    Action to install Python requirements from a given preset.
    """

    def run(self, preset: str) -> int:
        log.info(f"Installing Python requirements for preset: {preset}")
        install_python_requirements()
        return 0
