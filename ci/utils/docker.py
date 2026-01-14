"""
Module to manage docker images
"""

from json import load

from ci import root

docker_presets_path = root / "ci" / "dockerImages.json"


def determine_docker_image(preset: str) -> str:
    """
    Determine the docker image based on the preset.
    :param preset: The preset to check.
    :return: The docker image or "notfound".
    """
    import platform
    if platform.system() != "Linux":  # Docker images for CI are only used on Linux hosts
        return ""  # providing empty string prevent TeamCity from using docker runner

    with open(docker_presets_path) as js:
        data = load(js)
    result = "notfound"
    if preset in data["images"]:
        result = f"{data["images"][preset]}"
    if "namespace" in data:
        namespace = ""
        if "default" in data["namespace"]:
            namespace = data["namespace"]["default"]
        if preset in data["namespace"]:
            namespace = data["namespace"][preset]
        if namespace not in ["", "none"]:
            result = f"{namespace}/{result}"
    return result


def list_docker_presets() -> list[str]:
    """
    List all available docker presets.
    :return: List of presets.
    """
    with open(docker_presets_path) as js:
        data = load(js)
    return list(data["images"].keys())
