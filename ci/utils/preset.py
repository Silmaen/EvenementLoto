"""
Gathering tools for presets
"""
from ci import root, Path, log


def get_build_dir(preset: str) -> Path:
    """
    Get the build directory for a given preset.

    :param preset: The preset to get the build directory for.
    :return: The Path to the build directory.
    """
    return root / "output" / "build" / preset


class PresetConfig:
    """
    Class representing a preset configuration.
    """

    def __init__(self, preset: str):
        self.preset = preset
        self.docker_image = f""
        self.release_preset = f""
        self.run_tests = True
        self.run_deploy = False

    def from_json(self, data: dict):
        """
        Load configuration from a JSON dictionary.

        :param data: The JSON data as a dictionary.
        """

        p_data = data.get("defaults", {})
        for key, value in data.get("presets", {}).get(self.preset, {}).items():
            p_data[key] = value
        if "docker_image" in p_data:
            self.docker_image = f'{p_data["docker_image"]}'
            if "docker_namespace" in p_data:
                self.docker_image = f"{p_data['docker_namespace']}/{self.docker_image}"
            if "docker_registry" in p_data:
                self.docker_image = f"{p_data['docker_registry']}/{self.docker_image}"
        if "release_preset" in p_data:
            self.release_preset = p_data["release_preset"]
        if "run_tests" in p_data:
            self.run_tests = p_data["run_tests"]
        if "run_deploy" in p_data:
            self.run_deploy = p_data["run_deploy"]


def get_preset_config(preset: str) -> PresetConfig:
    """
    Get the configuration for a given preset.

    :param preset: The preset to get the configuration file for.
    :return: The Configuration for the preset.
    """
    config_file = root / "ci" / "PresetsParameters.json"
    config = PresetConfig(preset)
    if not config_file.exists():
        log.warning(f"Preset configuration file does not exist: {config_file}")
        return config
    import json
    with open(config_file, 'r') as f:
        data = json.load(f)
    config.from_json(data)
    return config
