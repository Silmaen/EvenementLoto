/**
* @file BaseDialog.cpp
* @author Silmaen
* @date 14/11/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "pch.h"

#include "BaseDialog.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

namespace evl::gui::dialog {

std::filesystem::path openFile(const FileTypes& type, const bool exist) {
	QSettings settings;
	QString lastDir = settings.value("dialog/last_dir", "").toString();
	if (lastDir.isEmpty()) {
		lastDir = settings.value("path/data_path", QString::fromStdString(g_baseExecPath.string())).toString();
	}
	QFileDialog dia;
	if (exist) {
		dia.setAcceptMode(QFileDialog::AcceptOpen);
		dia.setFileMode(QFileDialog::ExistingFile);
		if (type == FileTypes::Folder)
			dia.setFileMode(QFileDialog::Directory);
	} else {
		dia.setAcceptMode(QFileDialog::AcceptSave);
		dia.setFileMode(QFileDialog::AnyFile);
	}
	dia.setDirectory(lastDir);
	if (type == FileTypes::EventSave)
		dia.setNameFilter("fichier événement (*.lev)");
	else if (type == FileTypes::JSON)
		dia.setNameFilter("fichier json de partie (*.json)");
	else if (type == FileTypes::Text)
		dia.setNameFilters({"fichier texte brut (*.txt)", "fichier markdown (*.md)"});
	else if (type == FileTypes::ThemeFile)
		dia.setNameFilters({"fichier theme (*.lth)", "fichier json (*.json)"});
	else if (type == FileTypes::Images)
		dia.setNameFilters({
				"Toutes les images supportées (*.png *.jpg *.jpeg *.bmp *.svg)",
				"Fichiers portable network graphics (*.png)",
				"Fichiers join photographic expert group (*.jpg, *.jpeg)",
				"Fichiers bitmap (*.bmp)",
				"Fichiers scalar vector Graphic (*.svg)",
		});
	if (dia.exec()) {
		const QString selectedFile = dia.selectedFiles()[0];
		settings.setValue("dialog/last_dir", QFileInfo(selectedFile).absolutePath());
		return std::filesystem::path{dia.selectedFiles()[0].toStdString()};
	}
	return std::filesystem::path{};
}

bool question(const QString& title, const QString& question, const QString& add) {
	QMessageBox message;
	message.setIcon(QMessageBox::Question);
	message.setWindowTitle(title);
	message.setText(question);
	message.setInformativeText(add);
	message.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
	if (message.exec() == QMessageBox::StandardButton::Yes)
		return true;
	return false;
}

QColor colorSelection(const QColor& color) {
	QColorDialog box;
	box.setCurrentColor(color);
	if (box.exec() == QColorDialog::Accepted)
		return box.selectedColor();
	return color;
}

}// namespace evl::gui::dialog
