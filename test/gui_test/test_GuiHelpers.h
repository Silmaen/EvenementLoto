/**
 * @file test_GuiHelpers.h
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include <QApplication>

namespace evl::gui::test {

static QApplication* aa;

inline void finalize() {
    aa->quit();
    delete aa;
    aa= nullptr;
}

inline void initialize() {
    if(aa != nullptr)
        finalize();
    char arg0[] = "test";
    char* argv[]= {&arg0[0]};
    int argc    = 1;
    aa          = new QApplication(argc, argv);
}

}// namespace evl::gui::test
