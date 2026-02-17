# CLAUDE.md - EvenementLoto

## Project Overview

EvenementLoto is a C++23 desktop application for managing "loto associatif" (French charity bingo) events. It handles event configuration, game rounds, number drawing, prize tracking, statistics, and full-screen display for players. Current version: **0.4.0**.

Author: Silmaen

## Architecture

### Source Layout

- `source/core/` - Core library (`EvenementLoto_lib`): game logic, serialization, settings, logging, RNG, math utilities, statistics
- `source/gui/` - GUI library (`EvenementLoto_ui`): ImGui/Vulkan-based interface (views, popups, actions, event handling, theming)
  - `source/gui/event/` - Event system (keyboard/mouse events, key codes, application events)
  - `source/gui/views/` - View components (MainView, DisplayView, MenuBar, ToolBar, StatusBar, ConfigPopups, HelpPopups)
  - `source/gui/actions/` - Action handlers (FileActions, GameActions, SettingsActions, HelpActions)
  - `source/gui/vulkan/` - Vulkan rendering (VulkanContext, TextureLibrary, vkData)
  - `source/gui/utils/` - UI utilities (FileDialog, Convert, Rendering helpers)
- `source/resources/` - Icon resources (dark icons copied at build time)
- `source/main.cpp` - Entry point (supports ImGui UI; legacy Qt path still exists behind `USE_QT` ifdef)
- `test/lib_test/` - Unit tests for core library (Google Test, 10 test files)
- `test/gui_test/` - Unit tests for GUI library (Google Test, 6 test files)
- `ci/` - Python-based CI scripts (build, test, coverage, deploy, documentation)
- `cmake/` - CMake modules (14 files: BaseConfig, Vulkan, Sanitizers, Coverage, Depmanager, Poetry, Python, Environment, UtilityFunctions, DocumentationConfig)
- `document/` - User documentation (in French)
- `data/` - Runtime data files

### Key Domain Classes (namespace `evl::core`)

- `Event` - Top-level event: contains organizer info, rules, logos, game rounds, status state machine
- `GameRound` - A single game round: type (OneQuine, TwoQuines, FullCard, combinations, Enfant, Inverse, Pause), sub-rounds, draws
- `SubGameRound` - A sub-game within a round: type, draws, prize info, winner
- `Serializable` - Abstract base for binary stream, JSON (jsoncpp), and YAML (yaml-cpp) serialization
- `Settings` - Application settings (key-value store)
- `Statistics` - Draw statistics tracking
- `RandomNumberGenerator` - Number drawing engine (uses `std::mt19937` + `std::uniform_int_distribution`)
- `Log` - Logging wrapper around spdlog, with `LogBuffer` for in-app log display

### Math Utilities (namespace `evl::math`)

- `vectors.h` - Generic `Vector<BaseType, Dim>` template (fixed-size array backed)
- Type aliases: `vec2`, `vec2i`, etc.

### GUI (namespace `evl::gui`)

- `Application` - Singleton application class, manages views/popups/actions, Vulkan rendering, autosave (`rescue.lev` every 10s during active gameplay)
- `MainWindow` - GLFW window management with Vulkan surface
- `Theme` - Theme configuration for the UI (colors, rounding, spacing; persisted in settings)
- `event/` - Event system: `Event` base, `KeyEvent`, `MouseEvent`, `AppEvent`, `KeyCode`, `MouseCode`
- `views/` - View, MainView, DisplayView, MenuBar, ToolBar, StatusBar, Popups, ConfigPopups, HelpPopups
- `actions/` - Action base, FileActions, GameActions, SettingsActions, HelpActions
- `vulkan/` - VulkanContext (Vulkan instance/device/swapchain management), TextureLibrary (SVG/PNG/JPG loading), vkData
- `utils/` - FileDialog (open/save/folder dialogs), Convert (ImGui/core vector conversions), Rendering (action buttons, text auto-fit)

## Build System

- **CMake 3.24+** with CMake Presets (`CMakePresets.json` includes Linux, MinGW, and CI presets)
- **C++ Standard**: C++23 (`CMAKE_CXX_STANDARD 23`)
- **Supported compilers**: GCC 14+, Clang 18+
- **Supported platforms**: Linux, Windows (MinGW)
- **Dependency management**: [DepManager](https://github.com/Silmaen/DepManager) (`depmanager.yml`)
- **Python tooling**: Poetry (`pyproject.toml`), Python 3.12+
- **Code formatting**: clang-format (`.clang-format`), cmake-format (`.cmake-format.json`)

### External Dependencies (via DepManager)

glfw 3.4.0, googletest 1.17.0, imgui 1.92.5-docking, jsoncpp 1.9.6, magic_enum 0.9.7, nanosvg 1.0.0, nfd 1.2.1, spdlog 1.17.0, stb_image 2.28, vulkan_sdk 1.4.328, yaml-cpp 0.8.0

### Python Dependencies (via Poetry)

- depmanager ^0.5.1, black ^25.12.0, gcovr ^8.6, rich ^14.2.0

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
- `EVL_ENABLE_ADDRESS_SANITIZER`, `EVL_ENABLE_THREAD_SANITIZER`, `EVL_ENABLE_LEAK_SANITIZER`, `EVL_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER` - Sanitizers (Clang only, mutually exclusive)

### Build Presets

```bash
# Linux presets
cmake --preset linux-gcc-release
cmake --preset linux-gcc-debug        # with coverage
cmake --preset linux-clang-release
cmake --preset linux-clang-debug       # with coverage

# Windows/MinGW presets
cmake --preset windows-gcc-release
cmake --preset windows-gcc-debug
cmake --preset windows-clang-release
cmake --preset windows-clang-debug

# CI presets (sanitizers, clang-tidy)
cmake --preset linux-clang-tidy
cmake --preset linux-sanitizer-address
cmake --preset linux-sanitizer-thread
cmake --preset linux-sanitizer-undefined-behavior
cmake --preset linux-sanitizer-leak
```

### Build Commands

```bash
# Configure + Build
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug

# Run tests
ctest --preset linux-clang-debug
```

## CI System

Python-based CI scripts in `ci/` (21 files), driven by `ci_action.py`:

- `ci/actions/build.py` - CMake configure + Ninja build
- `ci/actions/test.py` - Test execution
- `ci/actions/coverage.py` - gcovr coverage reports
- `ci/actions/deploy.py` - CPack packaging
- `ci/actions/documentation.py` - Doxygen documentation generation
- `ci/actions/clean.py` - Build directory cleanup
- `ci/utils/run.py` - Command execution with real-time output
- `ci/utils/preset.py` - CMake preset parsing (from `ci/PresetsParameters.json`)
- `ci/utils/teamcity.py` - TeamCity CI integration

## Coding Conventions

### Style

- **Indentation**: Tabs (width 4), `UseTab: ForContinuationAndIndentation` in `.clang-format`
- **Braces**: Attach style (same line as control structure)
- **Column limit**: 120 characters
- **Pointer alignment**: Left (`int* ptr`)
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
- **Global constants/maps**: `g_` prefix (e.g., `g_typeConvert`, `g_statusConvert`)

### C++ Practices

- Prefer `const` for immutable variables
- Initialize variables at declaration
- Use early returns to reduce nesting
- Use `[[nodiscard]]` on getters and query functions
- Use `#pragma once` for header guards
- Namespaces: `evl`, `evl::core`, `evl::math`, `evl::logs`, `evl::gui`, `evl::gui::views`, `evl::gui::actions`, `evl::gui::event`, `evl::gui::vulkan`, `evl::gui::utils`
- Precompiled header: `pch.h` included in `.cpp` files

### Logging

- `log_trace(...)`, `log_debug(...)`, `log_info(...)`, `log_warn(...)`, `log_error(...)`, `log_critical(...)`
- Macros in `Log.h` using `std::format` (C++23) for formatting
- Backend: spdlog with console + file sinks
- In-app log buffer: `evl::logs::LogBuffer` singleton (thread-safe, max 1000 entries)

### Serialization

All domain objects inherit from `Serializable` and implement:
- `read()`/`write()` - Binary stream I/O (versioned with `getSaveVersion()`)
- `toJson()`/`fromJson()` - JSON via jsoncpp
- `toYaml()`/`fromYaml()` - YAML via yaml-cpp

## Testing

- Framework: Google Test
- Test files follow `test_<ClassName>.cpp` naming
- Tests are in `test/lib_test/` (core, 10 files) and `test/gui_test/` (GUI, 6 files)
- Coverage via gcovr (configured in `gcovr.cfg`)
- Test helper header: `test/TestMainHelper.h`
- Sanitizer suppressions: `lsan_suppressions.txt` (suppresses known libdbus leaks for Address/Leak sanitizer presets)
