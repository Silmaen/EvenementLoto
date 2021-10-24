/**
* @file test_About.cpp
* @author Silmaen
* @date 24/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/

#include "gui/About.h"
#include <QApplication>
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_About, base) {
    char arg0[] = "test";
    char* argv[]= {&arg0[0]};
    int argc    = 1;
    QApplication aa(argc, argv);
    About a;
    EXPECT_TRUE(a.isEnabled());
}
