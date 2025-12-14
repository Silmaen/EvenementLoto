/**
 * @file WinnerInput.cpp
 * @author damien.lachouette 
 * @date 14/09/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "WinnerInput.h"

// Les trucs de QT
#include "moc_WinnerInput.cpp"
#include "ui/ui_WinnerInput.h"

namespace evl::gui {

WinnerInput::WinnerInput(QWidget* parent) : QDialog(parent), ui(new Ui::WinnerInput) { ui->setupUi(this); }

WinnerInput::~WinnerInput() { delete ui; }

auto WinnerInput::getWinner() const -> QString { return ui->EditWinner->text(); }

}// namespace evl::gui
