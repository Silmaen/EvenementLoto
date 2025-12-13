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

NewFileAction::NewFileAction() = default;
NewFileAction::~NewFileAction() = default;
LoadFileAction::LoadFileAction() = default;
LoadFileAction::~LoadFileAction() = default;
SaveFileAction::SaveFileAction() = default;
SaveFileAction::~SaveFileAction() = default;
SaveAsFileAction::SaveAsFileAction() = default;
SaveAsFileAction::~SaveAsFileAction() = default;
StartGameAction::StartGameAction() = default;
StartGameAction::~StartGameAction() = default;
StopGameAction::StopGameAction() = default;
StopGameAction::~StopGameAction() = default;

QuitAction::QuitAction() = default;
QuitAction::~QuitAction() = default;
void QuitAction::onExecute() {
	log_trace("Quit action executed.");
	Application::get().requestClose();
}

}// namespace evl::gui_imgui::actions
