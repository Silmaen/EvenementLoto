/**
 * @file test_DisplayWindow.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/DisplayWindow.h"
#include "gui/baseDefinitions.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>
#include <ui/ui_DisplayWindow.h>

using namespace evl::gui;

TEST(gui_DisplayWindow, base) {
    test::initialize();
    evl::core::Event evl;
    DisplayWindow a{nullptr};
    a.updateDisplay();
    EXPECT_EQ(a.getUi()->PageManager->currentIndex(), 0);
    test::finalize();
}

TEST(gui_DisplayWindow, updateDisplay_minimal_eventTitle) {
    test::initialize();
    evl::core::Event evl;
    DisplayWindow a{&evl};
    evl.setName("toto");
    a.updateDisplay();
    EXPECT_STREQ(a.getUi()->ET_EventTitle->text().toStdString().c_str(), "toto");
    test::finalize();
}

TEST(gui_DisplayWindow, updateDisplay_eventTitle_falselogo) {
    test::initialize();
    evl::core::Event evl;
    DisplayWindow a{&evl};
    evl.setName("toto");
    evl.setOrganizerName("titi");
    evl.setLocation("ici");
    evl.setLogo("t");
    evl.setOrganizerLogo("r");
    a.updateDisplay();
    EXPECT_STREQ(a.getUi()->ET_OrganizerName->text().toStdString().c_str(), "titi");
    EXPECT_STREQ(a.getUi()->ET_EventLocation->text().toStdString().c_str(), "ici");
    test::finalize();
}

TEST(gui_DisplayWindow, updateDisplay_eventTitle_logo) {
    test::initialize();
    evl::core::Event evl;
    DisplayWindow a{&evl};
    evl.setName("toto");
    evl.setOrganizerName("titi");
    evl.setLocation("ici");
    std::filesystem::path dataPath= std::filesystem::current_path().parent_path().parent_path() / "data";
    evl.setLogo(dataPath / "poule.png");
    evl.setOrganizerLogo(dataPath / "poule.png");
    a.updateDisplay();
    EXPECT_FALSE(a.getUi()->ET_EventLogo->pixmap().isNull());
    EXPECT_FALSE(a.getUi()->ET_OrganizerLogo->pixmap().isNull());
    test::finalize();
}

TEST(gui_DisplayWindow, updateDisplay_different_pages) {
    /*
    test::initialize();
    evl::core::Event evl;
    evl.setName("toto");
    evl.setOrganizerName("titi");
    evl::core::GameRound gr;
    gr.setType(evl::core::GameRound::Type::Inverse);
    evl.pushGameRound(gr);
    evl.pushGameRound(gr);
    DisplayWindow a{&evl};
    evl.startEvent();
    evl.resumeEvent();
    a.updateDisplay();// page titre round
    EXPECT_EQ(a.getUi()->PageManager->currentIndex(), 1);
    evl.startCurrentRound();
    a.updateDisplay();// page round en cours
    EXPECT_EQ(a.getUi()->PageManager->currentIndex(), 2);
    evl.pauseEvent();
    a.updateDisplay();// page round en pause
    EXPECT_EQ(a.getUi()->PageManager->currentIndex(), 3);
    evl.resumeEvent();
    evl.endCurrentRound();
    a.updateDisplay();// page fin de round
    EXPECT_EQ(a.getUi()->PageManager->currentIndex(), 4);
    evl.pauseEvent();
    a.updateDisplay();// page événement en pause
    EXPECT_EQ(a.getUi()->PageManager->currentIndex(), 5);
    evl.resumeEvent();
    evl.closeCurrentRound();
    evl.startCurrentRound();
    evl.endCurrentRound();
    evl.closeCurrentRound();
    a.updateDisplay();// page fin événement
    EXPECT_EQ(a.getUi()->PageManager->currentIndex(), 6);
    test::finalize();
     */
}
