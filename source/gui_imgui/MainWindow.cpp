/**
 * @file MainWindow.cpp
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Application.h"
#include "MainWindow.h"
#include "core/Log.h"
#include "vulkan/VulkanContext.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>// NOLINT
#include <imgui.h>


// memory fonts...
#include "gui_imgui//fonts/Roboto-Bold.embed"
#include "gui_imgui//fonts/Roboto-Italic.embed"
//#include "core/fonts/Roboto-Regular.embed"// not used here

#include "event/AppEvent.h"
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"

namespace evl::gui_imgui {

namespace {

std::shared_ptr<ImGui_ImplVulkanH_Window> g_MainWindowData;

void glfw_error_callback(int iError, const char* iDescription) { log_error("GLFW Error %d: %s", iError, iDescription); }

auto vec(const math::vec4& iVec) -> ImVec4 { return {iVec.x(), iVec.y(), iVec.z(), iVec.w()}; }

}// namespace


MainWindow::MainWindow() = default;

MainWindow::~MainWindow() = default;


void MainWindow::init(const MainWindowOptions& iOptions) {
	m_options = iOptions;
	glfwSetErrorCallback(glfw_error_callback);
	if (glfwInit() == 0) {
		Application::get().reportError("Failed to initialize GLFW");
		return;
	}

	// Create window with Vulkan context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	const float main_scale =
			ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());// Valid on GLFW 3.3+ only
	GLFWwindow* window = glfwCreateWindow(static_cast<int>(static_cast<float>(m_options.size.x()) * main_scale),
										  static_cast<int>(static_cast<float>(m_options.size.y()) * main_scale),
										  m_options.title.c_str(), nullptr, nullptr);
	m_window = window;
	if (glfwVulkanSupported() == 0) {
		Application::get().reportError("GLFW: Vulkan Not Supported");
		return;
	}

	// Setup renderer
	{
		std::vector<const char*> extensions;
		{
			uint32_t extensions_count = 0;
			const char* const* glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
			extensions.reserve(extensions_count);
			for (uint32_t i = 0; i < extensions_count; i++) extensions.push_back(glfw_extensions[i]);
		}

		auto& vkContext = vulkan::VulkanContext::get();
		vkContext.init(extensions);
		g_MainWindowData = std::make_shared<ImGui_ImplVulkanH_Window>();

		auto [allocator, instance, physicalDevice, device, queueFamily, queue, pipelineCache, descriptorPool,
			  commandPool] = vkContext.getVkData();
		VkSurfaceKHR surface = nullptr;

		const VkResult err = glfwCreateWindowSurface(instance, window, allocator, &surface);
		vulkan::VulkanContext::checkVkResult(err);
		// Create Framebuffers
		int w = 0;
		int h = 0;
		glfwGetFramebufferSize(window, &w, &h);
		g_MainWindowData->Surface = surface;
		setupVulkanWindow(w, h);
	}

	// Setup Dear ImGui context
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;// Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;// Enable Multi-Viewport
		io.ConfigViewportsNoDecoration = true;
		io.ConfigViewportsNoAutoMerge = false;

		// Setup scaling
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(
				main_scale);// Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		style.FontScaleDpi =
				main_scale;// Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
	}

	// Setup Platform/Renderer backends
	{
		ImGui_ImplGlfw_InitForVulkan(window, true);
		auto [allocator, instance, physicalDevice, device, queueFamily, queue, pipelineCache, descriptorPool,
			  commandPool] = vulkan::VulkanContext::get().getVkData();
		ImGui_ImplVulkan_InitInfo init_info = {.ApiVersion = VK_API_VERSION_1_4,
											   .Instance = instance,
											   .PhysicalDevice = physicalDevice,
											   .Device = device,
											   .QueueFamily = queueFamily,
											   .Queue = queue,
											   .DescriptorPool = descriptorPool,
											   .DescriptorPoolSize = 0,
											   .MinImageCount = m_minImageCount,
											   .ImageCount = g_MainWindowData->ImageCount,
											   .PipelineCache = pipelineCache,
											   .PipelineInfoMain = {.RenderPass = g_MainWindowData->RenderPass,
																	.Subpass = 0,
																	.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
#ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
																	.PipelineRenderingCreateInfo = {},
#endif
																	.SwapChainImageUsage = {}},
											   .PipelineInfoForViewports = {},
											   .UseDynamicRendering = false,
											   .Allocator = allocator,
											   .CheckVkResultFn = vulkan::VulkanContext::checkVkResult,
											   .MinAllocationSize = 0,
											   .CustomShaderVertCreateInfo = {},
											   .CustomShaderFragCreateInfo = {}};
		ImGui_ImplVulkan_Init(&init_info);
	}
	if (Application::get().getState() == Application::State::Error)
		return;

	setTheme({});

	setCallbacks();
}

void MainWindow::setupVulkanWindow(const int iWidth, const int iHeight) {
	const auto vkData = vulkan::VulkanContext::get().getVkData();

	// Check for WSI support
	VkBool32 res = 0;
	vkGetPhysicalDeviceSurfaceSupportKHR(vkData.physicalDevice, vkData.queueFamily, g_MainWindowData->Surface, &res);
	if (res != VK_TRUE) {
		log_error("Error no WSI support on physical device 0");
		Application::get().reportError("Vulkan WSI not supported.");
		return;
	}

	// Select Surface Format
	const std::vector<VkFormat> requestSurfaceImageFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
															 VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
	constexpr VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	g_MainWindowData->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
			vkData.physicalDevice, g_MainWindowData->Surface, requestSurfaceImageFormat.data(),
			static_cast<int>(requestSurfaceImageFormat.size()), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
	const std::vector<VkPresentModeKHR> present_modes = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
														 VK_PRESENT_MODE_FIFO_KHR};
#else
	const std::vector<VkPresentModeKHR> present_modes = {VK_PRESENT_MODE_FIFO_KHR};
#endif
	g_MainWindowData->PresentMode =
			ImGui_ImplVulkanH_SelectPresentMode(vkData.physicalDevice, g_MainWindowData->Surface, present_modes.data(),
												static_cast<int>(present_modes.size()));
	log_info("[vulkan] Selected PresentMode = {}", magic_enum::enum_name(g_MainWindowData->PresentMode));

	// Create SwapChain, RenderPass, Framebuffer, etc.
	assert(m_minImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(vkData.instance, vkData.physicalDevice, vkData.device,
										   g_MainWindowData.get(), vkData.queueFamily, vkData.allocator, iWidth,
										   iHeight, m_minImageCount, 0);
	m_windowSetupDone = true;
}

void MainWindow::cleanupVulkanWindow() {
	if (!m_windowSetupDone)
		return;
	const auto vkData = vulkan::VulkanContext::get().getVkData();
	ImGui_ImplVulkanH_DestroyWindow(vkData.instance, vkData.device, g_MainWindowData.get(), vkData.allocator);
	m_windowSetupDone = false;
}

void MainWindow::setCallbacks() {
	auto* window = static_cast<GLFWwindow*>(m_window);
	glfwSetWindowUserPointer(window, &m_windowData);
	glfwSetWindowSizeCallback(window, [](GLFWwindow* iWindow, const int iWidth, const int iHeight) -> void {
		auto* const data = static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow));
		data->size.x() = static_cast<uint32_t>(iWidth);
		data->size.y() = static_cast<uint32_t>(iHeight);

		event::WindowResizeEvent event(data->size);
		data->eventCallback(event);
	});
	glfwSetWindowCloseCallback(window, [](GLFWwindow* iWindow) -> void {
		event::WindowCloseEvent event;
		static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
	});
	glfwSetKeyCallback(
			window,
			[](GLFWwindow* iWindow, const int iKey, [[maybe_unused]] int iScancode, const int iAction,
			   [[maybe_unused]] int iMods) -> void {
				const auto cKey = static_cast<KeyCode>(iKey);
				switch (iAction) {
					case GLFW_PRESS:
						{
							event::KeyPressedEvent event(cKey, 0u);
							static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
							break;
						}
					case GLFW_RELEASE:
						{
							event::KeyReleasedEvent event(cKey);
							static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
							break;
						}
					case GLFW_REPEAT:
						{
							event::KeyPressedEvent event(cKey, 1u);
							static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
							break;
						}
					default:
						break;
				}
			});
	glfwSetCharCallback(window, [](GLFWwindow* iWindow, const unsigned int iKeycode) -> void {
		event::KeyTypedEvent event(static_cast<KeyCode>(iKeycode));
		static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
	});
	glfwSetMouseButtonCallback(
			window,
			[](GLFWwindow* iWindow, const int iButton, const int iAction, [[maybe_unused]] const int iMods) -> void {
				switch (iAction) {
					case GLFW_PRESS:
						{
							event::MouseButtonPressedEvent event(static_cast<MouseCode>(iButton));
							static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
							break;
						}
					case GLFW_RELEASE:
						{
							event::MouseButtonReleasedEvent event(static_cast<MouseCode>(iButton));
							static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
							break;
						}
					default:
						break;
				}
			});
	glfwSetScrollCallback(window, [](GLFWwindow* iWindow, const double iXOffset, const double iYOffset) -> void {
		event::MouseScrolledEvent event(static_cast<float>(iXOffset), static_cast<float>(iYOffset));
		static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow* iWindow, const double iX, const double iY) -> void {
		event::MouseMovedEvent event(static_cast<float>(iX), static_cast<float>(iY));
		static_cast<WindowData*>(glfwGetWindowUserPointer(iWindow))->eventCallback(event);
	});
}

void MainWindow::close() {
	const auto vkData = vulkan::VulkanContext::get().getVkData();
	const auto err = vkDeviceWaitIdle(vkData.device);
	vulkan::VulkanContext::checkVkResult(err);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	cleanupVulkanWindow();
	g_MainWindowData.reset();
	vulkan::VulkanContext::get().reset();

	auto* window = static_cast<GLFWwindow*>(m_window);
	glfwDestroyWindow(window);
	glfwTerminate();
}

auto MainWindow::shouldClose() const -> bool {
	auto* window = static_cast<GLFWwindow*>(m_window);
	return glfwWindowShouldClose(window) != 0;
}

void MainWindow::newFrame() {
	// Poll and handle events (inputs, window resize, etc.)
	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
	glfwPollEvents();
	auto* window = static_cast<GLFWwindow*>(m_window);
	const auto vkData = vulkan::VulkanContext::get().getVkData();

	// Resize swap chain?
	int fb_width = 0;
	int fb_height = 0;
	glfwGetFramebufferSize(window, &fb_width, &fb_height);
	if (fb_width > 0 && fb_height > 0 &&
		(m_swapChainRebuild || g_MainWindowData->Width != fb_width || g_MainWindowData->Height != fb_height)) {
		ImGui_ImplVulkan_SetMinImageCount(m_minImageCount);
		ImGui_ImplVulkanH_CreateOrResizeWindow(vkData.instance, vkData.physicalDevice, vkData.device,
											   g_MainWindowData.get(), vkData.queueFamily, vkData.allocator, fb_width,
											   fb_height, m_minImageCount, 0);
		g_MainWindowData->FrameIndex = 0;
		m_swapChainRebuild = false;
	}
	auto& app = Application::get();
	if (app.getState() != Application::State::Running && app.getState() != Application::State::Waiting)
		return;
	if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
		ImGui_ImplGlfw_Sleep(10);
		app.setWaiting();
		return;
	}
	app.setRunning();

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void MainWindow::render(const math::vec4& iClearColor) {
	// Rendering
	ImGui::Render();
	if (const ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	ImDrawData* draw_data = ImGui::GetDrawData();

	if (const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		!is_minimized) {
		g_MainWindowData->ClearValue.color.float32[0] = iClearColor.r() * iClearColor.a();
		g_MainWindowData->ClearValue.color.float32[1] = iClearColor.g() * iClearColor.a();
		g_MainWindowData->ClearValue.color.float32[2] = iClearColor.b() * iClearColor.a();
		g_MainWindowData->ClearValue.color.float32[3] = iClearColor.a();
		vulkan::VulkanContext::get().frameRender(g_MainWindowData.get(), draw_data, m_swapChainRebuild);
	}
}


void MainWindow::setTheme(const Theme& iTheme) {
	m_currentTheme = iTheme;
	const ImGuiIO& io = ImGui::GetIO();
	// Better fonts
	if (!m_fontsLoaded) {
		m_fontsLoaded = true;

		ImFontConfig fontConfig;
		fontConfig.FontDataOwnedByAtlas = false;
		// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
		//ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(g_RobotoRegular)),
		//													sizeof(g_RobotoRegular), 20.0f, &fontConfig);
		io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(g_RobotoBold)), sizeof(g_RobotoBold),
									   20.0f, &fontConfig);
		io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(g_RobotoItalic)),
									   sizeof(g_RobotoItalic), 20.0f, &fontConfig);
		// NOLINTEND(cppcoreguidelines-pro-type-const-cast)
	}
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	auto& colors = ImGui::GetStyle().Colors;
	// ======================
	// Colors

	// Text 1 2
	colors[ImGuiCol_Text] = vec(iTheme.text);
	// colors[ImGuiCol_TextDisabled] = vec(iTheme.textDisabled);
	// Window Background 2 3 4 5
	colors[ImGuiCol_WindowBg] = vec(iTheme.windowBackground);
	colors[ImGuiCol_ChildBg] = vec(iTheme.childBackground);
	colors[ImGuiCol_PopupBg] = vec(iTheme.backgroundPopup);
	colors[ImGuiCol_Border] = vec(iTheme.border);
	// colors[ImGuiCol_BorderShadow] = vec(iTheme.border);
	// Frame BG 7 8 9
	colors[ImGuiCol_FrameBg] = vec(iTheme.frameBackground);
	colors[ImGuiCol_FrameBgHovered] = vec(iTheme.frameBackgroundHovered);
	colors[ImGuiCol_FrameBgActive] = vec(iTheme.frameBackgroundActive);
	// Title 10 11 12
	colors[ImGuiCol_TitleBg] = vec(iTheme.titleBar);
	colors[ImGuiCol_TitleBgActive] = vec(iTheme.titleBarActive);
	colors[ImGuiCol_TitleBgCollapsed] = vec(iTheme.titleBarCollapsed);
	// Menubar 13
	colors[ImGuiCol_MenuBarBg] = vec(iTheme.menubarBackground);
	// Scrollbar 14 15 16 17
	colors[ImGuiCol_ScrollbarBg] = vec(iTheme.scrollbarBackground);
	colors[ImGuiCol_ScrollbarGrab] = vec(iTheme.scrollbarGrab);
	colors[ImGuiCol_ScrollbarGrabHovered] = vec(iTheme.scrollbarGrabHovered);
	colors[ImGuiCol_ScrollbarGrabActive] = vec(iTheme.scrollbarGrabActive);
	// Checkbox 18
	colors[ImGuiCol_CheckMark] = vec(iTheme.checkMark);
	// Slider 19 20
	colors[ImGuiCol_SliderGrab] = vec(iTheme.sliderGrab);
	colors[ImGuiCol_SliderGrabActive] = vec(iTheme.sliderGrabActive);
	// Buttons 21 22 23
	colors[ImGuiCol_Button] = vec(iTheme.button);
	colors[ImGuiCol_ButtonHovered] = vec(iTheme.buttonHovered);
	colors[ImGuiCol_ButtonActive] = vec(iTheme.buttonActive);
	// Headers 24 25 26
	colors[ImGuiCol_Header] = vec(iTheme.groupHeader);
	colors[ImGuiCol_HeaderHovered] = vec(iTheme.groupHeaderHovered);
	colors[ImGuiCol_HeaderActive] = vec(iTheme.groupHeaderActive);
	// Separator 27 28 29
	colors[ImGuiCol_Separator] = vec(iTheme.separator);
	colors[ImGuiCol_SeparatorActive] = vec(iTheme.separatorActive);
	colors[ImGuiCol_SeparatorHovered] = vec(iTheme.separatorHovered);
	// Resize Grip 30 31 32
	colors[ImGuiCol_ResizeGrip] = vec(iTheme.resizeGrip);
	colors[ImGuiCol_ResizeGripHovered] = vec(iTheme.resizeGripHovered);
	colors[ImGuiCol_ResizeGripActive] = vec(iTheme.resizeGripActive);
	// Tabs 33 34 35 36 37 38 39
	colors[ImGuiCol_TabHovered] = vec(iTheme.tabHovered);
	colors[ImGuiCol_Tab] = vec(iTheme.tab);
	colors[ImGuiCol_TabSelected] = vec(iTheme.tabSelected);
	colors[ImGuiCol_TabSelectedOverline] = vec(iTheme.tabSelectedOverline);
	colors[ImGuiCol_TabDimmed] = vec(iTheme.tabDimmed);
	colors[ImGuiCol_TabDimmedSelected] = vec(iTheme.tabDimmedSelected);
	colors[ImGuiCol_TabDimmedSelectedOverline] = vec(iTheme.tabDimmedSelectedOverline);
	// Docking 40 41
	colors[ImGuiCol_DockingPreview] = vec(iTheme.dockingPreview);
	colors[ImGuiCol_DockingEmptyBg] = vec(iTheme.dockingEmptyBackground);
	// PlotLines 42 43 44 44
	// colors[ImGuiCol_PlotLines] = vec(iTheme.Text);
	// colors[ImGuiCol_PlotLinesHovered] = vec(iTheme.Text);
	// colors[ImGuiCol_PlotHistogram] = vec(iTheme.Text);
	// colors[ImGuiCol_PlotHistogramHovered] = vec(iTheme.Text);
	/// Tables 46 47 48 49 50
	colors[ImGuiCol_TableHeaderBg] = vec(iTheme.groupHeader);
	// colors[ImGuiCol_TableBorderStrong] = vec.(iTheme.Text);
	colors[ImGuiCol_TableBorderLight] = vec(iTheme.border);
	// colors[ImGuiCol_TableRowBg] = vec.(iTheme.Text);
	// colors[ImGuiCol_TableRowBgAlt] = vec.(iTheme.Text);
	// Text Selected 51
	// colors[ImGuiCol_TextSelectedBg] = vec.(iTheme.Text);
	// Drag n DRop 52
	// colors[ImGuiCol_DragDropTarget] = vec.(iTheme.Text);
	// Nav 53 54 55
	// colors[ImGuiCol_NavHighlight] = vec.(iTheme.Text);
	// colors[ImGuiCol_NavWindowingHighlight] = vec.(iTheme.Text);
	// colors[ImGuiCol_NavWindowingDimBg] = vec.(iTheme.Text);
	// Modal window 56
	// colors[ImGuiCol_ModalWindowDimBg] = vec.(iTheme.Text);

	//========================================================
	// Style
	auto& style = ImGui::GetStyle();

	// rounding
	style.WindowRounding = iTheme.windowRounding;
	style.FrameRounding = iTheme.frameRounding;
	style.FrameBorderSize = iTheme.frameBorderSize;

	style.TabRounding = iTheme.tabRounding;
	style.TabBarOverlineSize = iTheme.tabOverline;
	style.TabBorderSize = iTheme.tabBorder;

	style.GrabRounding = iTheme.controlsRounding;
	style.ScrollbarRounding = iTheme.controlsRounding;

	style.IndentSpacing = iTheme.indentSpacing;
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.ColorButtonPosition = ImGuiDir_Left;
	// When viewports are enabled we tweak WindowRounding/WindowBg so platform
	// windows can look identical to regular ones.
	if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
}

auto MainWindow::isKeyPressed(const KeyCode& iKeycode) const -> bool {
	auto* window = static_cast<GLFWwindow*>(m_window);
	const int state = glfwGetKey(window, static_cast<int>(iKeycode));
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

auto MainWindow::getModifiers() const -> Modifiers {
	return Modifiers{
			.ctrl = isKeyPressed(KeyCode::LeftControl) || isKeyPressed(KeyCode::RightControl),
			.shift = isKeyPressed(KeyCode::LeftShift) || isKeyPressed(KeyCode::RightShift),
			.alt = isKeyPressed(KeyCode::LeftAlt) || isKeyPressed(KeyCode::RightAlt),
			.altGr = isKeyPressed(KeyCode::RightAlt),
	};
}


void MainWindow::onEvent(event::Event& ioEvent) {
	event::EventDispatcher dispatcher(ioEvent);
	dispatcher.dispatch<event::KeyPressedEvent>([this]<typename T>(const T& ioInternalEvent) -> auto {
		ImGui_ImplGlfw_KeyCallback(static_cast<GLFWwindow*>(m_window), static_cast<int>(ioInternalEvent.getKeyCode()),
								   0, GLFW_PRESS, 0);
		return true;
	});
	dispatcher.dispatch<event::KeyReleasedEvent>([this]<typename T>(const T& ioInternalEvent) -> auto {
		ImGui_ImplGlfw_KeyCallback(static_cast<GLFWwindow*>(m_window), static_cast<int>(ioInternalEvent.getKeyCode()),
								   0, GLFW_RELEASE, 0);
		return true;
	});
	dispatcher.dispatch<event::KeyTypedEvent>([this]<typename T>(const T& ioInternalEvent) -> auto {
		ImGui_ImplGlfw_KeyCallback(static_cast<GLFWwindow*>(m_window), static_cast<int>(ioInternalEvent.getKeyCode()),
								   0, GLFW_REPEAT, 0);
		return true;
	});
	dispatcher.dispatch<event::MouseMovedEvent>([this]<typename T>(const T& ioInternalEvent) -> auto {
		ImGui_ImplGlfw_CursorPosCallback(static_cast<GLFWwindow*>(m_window),
										 static_cast<double>(ioInternalEvent.getX()),
										 static_cast<double>(ioInternalEvent.getY()));
		return true;
	});
	dispatcher.dispatch<event::MouseScrolledEvent>([this]<typename T>(const T& ioInternalEvent) -> auto {
		ImGui_ImplGlfw_ScrollCallback(static_cast<GLFWwindow*>(m_window),
									  static_cast<double>(ioInternalEvent.getXOff()),
									  static_cast<double>(ioInternalEvent.getYOff()));
		return true;
	});
	dispatcher.dispatch<event::MouseButtonPressedEvent>([this]<typename T>(const T& ioInternalEvent) -> auto {
		ImGui_ImplGlfw_MouseButtonCallback(static_cast<GLFWwindow*>(m_window),
										   static_cast<int>(ioInternalEvent.getMouseButton()), GLFW_PRESS, 0);
		return true;
	});
	dispatcher.dispatch<event::MouseButtonReleasedEvent>([this]<typename T>(const T& ioInternalEvent) -> auto {
		ImGui_ImplGlfw_MouseButtonCallback(static_cast<GLFWwindow*>(m_window),
										   static_cast<int>(ioInternalEvent.getMouseButton()), GLFW_RELEASE, 0);
		return true;
	});
}

}// namespace evl::gui_imgui
