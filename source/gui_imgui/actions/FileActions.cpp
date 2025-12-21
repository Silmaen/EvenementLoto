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
#include "gui_imgui/utils/FileDialog.h"

namespace evl::gui_imgui::actions {

NewFileAction::NewFileAction() { setIconName("new-file"); }
NewFileAction::~NewFileAction() = default;
void NewFileAction::onExecute() {
	log_trace("New file action executed.");
	auto& app = Application::get();
	app.getCurrentEvent() = core::Event{};
	app.getCurrentFile().clear();
}


LoadFileAction::LoadFileAction() { setIconName("folder_open"); }
LoadFileAction::~LoadFileAction() = default;
void LoadFileAction::onExecute() {
	log_trace("Load file action executed.");
	const auto file = utils::FileDialog::openFile("Loto Files|*.lev");
	if (file.empty() || !exists(file)) {
		log_trace("Load file action canceled.");
		return;
	}
	auto& app = Application::get();
	app.getCurrentFile() = file;

	std::ifstream f;
	f.open(file, std::ios::in | std::ios::binary);
	app.getCurrentEvent().setBasePath(file);
	app.getCurrentEvent().read(f, 0);
	f.close();
}


SaveFileAction::SaveFileAction() { setIconName("save"); }
SaveFileAction::~SaveFileAction() = default;
void SaveFileAction::onExecute() {
	log_trace("Save file action executed.");
	auto& app = Application::get();
	auto file = app.getCurrentFile();
	if (file.empty()) {
		file = utils::FileDialog::saveFile("Loto Files|*.lev");
		if (file.empty()) {
			log_trace("Save file action canceled.");
			return;
		}
	}
	std::ofstream f;
	f.open(file, std::ios::out | std::ios::binary);
	app.getCurrentEvent().setBasePath(file);
	app.getCurrentEvent().write(f);
	f.close();
}


SaveAsFileAction::SaveAsFileAction() { setIconName("save-as"); }
SaveAsFileAction::~SaveAsFileAction() = default;
void SaveAsFileAction::onExecute() {
	log_trace("SaveAs file action executed.");
	auto& app = Application::get();
	auto file = app.getCurrentFile();
	auto newfile = utils::FileDialog::saveFile("Loto Files|*.lev");
	if (newfile.empty()) {
		log_trace("Save file action canceled.");
		return;
	}
	file = newfile;
	std::ofstream f;
	f.open(file, std::ios::out | std::ios::binary);
	app.getCurrentEvent().setBasePath(file);
	app.getCurrentEvent().write(f);
	f.close();
}

StartGameAction::StartGameAction() { setIconName("toggle-on"); }
StartGameAction::~StartGameAction() = default;
void StartGameAction::onExecute() { log_trace("Start game action executed."); }


StopGameAction::StopGameAction() { setIconName("toggle-off"); }
StopGameAction::~StopGameAction() = default;
void StopGameAction::onExecute() { log_trace("Stop game action executed."); }

QuitAction::QuitAction() = default;
QuitAction::~QuitAction() = default;
void QuitAction::onExecute() {
	log_trace("Quit action executed.");
	Application::get().requestClose();
}

}// namespace evl::gui_imgui::actions
