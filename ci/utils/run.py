"""
Utility function for running application commands.
"""

from logging import INFO, WARNING, ERROR

from ci import log

# enum for mode of log level determination
MODE_BY_CONTENT = 0
MODE_BY_COLOR = 1
MODE_FOR_NINJA = 2

# list of regex patterns to exclude from ninja error detection
ninja_error_exclusions = [
    r"^CPack:.*"
]


class LevelDetector:
    """
    Determine the log level of each line of a command output.

    One instance per stream: colour based detection carries a state from one line to the
    next, and the two streams are read concurrently.
    """

    def __init__(self, mode: int = MODE_BY_CONTENT):
        self._mode = mode
        self._current = INFO
        self._next = INFO

    def level(self, line: str) -> int:
        """
        Determine the appropriate log level based on the line content.

        :param line: The log line to analyze.
        :return: The logging level (DEBUG, INFO, WARNING, ERROR).
        """
        import re

        if self._mode == MODE_BY_COLOR:
            self._current = self._next
            if "\x1b[31m" in line:  # Red
                self._current = ERROR
                self._next = ERROR
            elif "\x1b[33m" in line:  # Yellow
                self._current = WARNING
                self._next = WARNING
            if "\x1b[0m" in line:  # Green
                self._next = INFO
            return self._current
        if self._mode == MODE_FOR_NINJA:
            if re.match(r"^\[\d+/\d+]", line):
                return INFO
            for pattern in ninja_error_exclusions:
                if re.search(pattern, line):
                    return INFO
            return ERROR
        # old content-based detection (may trigger false positives)
        line_lower = line.lower()
        if re.search(r"\b0\s+(tests?|errors?)\s+(failed|error)", line_lower):
            return INFO
        if re.search(r"\b(error|failed|fatal|exception)\b", line_lower):
            return ERROR
        if re.search(r"\b(warning|warn|deprecated)\b", line_lower):
            return WARNING
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


def _log_stream(stream, is_stderr: bool, detection_mode: int) -> None:
    """
    Forward one output stream of a subprocess to the log, line by line.

    :param stream: The stream to read until the process closes it.
    :param is_stderr: Whether the stream is the error one, never logged below warning.
    :param detection_mode: Log level detection mode.
    """
    detector = LevelDetector(detection_mode)
    for line in stream:
        line = line.rstrip("\n")
        if not line:
            continue
        level = detector.level(line)
        if is_stderr:
            level = max(level, WARNING)
        if detection_mode == MODE_BY_COLOR:
            line = _strip_ansi_codes(line)
        log.log(level, line)


def run_command(command: list[str] | str, detection_mode: int = MODE_BY_CONTENT) -> int:
    """
    Runs a potentially long command as a subprocess and logs its output in real-time.

    Both streams are drained concurrently, by one thread each. Reading them one after
    the other would deadlock: once a pipe buffer is full the child blocks on write, and
    a build that writes mostly on stderr — `conan install` does — freezes for good.

    :param command: The command to run as a list of strings.
    :param detection_mode: Log Level detection mode.
    :return: The exit code of the command.
    """
    import subprocess
    from os import environ
    from threading import Thread

    if isinstance(command, str):
        command = command.split()
    log.info(f"Running command: {' '.join(command)}")
    try:
        env = environ.copy()
        if detection_mode == MODE_BY_COLOR:
            env["CLICOLOR_FORCE"] = "1"
        env["PYTHONUNBUFFERED"] = "1"
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
            env=env,
        )
        readers = [
            Thread(target=_log_stream, args=(process.stdout, False, detection_mode), daemon=True),
            Thread(target=_log_stream, args=(process.stderr, True, detection_mode), daemon=True),
        ]
        for reader in readers:
            reader.start()
        returncode = process.wait()
        for reader in readers:
            reader.join()
        return returncode
    except FileNotFoundError:
        log.error(
            f"Command not found: {command[0]}. Make sure it's installed and in PATH."
        )
        return 1
    except Exception as e:
        log.error(f"Error running command '{' '.join(command)}': {e}")
        return 1


def run_command_capture_output(command: list[str] | str) -> tuple[int, str]:
    """
    Runs a command as a subprocess and captures its output.

    :param command: The command to run as a list of strings.
    :return: A tuple containing the exit code, stdout, and stderr.
    """
    import subprocess

    if isinstance(command, str):
        command = command.split()
    try:
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        stdout, stderr = process.communicate()
        return process.returncode, stdout
    except FileNotFoundError:
        log.error(
            f"Command not found: {command[0]}. Make sure it's installed and in PATH."
        )
        return 1, ""
    except Exception as e:
        log.error(f"Error running command '{' '.join(command)}': {e}")
        return 1, ""
