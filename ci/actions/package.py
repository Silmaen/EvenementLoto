"""
Action to produce the ready-to-run archive of the application.
"""

from ci import log, root
from ci.actions.base.action import BaseAction
from ci.utils.preset import get_build_dir
from ci.utils.run import run_command, MODE_BY_COLOR, MODE_FOR_NINJA


class Package(BaseAction):
    """
    Action to build a release preset and pack it into a single ready-to-run archive.

    It does the whole job itself rather than reusing another configuration's build tree:
    the archive is CPack's output, and CPack needs the build tree it was configured in.
    """

    def run(self, preset: str) -> int:
        """
        Build the release preset and produce its archive.
        :param preset: The preset to package. Its release counterpart is used when it
                       declares one, since only a release build is worth shipping.
        :return: Exit code indicating success or failure.
        """
        from ci.utils.preset import get_preset_config

        config = get_preset_config(preset)
        if config.release_preset not in [None, ""]:
            preset = config.release_preset
        log.info(f"Packaging project with preset: {preset}")

        configure = run_command(
            ["cmake", "--preset", preset, "-S", str(root), "-G", "Ninja"],
            detection_mode=MODE_BY_COLOR,
        )
        if configure != 0:
            log.error("CMake configuration failed.")
            return configure

        build_dir = get_build_dir(preset)
        if not build_dir.exists():
            log.error(f"Build directory does not exist: {build_dir}")
            return 1

        build_result = run_command(
            ["cmake", "--build", str(build_dir)], detection_mode=MODE_FOR_NINJA
        )
        if build_result != 0:
            log.error("CMake build failed.")
            return build_result

        return self._pack(build_dir)

    @staticmethod
    def _pack(build_dir) -> int:
        """
        Run CPack in the build directory and normalise the archive name.
        :param build_dir: The configured build directory.
        :return: Exit code indicating success or failure.
        """
        from os import chdir, curdir

        cwd = curdir
        chdir(build_dir)
        pack_result = run_command(["cpack"], detection_mode=MODE_BY_COLOR)
        chdir(cwd)
        if pack_result != 0:
            log.error("CPack packaging failed.")
            return pack_result

        archives = sorted(build_dir.glob("EvenementLoto-*.zip"))
        # CPack's TGZ generator writes `.tar.gz`; the delivered name is `.tgz`, so the
        # two targets are told apart by their extension alone and neither needs the
        # platform spelled out in the file name.
        for archive in sorted(build_dir.glob("EvenementLoto-*.tar.gz")):
            renamed = archive.with_name(archive.name.removesuffix(".tar.gz") + ".tgz")
            archive.replace(renamed)
            archives.append(renamed)

        if not archives:
            log.error(f"CPack produced no archive in {build_dir}")
            return 1
        for archive in archives:
            log.info(f"Package: {archive.name} ({archive.stat().st_size} bytes)")
        return 0
