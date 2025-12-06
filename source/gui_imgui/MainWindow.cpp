/**
 * @file MainWindow.cpp
 * @author Silmaen
 * @date 05/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "MainWindow.h"

#include "Application.h"
#include "core/Log.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

namespace evl::gui_imgui {

/// memory fonts...
#include "Roboto-Bold.embed"
#include "Roboto-Italic.embed"
#include "Roboto-Regular.embed"

MainWindow::~MainWindow() {
	ImGui_ImplGlfw_Shutdown();
	// Destroy ImGui context
	ImGui::DestroyContext();
}

MainWindow::MainWindow(Parameters iParams) : Window(std::move(iParams)) {
	// Create ImGui context
	ImGui::CreateContext();
	if (ImGui::GetCurrentContext() != nullptr) {
		log_info("ImGui context created");
	} else {
		log_error("ImGui context creation failed");
	}
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad

	// Docking configuration
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;// Enable Docking
	io.ConfigDockingWithShift = false;
	io.ConfigDockingTransparentPayload = true;
	// ViewPort configuration
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;// Enable Multi-Viewport
	io.ConfigViewportsNoDecoration = true;
	io.ConfigViewportsNoAutoMerge = false;

	// Better fonts
	ImFontConfig fontConfig;
	fontConfig.FontDataOwnedByAtlas = false;
	// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
	ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(g_RobotoRegular)),
														sizeof(g_RobotoRegular), 20.0f, &fontConfig);
	io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(g_RobotoBold)), sizeof(g_RobotoBold),
								   20.0f, &fontConfig);
	io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(g_RobotoItalic)), sizeof(g_RobotoItalic),
								   20.0f, &fontConfig);
	// NOLINTEND(cppcoreguidelines-pro-type-const-cast)
	io.FontDefault = robotoFont;
	ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_window), true);
	if (!ImGui_ImplOpenGL3_Init("#version 430")) {
		log_error("ImGui OpenGL3 backend initialization failed");
		Application::get().reportError("");
	}
}

void MainWindow::onRender() {
	// Main window rendering code can be added here if needed
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	// Example window
	ImGui::Begin("Hello, ImGui!");
	ImGui::Text("This is the main window of the application.");
	ImGui::End();


	ImGui::EndFrame();
	ImGui::Render();
	GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	glfwMakeContextCurrent(backupCurrentContext);
}

}// namespace evl::gui_imgui
