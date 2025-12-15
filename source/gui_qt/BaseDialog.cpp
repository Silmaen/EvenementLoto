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

auto openFile(const FileTypes& iType, const bool iExist) -> std::filesystem::path {
	QSettings settings;
	QString lastDir = settings.value("dialog/last_dir", "").toString();
	if (lastDir.isEmpty()) {
		lastDir = settings.value("path/data_path", QString::fromStdString(core::getExecPath().string())).toString();
	}
	QFileDialog dia;
	if (iExist) {
		dia.setAcceptMode(QFileDialog::AcceptOpen);
		dia.setFileMode(QFileDialog::ExistingFile);
		if (iType == FileTypes::Folder)
			dia.setFileMode(QFileDialog::Directory);
	} else {
		dia.setAcceptMode(QFileDialog::AcceptSave);
		dia.setFileMode(QFileDialog::AnyFile);
	}
	dia.setDirectory(lastDir);
	if (iType == FileTypes::EventSave)
		dia.setNameFilter("fichier événement (*.lev)");
	else if (iType == FileTypes::JSON)
		dia.setNameFilter("fichier json de partie (*.json)");
	else if (iType == FileTypes::Text)
		dia.setNameFilters({"fichier texte brut (*.txt)", "fichier markdown (*.md)"});
	else if (iType == FileTypes::ThemeFile)
		dia.setNameFilters({"fichier theme (*.lth)", "fichier json (*.json)"});
	else if (iType == FileTypes::Images)
		dia.setNameFilters({
				"Toutes les images supportées (*.png *.jpg *.jpeg *.bmp *.svg)",
				"Fichiers portable network graphics (*.png)",
				"Fichiers join photographic expert group (*.jpg, *.jpeg)",
				"Fichiers bitmap (*.bmp)",
				"Fichiers scalar vector Graphic (*.svg)",
		});
	if (dia.exec() == QDialog::Accepted) {
		const QString selectedFile = dia.selectedFiles()[0];
		settings.setValue("dialog/last_dir", QFileInfo(selectedFile).absolutePath());
		return std::filesystem::path{dia.selectedFiles()[0].toStdString()};
	}
	return std::filesystem::path{};
}

auto question(const QString& iTitle, const QString& iQuestion, const QString& iAdd) -> bool {
	QMessageBox message;
	message.setIcon(QMessageBox::Question);
	message.setWindowTitle(iTitle);
	message.setText(iQuestion);
	message.setInformativeText(iAdd);
	message.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
	return message.exec() == QMessageBox::StandardButton::Yes;
}

auto colorSelection(const QColor& iColor) -> QColor {
	QColorDialog box;
	box.setCurrentColor(iColor);
	if (box.exec() == QColorDialog::Accepted)
		return box.selectedColor();
	return iColor;
}

}// namespace evl::gui::dialog
