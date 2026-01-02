"""
Utility functions for logging configuration and management.
"""

import logging
import os

try:
    import rich.logging
except ImportError:
    rich = None


class TeamCityFormatter(logging.Formatter):
    """Formatter for TeamCity service messages."""

    def format(self, record: logging.LogRecord) -> str:
        message = record.getMessage()
        level = record.levelname

        if level == "ERROR":
            return f"##teamcity[message text='{self._escape(message)}' status='ERROR']"
        elif level == "WARNING":
            return (
                f"##teamcity[message text='{self._escape(message)}' status='WARNING']"
            )
        else:
            return f"##teamcity[message text='{self._escape(message)}']"

    @staticmethod
    def _escape(text: str) -> str:
        """Escape special characters for TeamCity."""
        replacements = {
            "|": "||",
            "'": "|'",
            "\n": "|n",
            "\r": "|r",
            "[": "|[",
            "]": "|]",
        }
        for char, escaped in replacements.items():
            text = text.replace(char, escaped)
        return text


def setup_logging(level: int = logging.INFO) -> None:
    """
    Configures the logging system with Rich for development or TeamCity formatter for CI.

    :param level: The logging level to set (default is logging.INFO).
    """
    if "TEAMCITY_VERSION" in os.environ:
        handler = logging.StreamHandler()
        handler.setFormatter(TeamCityFormatter())
    elif rich is not None:
        handler = rich.logging.RichHandler()
        handler.setFormatter(logging.Formatter("%(message)s"))
    else:
        handler = logging.StreamHandler()

    logging.basicConfig(level=level, handlers=[handler])
