/**
* @author Silmaen
* @date 20/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "core/Event.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
namespace fs= std::filesystem;

using namespace evl::core;

TEST(Event, BaseDefines) {
    /*
    Event event;
    event.setOrganizerName("toto");
    event.setName("evenement de toto");
    event.setLocation("quelque part");
    event.setLogo("toto.png");
    event.setOrganizerLogo("toto.png");
    EXPECT_STREQ(event.getOrganizerName().c_str(), "toto");
    EXPECT_STREQ(event.getOrganizerLogo().generic_string().c_str(), "toto.png");
    EXPECT_STREQ(event.getName().c_str(), "evenement de toto");
    EXPECT_STREQ(event.getLocation().c_str(), "quelque part");
    EXPECT_STREQ(event.getLogo().generic_string().c_str(), "toto.png");
    GameRound gr;
    gr.setType(GameRound::Type::Normal);
    event.pushGameRound(gr);
    event.startEvent();
    event.setOrganizerName("toto2");
    event.setName("evenement de toto2");
    event.setLocation("quelque part2");
    event.setLogo("toto2.png");
    event.setOrganizerLogo("toto2.png");
    EXPECT_STREQ(event.getOrganizerName().c_str(), "toto");
    EXPECT_STREQ(event.getOrganizerLogo().generic_string().c_str(), "toto.png");
    EXPECT_STREQ(event.getName().c_str(), "evenement de toto");
    EXPECT_STREQ(event.getLocation().c_str(), "quelque part");
    EXPECT_STREQ(event.getLogo().generic_string().c_str(), "toto.png");
     */
}

TEST(Event, DefinesAfterStart) {

}

TEST(Event, RoundManipulation) {

}

TEST(Event, BaseToReady) {

}

TEST(Event, Workflow) {

}

TEST(Event, WorkflowPauseRound) {

}

TEST(Event, WorkflowFinished) {

}

TEST(Event, Serialize) {

}
