"""
Module to manage docker images
"""

from json import load
from pathlib import Path

here = Path(__file__).resolve().parent


def determine_docker_image(preset: str) -> str:
    """
    Determine the docker image based on the preset.
    :param preset: The preset to check.
    :return: The docker image or "notfound".
    """
    with open(here / "dockerImages.json") as js:
        data = load(js)
    result = "notfound"
    if preset in data["images"]:
        result = data["images"][preset]
    return result


def list_docker_presets() -> list[str]:
    """
    List all available docker presets.
    :return: List of presets.
    """
    with open(here / "dockerImages.json") as js:
        data = load(js)
    return list(data["images"].keys())
