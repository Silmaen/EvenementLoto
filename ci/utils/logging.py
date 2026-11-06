"""
Utility functions for logging configuration and management.
"""

import logging


def setup_logging(level: int = logging.DEBUG) -> None:
    """
    Configures the logging system with Rich for development, plain lines in GitLab CI.

    :param level: The logging level to set (default is logging.INFO).
    """
    from os import environ

    # Rich wraps at 80 columns when it is not given a terminal, which would cut a
    # finding in two in the job log.
    if "GITLAB_CI" in environ:
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter("%(levelname)-8s %(message)s"))
    else:
        try:
            import rich.logging
        except ImportError:
            print(
                "WARNING: Rich python library not found. Falling back to standard logging."
            )
            rich = None
        if rich is not None:
            handler = rich.logging.RichHandler(
                markup=True, log_time_format="[%Y/%m/%d %X]"
            )
            handler.setFormatter(logging.Formatter("%(message)s"))
        else:
            handler = logging.StreamHandler()

    logging.basicConfig(level=level, handlers=[handler])
