from abc import ABC, abstractmethod


class BaseAction(ABC):
    """Abstract base class for all CI actions."""

    @abstractmethod
    def run(self, preset: str) -> int:
        """
        Execute the action.
        :param preset: The preset to use for the action.
        :return: Exit code indicating success or failure.
        """
        pass

    def __call__(self, preset: str) -> int:
        """Allow the action to be called directly."""
        return self.run(preset)
