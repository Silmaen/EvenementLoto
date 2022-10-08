/**
* @file BaseDialog.cpp
* @author Silmaen
* @date 14/11/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "BaseDialog.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>

namespace evl::gui::dialog {

path openFile(const FileTypes& type, bool exist) {
    QFileDialog dia;
    if(exist) {
        dia.setAcceptMode(QFileDialog::AcceptOpen);
        dia.setFileMode(QFileDialog::ExistingFile);
        if(type == FileTypes::Folder)
            dia.setFileMode(QFileDialog::Directory);
    } else {
        dia.setAcceptMode(QFileDialog::AcceptSave);
        dia.setFileMode(QFileDialog::AnyFile);
    }
    dia.setDirectory(QString::fromStdString(baseExecPath.string()));
    if(type == FileTypes::EventSave)
        dia.setNameFilter("fichier événement (*.lev)");
    else if(type == FileTypes::JSON)
        dia.setNameFilter("fichier json de partie (*.json)");
    else if(type == FileTypes::Text)
        dia.setNameFilters({"fichier texte brut (*.txt)",
                            "fichier markdown (*.md)"});
    else if(type == FileTypes::ThemeFile)
        dia.setNameFilters({"fichier theme (*.lth)",
                            "fichier json (*.json)"});
    else if(type == FileTypes::Images)
        dia.setNameFilters({
                "Toutes les images supportées (*.png *.jpg *.jpeg *.bmp *.svg)",
                "Fichiers portable network graphics (*.png)",
                "Fichiers join photographic expert group (*.jpg, *.jpeg)",
                "Fichiers bitmap (*.bmp)",
                "Fichiers scalar vector Graphic (*.svg)",
        });
    if(dia.exec()) {
        return path{dia.selectedFiles()[0].toStdString()};
    }
    return path{};
}

bool question(const QString& title, const QString& question, const QString& add) {
    QMessageBox message;
    message.setIcon(QMessageBox::Question);
    message.setWindowTitle(title);
    message.setText(question);
    message.setInformativeText(add);
    message.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
    if(message.exec() == QMessageBox::StandardButton::Yes)
        return true;
    return false;
}

QColor colorSelection(const QColor& color) {
    QColorDialog box;
    box.setCurrentColor(color);
    if(box.exec() == QColorDialog::Accepted)
        return box.selectedColor();
    return color;
}

}// namespace evl::gui::dialog
