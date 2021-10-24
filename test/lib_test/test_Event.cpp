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
    gr.setType(GameRound::Type::FullCard);
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
}

TEST(Event, DefinesAfterStart) {
    Event event;
    event.setOrganizerName("toto");
    event.setName("evenement de toto");
    event.setLocation("quelque part");
    event.setLogo("toto.png");
    event.setOrganizerLogo("toto.png");
    GameRound gr;
    event.pushGameRound(gr);
    gr.setType(GameRound::Type::FullCard);
    event.pushGameRound(gr);
    event.startEvent();
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
}

TEST(Event, BaseToReady) {
    Event event;
    EXPECT_EQ(event.getStatus(), Event::Status::Invalid);
    event.setOrganizerName("toto");
    event.setName("evenement de toto");

    EXPECT_EQ(event.getStatus(), Event::Status::MissingParties);
    GameRound gr;
    gr.setType(GameRound::Type::FullCard);
    event.pushGameRound(gr);
    EXPECT_EQ(event.getStatus(), Event::Status::Ready);
}

TEST(Event, Workflow) {
    // event definition
    Event event;
    event.setOrganizerName("toto");
    event.setName("evenement de toto");
    GameRound gr;
    gr.setType(GameRound::Type::FullCard);
    event.pushGameRound(gr);
    gr.setType(GameRound::Type::Inverse);
    event.pushGameRound(gr);
    event.startEvent();
    EXPECT_EQ(event.getStatus(), Event::Status::EventStarted);
    event.resumeEvent();
    EXPECT_EQ(event.getStatus(), Event::Status::GameStart);
    event.startCurrentRound();
    EXPECT_EQ(event.getStatus(), Event::Status::GameRunning);
    event.endCurrentRound();
    EXPECT_EQ(event.getStatus(), Event::Status::GameFinished);
    event.pauseEvent();
    EXPECT_EQ(event.getStatus(), Event::Status::Paused);
    event.resumeEvent();
    event.closeCurrentRound();
    EXPECT_EQ(event.getStatus(), Event::Status::GameStart);
    event.startCurrentRound();
    event.endCurrentRound();
    event.closeCurrentRound();
    event.stopEvent();
    EXPECT_EQ(event.getStatus(), Event::Status::Finished);
}

TEST(Event, WorkflowPauseRound) {
    // event definition
    Event event;
    event.setOrganizerName("toto");
    event.setName("evenement de toto");
    GameRound gr;
    gr.setType(GameRound::Type::FullCard);
    event.pushGameRound(gr);
    // bad actions
    event.stopEvent();
    event.pauseEvent();
    event.resumeEvent();
    event.startEvent();
    event.pushGameRound(gr);// you can't push a round after start
    EXPECT_EQ(event.getGameRounds().size(), 1);
    event.resumeEvent();
    event.startCurrentRound();
    event.pauseEvent();
    EXPECT_EQ(event.getStatus(), Event::Status::GamePaused);
    event.resumeEvent();
    EXPECT_EQ(event.getStatus(), Event::Status::GameRunning);
    event.endCurrentRound();
    event.stopEvent();
    EXPECT_EQ(event.getStatus(), Event::Status::Finished);
}

TEST(Event, WorkflowFinished) {
    // event definition
    Event event;
    event.setOrganizerName("toto");
    event.setName("evenement de toto");
    GameRound gr;
    gr.setType(GameRound::Type::FullCard);
    event.pushGameRound(gr);
    // bad actions
    event.startEvent();
    event.resumeEvent();
    event.startCurrentRound();
    event.endCurrentRound();
    event.closeCurrentRound();

    event.startCurrentRound();
    event.pauseEvent();
    event.resumeEvent();
    event.endCurrentRound();
    event.closeCurrentRound();
    event.stopEvent();
    EXPECT_EQ(event.getStatus(), Event::Status::Finished);
}

TEST(Event, Serialize) {
    Event event;
    event.setOrganizerName("toto");
    event.setName("evenement de toto");
    event.setOrganizerLogo("toto.png");
    GameRound gr;
    gr.setType(GameRound::Type::FullCard);
    event.pushGameRound(gr);

    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "testEvent.sdeg";

    std::ofstream fileSave;
    fileSave.open(file, std::ios::out | std::ios::binary);
    event.write(fileSave);
    fileSave.close();

    Event event2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    event2.read(fileRead);
    fileRead.close();

    EXPECT_STREQ(event2.getName().c_str(), "evenement de toto");
    EXPECT_STREQ(event2.getLocation().c_str(), "");
    EXPECT_EQ(event2.getGameRounds().size(), 1);
    fs::remove_all(tmp);
}
