import os

from conan import ConanFile
from conan.tools.files import copy


class EvenementLotoConan(ConanFile):
    """Third parties of EvenementLoto.

    CMake drives this file: it is never invoked by hand. `cmake/conan_provider.cmake`
    runs `conan install` on the first find_package(), and `cmake/Conan.cmake` pushes the
    profile, the compiler and its version.
    """

    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps"

    # Everything static but the Vulkan loader, which is a shared library by nature.
    #
    # The project builds a single executable out of static libraries, so nothing is
    # gained by shipping DLLs, and two concrete problems disappear:
    #   - a shared imgui marks IMGUI_API as dllimport for consumers while the backends
    #     we compile ourselves export nothing, so every ImGui_Impl* call failed to link
    #     on MinGW with an undefined `__imp_` symbol;
    #   - a shared glfw left no DLL beside the test executables on Windows, which then
    #     died with STATUS_DLL_NOT_FOUND.
    # Static also means a single copy of the ImGui globals, which is what we want.
    default_options = {
        "imgui/*:shared": False,
        "glfw/*:shared": False,
        "spdlog/*:shared": False,
        "spdlog/*:use_std_fmt": True,
        "jsoncpp/*:shared": False,
        "yaml-cpp/*:shared": False,
        "gtest/*:shared": False,
        # Wayland and X11 are both supported: glfw picks the platform at runtime, and
        # the loader carries every WSI backend.
        "glfw/*:with_x11": True,
        "glfw/*:with_wayland": True,
        "vulkan-loader/*:with_wsi_xlib": True,
        "vulkan-loader/*:with_wsi_xcb": True,
        "vulkan-loader/*:with_wsi_wayland": True,
    }

    def requirements(self):
        self.requires("jsoncpp/1.9.6")
        self.requires("yaml-cpp/0.8.0")
        self.requires("spdlog/1.17.0")
        self.requires("magic_enum/0.9.7")
        self.requires("imgui/1.92.9b-docking")
        self.requires("glfw/3.4")
        self.requires("stb/cci.20240531")
        self.requires("nanosvg/cci.20231025")
        self.requires("vulkan-headers/1.4.350.0")
        self.requires("vulkan-loader/1.4.350.0")

    def build_requirements(self):
        self.test_requires("gtest/1.17.0")

    def generate(self):
        # ImGui ships its backends and its std::string helper as sources only. Expose the
        # backends under a "backends" folder so the existing
        # #include <backends/imgui_impl_*.h> keep working.
        imgui = self.dependencies["imgui"].package_folder
        bindings = os.path.join(self.generators_folder, "imgui_bindings")
        copy(self, "imgui_impl_glfw.*", os.path.join(imgui, "res", "bindings"),
             os.path.join(bindings, "backends"))
        copy(self, "imgui_impl_vulkan.*", os.path.join(imgui, "res", "bindings"),
             os.path.join(bindings, "backends"))
        copy(self, "imgui_stdlib.*", os.path.join(imgui, "res", "misc", "cpp"), bindings)
