import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get, rmdir

required_conan_version = ">=2.0"


class NfdConan(ConanFile):
    """nativefiledialog-extended is not available on ConanCenter, hence this local recipe.

    On Linux the backend links against the system GTK3 (or D-Bus with `portal`), found
    through pkg-config exactly as upstream does: the development packages are expected
    to be provided by the build image.
    """

    name = "nfd"
    description = "Native file dialogs for Windows, macOS and Linux"
    license = "Zlib"
    homepage = "https://github.com/btzy/nativefiledialog-extended"
    topics = ("dialog", "file", "gui", "native")
    package_type = "library"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "portal": [True, False],
        "wayland": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "portal": False,
        "wayland": False,
    }
    implements = ["auto_shared_fpic"]

    @property
    def _is_unix_desktop(self):
        return self.settings.os in ["Linux", "FreeBSD"]

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
        if not self._is_unix_desktop:
            del self.options.portal
            del self.options.wayland

    def layout(self):
        cmake_layout(self, src_folder="src")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["NFD_BUILD_TESTS"] = False
        tc.cache_variables["NFD_INSTALL"] = True
        if self._is_unix_desktop:
            tc.cache_variables["NFD_PORTAL"] = bool(self.options.portal)
            tc.cache_variables["NFD_WAYLAND"] = bool(self.options.wayland)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.libs = ["nfd"]
        if self.settings.os == "Windows":
            self.cpp_info.system_libs = ["ole32", "uuid", "shell32"]
        elif self.settings.os == "Macos":
            self.cpp_info.frameworks = ["AppKit", "UniformTypeIdentifiers"]
        elif self._is_unix_desktop:
            if self.options.portal:
                self.cpp_info.system_libs = ["dbus-1"]
            else:
                self.cpp_info.system_libs = ["gtk-3", "gdk-3", "gobject-2.0", "glib-2.0"]
            if self.options.wayland:
                self.cpp_info.system_libs.append("wayland-client")
