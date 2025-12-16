/**
 * @file FileActions.cpp
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "FileActions.h"

#include "gui_imgui/Application.h"

namespace evl::gui_imgui::actions {

NewFileAction::NewFileAction() { setIconName("new-file"); }
NewFileAction::~NewFileAction() = default;
LoadFileAction::LoadFileAction() { setIconName("folder_open"); }
LoadFileAction::~LoadFileAction() = default;
SaveFileAction::SaveFileAction() { setIconName("save"); }
SaveFileAction::~SaveFileAction() = default;
SaveAsFileAction::SaveAsFileAction() { setIconName("save-as"); }
SaveAsFileAction::~SaveAsFileAction() = default;
StartGameAction::StartGameAction() { setIconName("toggle-on"); }
StartGameAction::~StartGameAction() = default;
StopGameAction::StopGameAction() { setIconName("toggle-off"); }
StopGameAction::~StopGameAction() = default;

QuitAction::QuitAction() = default;
QuitAction::~QuitAction() = default;
void QuitAction::onExecute() {
	log_trace("Quit action executed.");
	Application::get().requestClose();
}

}// namespace evl::gui_imgui::actions
