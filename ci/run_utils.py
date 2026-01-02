"""
Utility functuion for running application commands.
"""

import logging

log = logging.getLogger(__name__)


def _determine_log_level(line: str) -> int:
    """
    Determine the appropriate log level based on line content.

    :param line: The log line to analyze.
    :return: The logging level (DEBUG, INFO, WARNING, ERROR).
    """
    import re

    line_lower = line.lower()
    if re.search(r"\b0\s+(tests?|errors?)\s+(failed|error)", line_lower):
        return logging.INFO
    if re.search(r"\b(error|failed|fatal|exception)\b", line_lower):
        return logging.ERROR
    elif re.search(r"\b(warning|warn|deprecated)\b", line_lower):
        return logging.WARNING
    else:
        return logging.INFO


def run_command(command: list[str] | str) -> int:
    """
    Runs a command as a subprocess and logs its output in real-time.

    :param command: The command to run as a list of strings.
    :return: The exit code of the command.
    """
    import subprocess

    if isinstance(command, str):
        command = command.split()
    log.info(f"Running command: {' '.join(command)}")
    try:
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )

        for line in process.stdout:
            line = line.rstrip("\n")
            if line:
                level = _determine_log_level(line)
                log.log(level, line)

        for line in process.stderr:
            line = line.rstrip("\n")
            if line:
                level = max(_determine_log_level(line), logging.WARNING)
                log.log(level, line)

        process.wait()
        return process.returncode
    except FileNotFoundError:
        log.error(
            f"Command not found: {command[0]}. Make sure it's installed and in PATH."
        )
        return 1
    except Exception as e:
        log.error(f"Error running command '{' '.join(command)}': {e}")
        return 1
