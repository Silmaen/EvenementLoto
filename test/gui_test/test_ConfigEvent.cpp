/**
 * @file test_ConfigEvent.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/ConfigEvent.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_ConfigEvent, base) {
    test::initialize();
    ConfigEvent a{};
    EXPECT_STREQ(a.getEvent().getName().c_str(), "");
    a.setEvent(a.getEvent());
    test::finalize();
}

TEST(gui_ConfigEvent, ControlButtons) {
    test::initialize();
    ConfigEvent a{};
    evl::core::Event evt;
    evt.setName("toto");
    evt.setOrganizerName("titi");
    evt.setLocation("ici");
    evt.setLogo("ttt");
    evt.setOrganizerLogo("ttt");
    evl::core::GameRound gr;
    gr.setType(evl::core::GameRound::Type::FullCard);
    evt.pushGameRound(gr);
    a.setEvent(evt);
    a.actOk();
    evt.startEvent();
    a.setEvent(evt);
    a.actOk();
    a.actApply();
    a.actCancel();
    EXPECT_STREQ(a.getEvent().getName().c_str(), "toto");
    test::finalize();
}
