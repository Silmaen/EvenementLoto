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

    def with_options(self, options: list[str]) -> "BaseAction":
        """
        Take the command-line options the entry point did not recognise.

        Actions that accept options override this. The default refuses them: an option
        handed to an action that would ignore it must be an error, not a silence.

        :param options: The leftover command-line arguments.
        :return: The action itself, configured.
        """
        if options:
            raise ValueError(
                f"{type(self).__name__} takes no option: {' '.join(options)}"
            )
        return self

    def __call__(self, preset: str) -> int:
        """Allow the action to be called directly."""
        return self.run(preset)
