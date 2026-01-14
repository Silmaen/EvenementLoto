"""
Utility function for running application commands.
"""

from logging import INFO, WARNING, ERROR

from ci import log

# enum for mode of log level determination
MODE_BY_CONTENT = 0
MODE_BY_COLOR = 1
MODE_FOR_NINJA = 2

# variables to keep track of current and next log levels for color-based detection
current_level = INFO
next_level = INFO


def _determine_log_level(line: str, mode: int = MODE_BY_CONTENT) -> int:
    """
    Determine the appropriate log level based on the line content.

    :param line: The log line to analyze.
    :return: The logging level (DEBUG, INFO, WARNING, ERROR).
    """
    import re

    global current_level, next_level
    current_level = next_level
    if mode == MODE_BY_COLOR:
        if "\x1b[31m" in line:  # Red
            current_level = ERROR
            next_level = ERROR
        elif "\x1b[33m" in line:  # Yellow
            current_level = WARNING
            next_level = WARNING
        if "\x1b[0m" in line:  # Green
            next_level = INFO
        return current_level
    elif mode == MODE_FOR_NINJA:
        if re.match(r"^\[\d+/\d+]", line):
            return INFO
        else:
            return ERROR
    else:
        # old content-based detection (may trigger false positives)
        line_lower = line.lower()
        if re.search(r"\b0\s+(tests?|errors?)\s+(failed|error)", line_lower):
            return INFO
        if re.search(r"\b(error|failed|fatal|exception)\b", line_lower):
            return ERROR
        elif re.search(r"\b(warning|warn|deprecated)\b", line_lower):
            return WARNING
        else:
            return INFO


def _strip_ansi_codes(text: str) -> str:
    """
    Remove ANSI escape codes from the given text.

    :param text: The text containing ANSI codes.
    :return: The cleaned text without ANSI codes.
    """
    import re

    ansi_escape = re.compile(r"\x1B[@-_][0-?]*[ -/]*[@-~]")
    return ansi_escape.sub("", text)


def run_command(command: list[str] | str, detection_mode: int = MODE_BY_CONTENT) -> int:
    """
    Runs a command as a subprocess and logs its output in real-time.

    :param command: The command to run as a list of strings.
    :param detection_mode: Log Level detection mode.
    :return: The exit code of the command.
    """
    import subprocess
    from os import environ

    if isinstance(command, str):
        command = command.split()
    log.info(f"Running command: {' '.join(command)}")
    try:
        env = environ.copy()
        if detection_mode == MODE_BY_COLOR:
            env["CLICOLOR_FORCE"] = "1"
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
            env=env,
        )

        for line in process.stdout:
            line = line.rstrip("\n")
            if line:
                level = _determine_log_level(line, detection_mode)
                if detection_mode == MODE_BY_COLOR:
                    line = _strip_ansi_codes(line)
                log.log(level, line)

        for line in process.stderr:
            line = line.rstrip("\n")
            if line:
                level = max(_determine_log_level(line, detection_mode), WARNING)
                if detection_mode == MODE_BY_COLOR:
                    line = _strip_ansi_codes(line)
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
