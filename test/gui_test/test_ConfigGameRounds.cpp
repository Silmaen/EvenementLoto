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
    grd.setType(evl::core::GameRound::Type::None);
    evt.pushGameRound(grd);// add invalid game round
    gr.setEvent(evt);
    gr.actCancel();
    gr.actApply();
    gr.actOk();
    test::finalize();
}

TEST(gui_ConfigGameRounds, loadFullValidEvent) {
    test::initialize();
    ConfigGameRounds gr;
    evl::core::Event evt;
    evt.setName("toto");
    evt.setOrganizerName("bob");
    evl::core::GameRound grd;
    grd.setType(evl::core::GameRound::Type::Inverse);
    evt.pushGameRound(grd);
    gr.setEvent(evt);
    grd.setType(evl::core::GameRound::Type::TwoQuines);
    evt.pushGameRound(grd);// add invalid game round
    gr.setEvent(evt);
    grd.setType(evl::core::GameRound::Type::OneQuine);
    evt.pushGameRound(grd);// add invalid game round
    gr.setEvent(evt);
    grd.setType(evl::core::GameRound::Type::FullCard);
    evt.pushGameRound(grd);// add invalid game round
    gr.setEvent(evt);
    gr.actCreateGameRound();
    gr.actSaveGameRound();
    gr.actOk();
    test::finalize();
}

TEST(gui_ConfigGameRounds, loadFinishedEvent) {
    test::initialize();
    ConfigGameRounds gr;
    evl::core::Event evt;
    evt.setName("toto");
    evt.setOrganizerName("bob");
    evl::core::GameRound grd;
    grd.setType(evl::core::GameRound::Type::Inverse);
    evt.pushGameRound(grd);
    grd.setType(evl::core::GameRound::Type::TwoQuines);
    evt.pushGameRound(grd);
    evt.startEvent();
    evt.startCurrentRound();
    evt.findFirstNotFinished()->addPickedNumber(15);
    evt.endCurrentRound();
    evt.closeCurrentRound();
    gr.setEvent(evt);
    gr.actOk();
    test::finalize();
}
