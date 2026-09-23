/**
 * @file MainWindow.h
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Theme.h"

#include "core/maths/vectors.h"
#include "event/Event.h"
#include "event/KeyCodes.h"

#include <functional>

namespace evl::gui {

/**
 * @brief Struct MainWindowOptions.
 */
struct MainWindowOptions {
	/// Window title.
	std::string title;
	/// Window size.
	math::vec2ui size{1280, 800};
	/// Icon path.
	std::filesystem::path iconPath;
};

/**
 * @brief Struct MonitorInfo.
 */
struct MonitorInfo {
	/// Monitor name.
	std::string name;
	/// Position.
	math::vec2i position{0, 0};
	/// Size.
	math::vec2ui size{0, 0};
	/// Physical size.
	math::vec2i physicalSize{0, 0};
	/// Work area position.
	math::vec2i workAreaPosition{0, 0};
	/// Work area size.
	math::vec2i workAreaSize{0, 0};

	/// Is main window.
	bool isMainWindow = false;
};

/**
 * @brief Class MainWindow.
 */
class MainWindow final {
public:
	/// Call back function's type.
	using event_callback = std::function<void(event::Event&)>;
	/**
	 * @brief Default constructor.
	 */
	MainWindow();
	/**
	 * @brief Default destructor.
	 */
	~MainWindow();

	MainWindow(const MainWindow&) = delete;
	MainWindow(MainWindow&&) = delete;
	auto operator=(const MainWindow&) -> MainWindow& = delete;
	auto operator=(MainWindow&&) -> MainWindow& = delete;

	/**
	 * @brief Detect if the window should close.
	 * @return True if the window should close.
	 */
	[[nodiscard]] auto shouldClose() const -> bool;

	/**
	 * @brief Initialize the window.
	 * @param iOptions The window options.
	 */
	void init(const MainWindowOptions& iOptions);

	/**
	 * @brief Close the window.
	 */
	void close();

	/**
	 * @brief Start a new frame.
	 */
	void newFrame();
	/**
	 * @brief Render the window.
	 * @param iClearColor The clear color.
	 */
	void render(const math::vec4& iClearColor = {0.45f, 0.55f, 0.60f, 1.00f});

	/**
	 * @brief Set the theme.
	 * @param iTheme The theme to set.
	 */
	void setTheme(const Theme& iTheme);

	/**
	 * @brief Define the Event Callback function.
	 * @param iCallback The new callback function.
	 */
	void setEventCallback(const event_callback& iCallback) { m_windowData.eventCallback = iCallback; }

	/**
	 * @brief Check if a key is pressed.
	 * @param iKeycode The key code to check.
	 * @return True if the key is pressed.
	 */
	[[nodiscard]] auto isKeyPressed(const KeyCode& iKeycode) const -> bool;

	/**
	 * @brief Get the current modifiers.
	 * @return The current modifiers.
	 */
	[[nodiscard]] auto getModifiers() const -> Modifiers;

	/**
	 * @brief Set the window icon.
	 * @param iIconName The icon file name.
	 */
	void setIcon(const std::string& iIconName) const;

	/**
	 * @brief Ferme la frame en cours sans la rendre.
	 *
	 * À appeler quand une image a échoué : une frame ouverte par `newFrame()` doit être
	 * fermée, sinon la suivante déclenche le contrôle de cohérence d'ImGui.
	 */
	static void abandonFrame();
	/**
	 * @brief Event handler.
	 * @param[in,out] ioEvent The Event to react.
	 */
	void onEvent(event::Event& ioEvent);

	/**
	 * @brief Get the native window pointer.
	 * @return The native window pointer.
	 */
	[[nodiscard]] auto getNativeWindow() const -> void* { return m_window; }

	/**
	 * @brief Get monitor information.
	 * @return The monitor information.
	 */
	[[nodiscard]] auto getMonitorsInfo() const -> std::vector<MonitorInfo>;

private:
	/// Window options.
	MainWindowOptions m_options{};
	/// Native window pointer.
	void* m_window{};

	/**
	 * @brief How far `init()` got, so `close()` undoes exactly that and no more.
	 *
	 * `init()` gives up at the first step that fails — no display server, no window, no
	 * Vulkan — and the application then leaves through `main()`'s `EXIT_FAILURE`. It
	 * must not leave through a segmentation fault in a teardown that assumed the start
	 * had succeeded.
	 */
	enum struct Stage : uint8_t {
		Nothing,///< Not even GLFW is up.
		Glfw,///< `glfwInit` succeeded.
		Window,///< The window exists.
		Vulkan,///< The Vulkan context, the surface and the swapchain exist.
		ImGuiContext,///< The ImGui context exists.
		Backends,///< The GLFW and Vulkan backends are initialised.
	};
	/// What has been set up, and therefore what has to be taken down.
	Stage m_stage = Stage::Nothing;
	/// Swap chain rebuild flag.
	bool m_swapChainRebuild = false;
	/// Current theme.
	Theme m_currentTheme{};
	/// Fonts loaded flag.
	bool m_fontsLoaded = false;
	/// Minimum image count.
	uint32_t m_minImageCount = 2;
	/// Vulkan window setup done flag.
	bool m_windowSetupDone = false;
	/// True on a Wayland session, where a client cannot position its own windows.
	bool m_wayland = false;

	/**
	 * @brief Choisit le serveur d'affichage avant l'initialisation de GLFW.
	 *
	 * Wayland interdit à un client de placer ses propres fenêtres, or la vue
	 * d'affichage détachée doit aller sur le second écran — c'est tout l'intérêt de la
	 * version « docking » d'ImGui ici. X11, via XWayland en session Wayland, le permet,
	 * donc il est préféré sauf réglage contraire (`gui/display_server`).
	 */
	static void selectPlatform();
	/// Setup Vulkan window.
	void setupVulkanWindow(int iWidth, int iHeight);
	/// Cleanup Vulkan window.
	void cleanupVulkanWindow();
	/// Set callbacks.
	void setCallbacks();
	/**
	 * @brief Window's data.
	 */
	struct WindowData {
		/// Window's size.
		math::vec2ui size;
		/// Event Call back.
		event_callback eventCallback;
	};

	/// The Window's data.
	WindowData m_windowData{};
};

}// namespace evl::gui
