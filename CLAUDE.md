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
- `CHANGELOG.md` / `ROADMAP.md` - what is released and what is planned, one line each.
  `TODO.md` holds the detail of the work in progress.
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

### Robustness

- Vulkan waits are bounded: 2 s to acquire a swapchain image (rebuild on timeout), 5 s
  for a frame fence (a stuck GPU reports an error instead of freezing forever)
- `VK_ERROR_DEVICE_LOST` is reported with its own message; any fatal Vulkan error saves
  the game before leaving the loop
- `Application::saveProgress()` is called after every change of the game state (draw,
  cancelled draw, round change), on top of the 10 s periodic autosave

## Build System

- **CMake 3.24+** with CMake Presets (`CMakePresets.json` includes Linux, MinGW, and CI presets)
- **C++ Standard**: C++23 (`CMAKE_CXX_STANDARD 23`)
- **Supported compilers**: GCC 14+, Clang 18+ (CI builds with GCC 14 and Clang 22 from
  the same `builder-ubuntu2404` image)
- **Supported platforms**: Linux, Windows (MinGW)
- **Dependency management**: [Conan 2](https://conan.io) driven by CMake through
  [cmake-conan](https://github.com/conan-io/cmake-conan) `0.19.0` (`conanfile.py`, `conan/`)
- **Python tooling**: Poetry (`pyproject.toml`), Python 3.12+
- **Code formatting**: clang-format (`.clang-format`), cmake-format (`.cmake-format.json`)

### External Dependencies (via Conan)

glfw 3.4, gtest 1.17.0, imgui 1.92.9b-docking, jsoncpp 1.9.6, magic_enum 0.9.7,
nanosvg cci.20231025, nfd 1.2.1, spdlog 1.17.0, stb cci.20240531,
vulkan-headers/vulkan-loader 1.4.350.0, yaml-cpp 0.8.0

Plus `wayland` and `xkbcommon`, pulled in by glfw for its Wayland backend (build time
only: glfw `dlopen`s them by soname at runtime).

All come from ConanCenter except `nfd` (nativefiledialog-extended), which is not
published there and is built from the in-tree recipe in `conan/local-recipes/`.

Linux builds run in `registry.argawaen.net/builder/builder-ubuntu2404`, which carries
**both gcc 14 and clang 22** plus the full X11/XCB development set expected by
`xorg/system`, the Wayland and libdecor headers and `xkb-data`. Having gcc beside clang
matters: the **build** profile (`conan/config/profiles/linux-build`) always uses gcc,
because the autotools based build tools Conan compiles (flex, m4, libiconv…) expect a
native toolchain, and gcc matches the prebuilt tool packages, which are then shared
between the gcc and clang host profiles.

Both display servers are supported: glfw is built with `with_x11` and `with_wayland`,
and `vulkan-loader` carries the xlib, xcb and wayland WSI backends.

**X11 is asked for by default**, XWayland included, through `glfwInitHint(GLFW_PLATFORM,
…)` in `MainWindow::selectPlatform()` — the setting `gui/display_server` takes `x11`,
`wayland` or `auto`. The reason is not nostalgia: the detached display view has to go
fullscreen on a second screen, and Wayland forbids a client from choosing where its own
windows land. On Wayland the ImGui multi-viewport flag therefore stays off and the
monitor hosting the control window is deduced from the primary monitor.

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
cmake --preset linux-analysis            # compilation database for ci_action.py Analysis
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

Python-based CI scripts in `ci/`, driven by `ci_action.py`. Unrecognised arguments are
passed through to the action, so `Analysis` takes flags (`--tool`, `--mode`, …) instead
of environment variables; an action that takes none refuses them rather than ignoring
them.

Every finding a CI action reports is printed as `path:line:column: level: message`: the
teamcity-github-bridge plugin scans the build log for that shape and turns each one into
a GitHub check run annotation pinned to the diff. A finding printed otherwise never
leaves the build log.

The TeamCity configurations are described in `.teamcity/settings.kts`:

- **Build** (one per preset) and the **sanitizers** — pull requests marked ready, and
  pushes to `main`
- **Code Style** — the only one that also runs on **draft** pull requests
- **Clang-Tidy (diff)** / **Static Analyzer (diff)** — the gate: a finding fails the
  build and lands on the diff
- **Clang-Tidy** / **Static Analyzer** — the full scan on `main`, findings are warnings
  and are not annotated
- **Package** (Linux, Windows) — never triggered by a pull request

Branch pushes are built by a `vcsTrigger` limited to `main`: the bridge plugin enqueues
builds from pull request events only, its webhook controller ignoring `push`.

The actions:

- `ci/actions/build.py` - CMake configure + Ninja build
- `ci/actions/test.py` - Test execution
- `ci/actions/coverage.py` - gcovr coverage reports
- `ci/actions/package.py` - the ready-to-run archive: configure, build, CPack, and the
  `.tar.gz` renamed `.tgz` so `EvenementLoto-<version>.tgz` / `.zip` is the only name
- `ci/actions/code_style.py` - clang-format and black, inspect only, never rewrite
- `ci/actions/analysis.py` - clang-tidy or the same binary restricted to
  `clang-analyzer-*`, over every translation unit or only over what a diff touches
- `ci/actions/documentation.py` - Doxygen documentation generation
- `ci/actions/clean.py` - Build directory cleanup
- `ci/actions/define_docker_image.py` - Docker image configuration
- `ci/actions/define_variables.py` - Variable definitions
- `ci/actions/python_requirements.py` - Python requirements handling
- `ci/utils/run.py` - Command execution with real-time output
- `ci/utils/changed_tus.py` - which translation units a diff requires analysing: a
  changed header pulls in every unit that includes it, transitively
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
- Sanitizer suppressions: `lsan_suppressions.txt` (known libdbus leaks, Address/Leak
  presets) and `tsan_suppressions.txt` (races inside lavapipe and its LLVM JIT, which
  run because the GUI tests create a real window). Both are scoped so a finding in our
  own code is still reported
- The GUI suite needs a display: `ctest` runs it under `xvfb-run -a`, with lavapipe as
  the Vulkan driver. `test_Application.cpp` exercises the render loop and the exception
  net, registering a throwing view through `Application::addView()`
