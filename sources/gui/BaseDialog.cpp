/**
* @file baseDialog.cpp
* @author Silmaen
* @date 14/11/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "gui/BaseDialog.h"
#include <QFileDialog>

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
    else if(type == FileTypes::Images)
        dia.setNameFilters(imageFilter);
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

}// namespace evl::gui::dialog