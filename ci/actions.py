"""
Gathering all action implementations for easy access.
"""

import logging
from pathlib import Path

from .run_utils import run_command

log = logging.getLogger(__name__)
here = Path(__file__).resolve().parent
root = here.parent


def define_docker_image(preset: str) -> int:
    """
    Determine the docker image based on the preset and set it as a TeamCity parameter.

    :param preset: The preset to check.
    :return: Exit code indicating success or failure.
    """
    from .docker_image import determine_docker_image
    from .teamcity_utils import set_teamcity_parameter

    result = determine_docker_image(preset)
    if result == "notfound":
        log.error(f"Docker image not found for preset: {preset}")
        log.info("Available presets are:")
        from .docker_image import list_docker_presets

        for p in list_docker_presets():
            log.info(f" - {p}")
        return 1

    log.info(f"Docker image for preset '{preset}': {result}")
    set_teamcity_parameter("docker_image", result)
    return 0


def build_project(preset: str) -> int:
    """
    Placeholder function to build the project.

    :param preset: The preset to use for building.
    :return: Exit code indicating success or failure.
    """
    log.info(f"Building project with preset: {preset}")
    # run cmake configure, capture output in real-time, and print it using logging
    configure = run_command(["cmake", "--preset", preset, "-S", str(root)])
    if configure != 0:
        log.error("CMake configuration failed.")
        return configure
    build_dir = root / "output" / "build" / preset
    if not build_dir.exists():
        log.error(f"Build directory does not exist: {build_dir}")
        return 1
    build = run_command(["cmake", "--build", str(build_dir)])
    if build != 0:
        log.error("CMake build failed.")
        return build
    return 0


def run_tests(preset: str) -> int:
    """
    Placeholder function to run tests.

    :param preset: The preset to use for testing.
    :return: Exit code indicating success or failure.
    """
    log.info(f"Running tests with preset: {preset}")
    build_dir = root / "output" / "build" / preset
    if not build_dir.exists():
        log.error(f"Build directory does not exist: {build_dir}")
        return 1
    test = run_command(["ctest", "--test-dir", str(build_dir), "--output-on-failure"])
    if test != 0:
        log.error("Tests failed.")
        return test
    return 0


def deploy(preset: str) -> int:
    """
    Placeholder function to deploy the project.

    :param preset: The preset to use for deployment.
    :return: Exit code indicating success or failure.
    """
    log.info(f"Deploying project with preset: {preset}")
    build_dir = root / "output" / "build" / preset
    if not build_dir.exists():
        log.error(f"Build directory does not exist: {build_dir}")
        return 1
    pack = run_command(["cpack", "--config", str(build_dir / "CPackConfig.cmake")])
    if pack != 0:
        log.error("Deployment (packaging) failed.")
        return pack
    return 0


def clean(preset: str) -> int:
    """
    Placeholder function to clean the build artifacts.

    :param preset: The preset to use for cleaning.
    :return: Exit code indicating success or failure.
    """
    log.info(f"Cleaning build artifacts with preset: {preset}")
    build_dir = root / "output"
    if build_dir.exists():
        import shutil

        shutil.rmtree(build_dir, ignore_errors=True)
        log.info(f"Removed build directory: {build_dir}")
    else:
        log.info(f"No build directory to remove: {build_dir}")
    return 0
