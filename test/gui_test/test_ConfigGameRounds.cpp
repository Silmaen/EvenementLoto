/**
 * @file test_ConfigGameRounds.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/ConfigGameRounds.h"
#include "test_GuiHelpers.h"
#include "ui/ui_ConfigGameRounds.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_ConfigGameRounds, base) {
    test::initialize();
    ConfigGameRounds gr;
    EXPECT_STREQ(gr.getEvent().getName().c_str(), "");
    gr.setEvent(gr.getEvent());
    test::finalize();
}

TEST(gui_ConfigGameRounds, saveFile) {
    test::initialize();
    ConfigGameRounds gr;
    evl::core::Event evt;
    evt.setName("toto");
    evt.setOrganizerName("bob");
    evl::core::GameRound grd;
    evt.pushGameRound(grd);// add invalid game round
    gr.setEvent(evt);
    gr.actCancel();
    gr.actApply();
    gr.actOk();
    test::finalize();
}
