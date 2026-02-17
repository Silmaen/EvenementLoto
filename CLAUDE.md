# CLAUDE.md - EvenementLoto

## Project Overview

EvenementLoto is a C++23 desktop application for managing "loto associatif" (French charity bingo) events. It handles event configuration, game rounds, number drawing, prize tracking, statistics, and full-screen display for players. Current version: **0.4.0**.

Author: Silmaen

## Architecture

### Source Layout

- `source/core/` - Core library (`EvenementLoto_lib`): game logic, serialization, settings, logging, RNG, statistics
- `source/gui/` - GUI library (`EvenementLoto_ui`): ImGui/Vulkan-based interface (views, popups, actions, event handling, theming)
- `source/resources/` - Icon resources (dark icons copied at build time)
- `source/main.cpp` - Entry point (supports ImGui UI; legacy Qt path still exists behind `USE_QT` ifdef)
- `test/lib_test/` - Unit tests for the core library (Google Test)
- `test/gui_test/` - Unit tests for the GUI library (Google Test)
- `ci/` - Python-based CI scripts (build, test, coverage, deploy, documentation)
- `cmake/` - CMake modules (BaseConfig, Vulkan, Sanitizers, Coverage, Depmanager, Poetry, Python)
- `document/` - User documentation (in French)
- `data/` - Runtime data files

### Key Domain Classes (namespace `evl::core`)

- `Event` - Top-level event: contains organizer info, rules, logos, game rounds, status state machine
- `GameRound` - A single game round: type (OneQuine, TwoQuines, FullCard, combinations, Enfant, Inverse, Pause), sub-rounds, draws
- `SubGameRound` - A sub-game within a round: type, draws, prize info, winner
- `Serializable` - Abstract base for binary stream, JSON (jsoncpp), and YAML (yaml-cpp) serialization
- `Settings` - Application settings (key-value store)
- `Statistics` - Draw statistics tracking
- `RandomNumberGenerator` - Number drawing engine
- `Log` - Logging wrapper around spdlog

### GUI (namespace `evl::gui`)

- `Application` - Singleton application class, manages views/popups/actions, Vulkan rendering
- `MainWindow` - GLFW window management
- `views/` - MainView, DisplayView (player screen), MenuBar, ToolBar, StatusBar
- `actions/` - FileActions, GameActions, SettingsActions, HelpActions
- `vulkan/` - VulkanContext, TextureLibrary

## Build System

- **CMake 3.24+** with CMake Presets (`CMakePresets.json` includes Linux, MinGW, and CI presets)
- **C++ Standard**: C++23 (`CMAKE_CXX_STANDARD 23`)
- **Supported compilers**: GCC 14+, Clang 18+
- **Supported platforms**: Linux, Windows (MinGW)
- **Dependency management**: [DepManager](https://github.com/Silmaen/DepManager) (`depmanager.yml`)
- **Python tooling**: Poetry (`pyproject.toml`), Python 3.12+

### External Dependencies (via DepManager)

glfw 3.4.0, googletest 1.17.0, imgui 1.92.5-docking, jsoncpp 1.9.6, magic_enum 0.9.7, nanosvg 1.0.0, nfd 1.2.1, spdlog 1.17.0, stb_image 2.28, vulkan_sdk 1.4.328, yaml-cpp 0.8.0

### Build Targets

- `EvenementLoto` - Main executable
- `EvenementLoto_lib` - Core library
- `EvenementLoto_ui` - GUI library
- `EvenementLoto_resource` - Resource copy target
- `All_Tests` - Runs all unit tests
- `evl_lib_test_unit_test` - Core library unit tests
- `evl_gui_test_unit_test` - GUI unit tests

### Build Options

- `EVL_TESTING` - Build tests (default: ON)
- `EVL_PACKAGING` - Enable CPack packaging (default: ON)
- `EVL_ENABLE_COVERAGE` - Code coverage with gcovr
- `EVL_ENABLE_CLANG_TIDY` - Clang-tidy analysis
- `EVL_ENABLE_ADDRESS_SANITIZER`, `EVL_ENABLE_THREAD_SANITIZER`, `EVL_ENABLE_MEMORY_SANITIZER`, `EVL_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER` - Sanitizers

### Build Commands

```bash
# Configure (using presets)
cmake --preset linux-clang-debug
cmake --preset linux-clang-release

# Build
cmake --build --preset linux-clang-debug
cmake --build --preset linux-clang-release

# Run tests
ctest --preset linux-clang-debug
```

## Coding Conventions

### Style

- **Indentation**: Tabs (width 4), as per `.clang-format`
- **Braces**: Attach style (same line as control structure)
- **Column limit**: 120 characters
- **Formatting tool**: clang-format (`.clang-format` at project root)
- **CMake formatting**: cmake-format (`.cmake-format.json`)
- **Comments**: In English
- **Documentation**: Doxygen style (`@brief`, `@param`, `@return`)

### Naming

- **Member variables**: `m_` prefix (e.g., `m_status`, `m_gameRounds`)
- **Input parameters**: `i` prefix (e.g., `iName`, `iType`)
- **Output parameters**: `o` prefix (e.g., `oBs`)
- **Input/output parameters**: `io` prefix (e.g., `ioEvent`)
- **Local variables**: camelCase
- **Return types**: Use trailing return type syntax (`auto func() -> ReturnType`)
- **Enums**: `enum struct` (scoped enums)

### C++ Practices

- Prefer `const` for immutable variables
- Initialize variables at declaration
- Use early returns to reduce nesting
- Use `[[nodiscard]]` on getters and query functions
- Use `#pragma once` for header guards
- Namespaces: `evl::core` for core logic, `evl::gui` for GUI, `evl::gui::views`, `evl::gui::actions`, `evl::gui::event`, `evl::gui::vulkan`

### Logging

- `log_error(...)` for errors
- `log_warn(...)` for warnings
- `log_info(...)` for informational messages
- Based on spdlog with fmt-style formatting

### Serialization

All domain objects inherit from `Serializable` and implement:
- `read()`/`write()` - Binary stream I/O
- `toJson()`/`fromJson()` - JSON via jsoncpp
- `toYaml()`/`fromYaml()` - YAML via yaml-cpp

## Testing

- Framework: Google Test
- Test files follow `test_<ClassName>.cpp` naming
- Tests are in `test/lib_test/` (core) and `test/gui_test/` (GUI)
- Coverage via gcovr (configured in `gcovr.cfg`)
