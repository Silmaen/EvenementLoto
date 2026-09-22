# CLAUDE.md - EvenementLoto

## Project Overview

EvenementLoto is a C++23 desktop application for managing "loto associatif" (French charity bingo) events. It handles event configuration, game rounds, number drawing, prize tracking, statistics, and full-screen display for players. Current version: **0.4.1**.

Author: Silmaen

## Architecture

### Source Layout

- `source/core/` - Core library (`EvenementLoto_lib`): game logic, serialization, settings, logging, RNG, math utilities, statistics
- `source/gui/` - GUI library (`EvenementLoto_ui`): ImGui/Vulkan-based interface (views, popups, actions, event handling, theming)
  - `source/gui/event/` - Event system (keyboard/mouse events, key codes, application events)
  - `source/gui/views/` - View components (MainView, DisplayView, HelpView, MenuBar, ToolBar, StatusBar, ConfigPopups, HelpPopups)
  - `source/gui/actions/` - Action handlers (FileActions, GameActions, SettingsActions, HelpActions)
  - `source/gui/vulkan/` - Vulkan rendering (VulkanContext, TextureLibrary, vkData)
  - `source/gui/utils/` - UI utilities (FileDialog, Convert, MarkdownParser, Rendering helpers)
  - `source/gui/fonts/` - Embedded fonts (Roboto-Regular, Roboto-Bold, Roboto-Italic as `.embed` files)
- `source/third_party/` - Single translation unit instantiating the header-only third
  parties, excluded from the project warnings and from clang-tidy
- `source/resources/` - Resources copied at build time (dark icons, user documentation + images)
- `source/main.cpp` - Entry point (ImGui UI); returns `EXIT_FAILURE` when the application ends in `State::Error`
- `test/lib_test/` - Unit tests for core library (Google Test, 10 test files)
- `test/gui_test/` - Unit tests for GUI library (Google Test, 6 test files)
- `ci/` - Python-based CI scripts (build, test, coverage, deploy, documentation)
- `cmake/` - CMake modules (BaseConfig, Conan, conan_provider, Vulkan, Sanitizers,
  Coverage, Poetry, UtilityFunctions, DocumentationConfig) + preset fragments
- `conan/` - Conan profiles and `global.conf` (`conan/config/`) and the in-tree recipe
  index (`conan/local-recipes/`)
- `document/` - User documentation (in French)
- `data/` - Runtime data files

### Key Domain Classes (namespace `evl::core`)

- `Event` - Top-level event: contains organizer info, rules, logos, game rounds, status state machine
- `GameRound` - A single game round: type (OneQuine, TwoQuines, FullCard, combinations, Enfant, Inverse, Pause), sub-rounds, draws
- `SubGameRound` - A sub-game within a round: type, draws, prize info, winner
- `Serializable` - Abstract base for binary stream, JSON (jsoncpp), and YAML (yaml-cpp) serialization
- `Settings` - Application settings (key-value store)
- `Statistics` - Draw statistics tracking
- `EnumLabel.h` - `constexpr` enum ⇄ French label tables, replacing static maps that
  could throw during static initialization
- `AtomicFile.h` - `writeFileAtomically()`: write to `<name>.tmp`, flush, check, then
  rename, so a crash never destroys the previous version
- `StreamRead.h` - defensive binary readers (`readRaw`, `readEnum`, `readLength`,
  `readString`, `readVector`); the stream's `failbit` is the error channel
- `Rescue.h` - the interrupted game: `saveRescue`, `findRescue`, `loadRescue`,
  `archiveRescue`, with two generations of `rescue.lev`
- `RandomNumberGenerator` - Number drawing engine (uses `std::mt19937` + `std::uniform_int_distribution`)
- `Log` - Logging wrapper around spdlog, with `LogBuffer` for in-app log display

### Math Utilities (namespace `evl::math`)

- `vectors.h` - Generic `Vector<BaseType, Dim>` template (fixed-size array backed)
- Type aliases: `vec2`, `vec2i`, etc.

### GUI (namespace `evl::gui`)

- `Application` - Singleton application class, manages views/popups/actions, Vulkan
  rendering, autosave (`rescue.lev` every 10s during active gameplay, atomic, two
  generations) and the recovery prompt at startup
- `MainWindow` - GLFW window management with Vulkan surface
- `Theme` - Theme configuration for the UI (colors, rounding, spacing; persisted in settings)
- `event/` - Event system: `Event` base, `KeyEvent`, `MouseEvent`, `AppEvent`, `KeyCode`, `MouseCode`
- `views/` - View, MainView, DisplayView, HelpView (non-modal markdown help), MenuBar, ToolBar, StatusBar, Popups, ConfigPopups, HelpPopups, RescuePopup (resume an interrupted game)
- `actions/` - Action base, FileActions, GameActions, SettingsActions, HelpActions
- `vulkan/` - VulkanContext (Vulkan instance/device/swapchain management), TextureLibrary (SVG/PNG/JPG loading), vkData
- `utils/` - FileDialog (open/save/folder dialogs), Convert (ImGui/core vector conversions), MarkdownParser (lightweight markdown-to-elements parser), Rendering (action buttons, text auto-fit)

## Build System

- **CMake 3.24+** with CMake Presets (`CMakePresets.json` includes Linux, MinGW, and CI presets)
- **C++ Standard**: C++23 (`CMAKE_CXX_STANDARD 23`)
- **Supported compilers**: GCC 14+, Clang 18+ (CI builds with GCC 14 and Clang 22)
- **Supported platforms**: Linux, Windows (MinGW)
- **Dependency management**: [Conan 2](https://conan.io) driven by CMake through
  [cmake-conan](https://github.com/conan-io/cmake-conan) `0.19.0` (`conanfile.py`, `conan/`)
- **Python tooling**: Poetry (`pyproject.toml`), Python 3.12+
- **Code formatting**: clang-format (`.clang-format`), cmake-format (`.cmake-format.json`)

### External Dependencies (via Conan)

glfw 3.4, gtest 1.17.0, imgui 1.92.9b-docking, jsoncpp 1.9.6, magic_enum 0.9.7,
nanosvg cci.20231025, nfd 1.2.1, spdlog 1.17.0, stb cci.20240531,
vulkan-headers/vulkan-loader 1.4.350.0, yaml-cpp 0.8.0

All come from ConanCenter except `nfd` (nativefiledialog-extended), which is not
published there and is built from the in-tree recipe in `conan/local-recipes/`.
X11 is declared as provided by the platform, so the build image must carry the X11
development packages.

### Python Dependencies (via Poetry)

- conan ^2.32, black ^25.12.0, gcovr ^8.6, rich ^14.2.0

Poetry owns the virtual environment and every build tool in it; `cmake/Poetry.cmake`
runs `poetry sync` at configure time and prepends the venv to `PATH`.

### Build Targets

- `EvenementLoto` - Main executable
- `EvenementLoto_third_party` - third parties that must be compiled locally: ImGui
  backends and `std::string` helper (ConanCenter ships them as sources only) and the
  single translation unit instantiating stb_image and nanosvg
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
# Configure + Build (no build/test presets defined, use output directory)
cmake --preset linux-gcc-release
cmake --build output/build/linux-gcc-release

# Run tests
cd output/build/linux-gcc-release && ctest --output-on-failure
```

## CI System

Python-based CI scripts in `ci/` (21 Python files), driven by `ci_action.py`:

- `ci/actions/build.py` - CMake configure + Ninja build
- `ci/actions/test.py` - Test execution
- `ci/actions/coverage.py` - gcovr coverage reports
- `ci/actions/deploy.py` - CPack packaging
- `ci/actions/documentation.py` - Doxygen documentation generation
- `ci/actions/clean.py` - Build directory cleanup
- `ci/actions/define_docker_image.py` - Docker image configuration
- `ci/actions/define_variables.py` - Variable definitions
- `ci/actions/python_requirements.py` - Python requirements handling
- `ci/utils/run.py` - Command execution with real-time output
- `ci/utils/preset.py` - CMake preset parsing (from `ci/PresetsParameters.json`)
- `ci/utils/teamcity.py` - TeamCity CI integration
- `ci/utils/cmake.py`, `ci/utils/docker.py`, `ci/utils/logging.py`, `ci/utils/python.py` - Additional utilities

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
- Backend: spdlog with console + rotating file sinks (5 MB × 5, appended so a crash
  trace survives the next start), flushed every second
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
- `test_Serialization.cpp` checks that no truncated or corrupted file is ever accepted
- `test_Rescue.cpp` checks the interrupted-game save, detection and fallback
- Sanitizer suppressions: `lsan_suppressions.txt` (suppresses known libdbus leaks for Address/Leak sanitizer presets)
