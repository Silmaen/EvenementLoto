/**
 * @file FileActions.cpp
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "FileActions.h"

#include "core/AtomicFile.h"
#include "gui/Application.h"
#include "gui/utils/FileDialog.h"

namespace evl::gui::actions {

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
	const auto file = utils::FileDialog::openFile(utils::g_gameFilter);
	if (file.empty() || !exists(file)) {
		log_trace("Load file action canceled.");
		return;
	}
	auto& app = Application::get();
	std::ifstream f(file, std::ios::in | std::ios::binary);
	if (!f.is_open()) {
		log_error("Impossible d'ouvrir '{}'.", file.string());
		return;
	}
	// Read into a candidate: a file that turns out to be truncated or corrupted must
	// not leave the application holding half an event, and the message must not claim
	// success. The current event is only replaced once the read is known complete.
	core::Event candidate;
	candidate.setBasePath(file);
	candidate.read(f, {});
	if (!f.good()) {
		log_error("Le fichier '{}' est incomplet ou corrompu, il n'a pas été chargé.", file.string());
		app.tell("Fichier illisible",
				 "Ce fichier est incomplet ou corrompu, il n'a pas été chargé. La partie en cours est intacte.",
				 file.string());
		return;
	}
	app.getCurrentEvent() = candidate;
	app.getCurrentFile() = file;
	log_info("File '{}' loaded successfully.", file.string());
}


SaveFileAction::SaveFileAction() { setIconName("save"); }
SaveFileAction::~SaveFileAction() = default;
void SaveFileAction::onExecute() {
	log_trace("Save file action executed.");
	auto& app = Application::get();
	auto file = app.getCurrentFile();
	if (file.empty() || !exists(file)) {
		if (file.empty()) {
			log_trace("No current file, prompting Save As dialog.");
		} else {
			log_trace("Current file '{}' does not exist, prompting Save As dialog.", file.string());
		}
		file = utils::FileDialog::saveFile(utils::g_gameFilter);
		if (file.empty()) {
			log_trace("Save file action canceled.");
			return;
		}
	}
	app.getCurrentEvent().setBasePath(file);
	if (!core::writeFileAtomically(file,
								   [&app](std::ostream& oStream) -> void { app.getCurrentEvent().write(oStream); })) {
		log_error("Failed to save file '{}'.", file.string());
		app.tell("Enregistrement impossible",
				 "Le fichier n'a pas pu être écrit. La partie n'est pas perdue : "
				 "la sauvegarde de secours continue.",
				 file.string());
		return;
	}
	Application::forgetRescue();
	log_info("File '{}' saved successfully.", file.string());
}


SaveAsFileAction::SaveAsFileAction() { setIconName("save-as"); }
SaveAsFileAction::~SaveAsFileAction() = default;
void SaveAsFileAction::onExecute() {
	log_trace("SaveAs file action executed.");
	auto& app = Application::get();
	auto file = app.getCurrentFile();
	auto newfile = utils::FileDialog::saveFile(utils::g_gameFilter);
	if (newfile.empty()) {
		log_trace("Save file action canceled.");
		return;
	}
	file = newfile;
	app.getCurrentEvent().setBasePath(file);
	if (!core::writeFileAtomically(file,
								   [&app](std::ostream& oStream) -> void { app.getCurrentEvent().write(oStream); })) {
		log_error("Failed to save file '{}'.", file.string());
		app.tell("Enregistrement impossible",
				 "Le fichier n'a pas pu être écrit. La partie n'est pas perdue : "
				 "la sauvegarde de secours continue.",
				 file.string());
		return;
	}
	app.getCurrentFile() = file;
	Application::forgetRescue();
	log_info("File '{}' saved successfully.", file.string());
}

QuitAction::QuitAction() = default;
QuitAction::~QuitAction() = default;
void QuitAction::onExecute() {
	log_trace("Quit action executed.");
	Application::get().requestClose();
}

}// namespace evl::gui::actions
