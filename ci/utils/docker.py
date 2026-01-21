"""
Module to manage docker images
"""


def determine_docker_image(preset: str) -> str:
    """
    Determine the docker image based on the preset.
    :param preset: The preset to check.
    :return: The docker image or "notfound".
    """
    import platform
    from ci.utils.preset import get_preset_config

    if platform.system() != "Linux":  # Docker images for CI are only used on Linux hosts
        return ""  # providing empty string prevent TeamCity from using docker runner
    config = get_preset_config(preset)
    return config.docker_image
