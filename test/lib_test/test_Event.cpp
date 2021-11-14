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

TEST(Event, DefaultDefines) {
    Event evt;
    EXPECT_EQ(evt.sizeRounds(), 0);
    EXPECT_EQ(evt.getEnding(), epoch);
    EXPECT_EQ(evt.getStarting(), epoch);
    EXPECT_EQ(evt.getStatus(), Event::Status::Invalid);
    EXPECT_STREQ(evt.getName().c_str(), "");
    EXPECT_STREQ(evt.getOrganizerName().c_str(), "");
    EXPECT_STREQ(evt.getLocation().c_str(), "");
    EXPECT_STREQ(evt.getLogo().string().c_str(), "");
    EXPECT_STREQ(evt.getOrganizerLogo().string().c_str(), "");
    evt.pushGameRound(GameRound());
    EXPECT_EQ(evt.sizeRounds(), 1);
    evt.startEvent();
    EXPECT_EQ(evt.getStatus(), Event::Status::Invalid);
}

TEST(Event, BaseDefines) {
    Event evt;
    evt.setName("toto");
    evt.setOrganizerName("toto tata");
    evt.setOrganizerLogo("toto.png");
    evt.setLocation("toto toto");
    evt.setLogo("toto.jpg");
    EXPECT_STREQ(evt.getName().c_str(), "toto");
    EXPECT_STREQ(evt.getOrganizerName().c_str(), "toto tata");
    EXPECT_STREQ(evt.getLocation().c_str(), "toto toto");
    EXPECT_STREQ(evt.getLogo().string().c_str(), "toto.jpg");
    EXPECT_STREQ(evt.getOrganizerLogo().string().c_str(), "toto.png");
    EXPECT_EQ(evt.getStatus(), Event::Status::MissingParties);
    evt.pushGameRound(GameRound());
    EXPECT_EQ(evt.sizeRounds(), 1);
    EXPECT_EQ(evt.getStatus(), Event::Status::Ready);
}

TEST(Event, DefinesAfterStart) {
    Event evt;
    evt.setName("toto");
    evt.setOrganizerName("toto tata");
    evt.pushGameRound(GameRound());
    evt.pauseEvent();
    evt.resumeEvent();
    evt.startCurrentRound();
    evt.closeCurrentRound();
    evt.startEvent();
    evt.swapRoundByIndex(0, 0);
    evt.pushGameRound(GameRound());
    evt.deleteRoundByIndex(0);
    evt.addWinnerToCurrentRound(156);
    EXPECT_EQ(evt.sizeRounds(), 1);
    EXPECT_EQ(evt.getCurrentIndex(), 0);
}

TEST(Event, RoundManipulation) {
    Event evt;
    evt.ActiveFirstRound();
    evt.setName("toto");
    evt.setOrganizerName("toto tata");
    evt.pushGameRound(GameRound());
    evt.pushGameRound(GameRound(GameRound::Type::Enfant));
    evt.pushGameRound(GameRound(GameRound::Type::Inverse));
    evt.deleteRoundByIndex(1);
    evt.pushGameRound(GameRound(GameRound::Type::Enfant));
    evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
    evt.swapRoundByIndex(2, 3);
    EXPECT_EQ(evt.getGameRound(3)->getType(), GameRound::Type::Enfant);
}

TEST(Event, Workflow) {
    Event evt;
    evt.setName("toto");
    evt.setOrganizerName("toto tata");
    evt.pushGameRound(GameRound(GameRound::Type::Enfant));
    evt.pushGameRound(GameRound(GameRound::Type::Enfant));
    EXPECT_EQ(evt.getStatus(), Event::Status::Ready);
    evt.startEvent();
    EXPECT_EQ(evt.getStatus(), Event::Status::EventStarted);
    evt.ActiveFirstRound();
    EXPECT_EQ(evt.getStatus(), Event::Status::GameStart);
    evt.startCurrentRound();
    EXPECT_EQ(evt.getStatus(), Event::Status::GameRunning);
    evt.addWinnerToCurrentRound(153);
    EXPECT_EQ(evt.getStatus(), Event::Status::GameFinished);
    evt.closeCurrentRound();
    EXPECT_EQ(evt.getStatus(), Event::Status::GameStart);
    evt.startCurrentRound();
    EXPECT_EQ(evt.getStatus(), Event::Status::GameRunning);
    evt.addWinnerToCurrentRound(152);
    EXPECT_EQ(evt.getStatus(), Event::Status::GameFinished);
    evt.closeCurrentRound();
    EXPECT_EQ(evt.getStatus(), Event::Status::Finished);
}

TEST(Event, WorkflowPauseRound) {
    Event evt;
    evt.setName("toto");
    evt.setOrganizerName("toto tata");
    evt.pushGameRound(GameRound(GameRound::Type::Enfant));
    evt.pushGameRound(GameRound(GameRound::Type::Enfant));
    EXPECT_EQ(evt.getStatus(), Event::Status::Ready);
    evt.startEvent();
    evt.ActiveFirstRound();
    evt.startCurrentRound();
    evt.pauseEvent();
    EXPECT_EQ(evt.getStatus(), Event::Status::Paused);
    evt.resumeEvent();
    evt.addWinnerToCurrentRound(153);
    evt.pauseEvent();
    EXPECT_EQ(evt.getStatus(), Event::Status::Paused);
    evt.resumeEvent();
    evt.closeCurrentRound();
    evt.startCurrentRound();
    evt.addWinnerToCurrentRound(152);
    evt.closeCurrentRound();
    EXPECT_EQ(evt.findFirstNotFinished(), evt.endRounds());
}

TEST(Event, Serialize) {
    Event evt;
    evt.setName("toto");
    evt.setOrganizerName("toto tata");
    evt.setOrganizerLogo("toto.png");
    evt.setLocation("toto toto");
    evt.setLogo("toto.jpg");
    evt.pushGameRound(GameRound());

    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "testGameRound.sdeg";

    std::ofstream fileSave;
    fileSave.open(file, std::ios::out | std::ios::binary);
    evt.write(fileSave);
    fileSave.close();

    Event evt2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    evt2.read(fileRead);
    fileRead.close();

    EXPECT_EQ(evt2.getName(), evt.getName());
    fs::remove_all(tmp);
}

TEST(Event, JSONSerialize) {
    Event evt;
    evt.setName("toto");
    evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
    evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
    auto round= evt.getGameRound(0);
    auto sub  = round->getSubRound(0);
    sub->define(sub->getType(), "Un canard en plastique\ndes chaussettes sales");
    sub= round->getSubRound(1);
    sub->define(sub->getType(), "un pistolet à eau\nun saucisson");
    sub= round->getSubRound(2);
    sub->define(sub->getType(), "un vibromasseur\ndes piles");
    round= evt.getGameRound(1);
    sub  = round->getSubRound(0);
    sub->define(sub->getType(), "Un bob ricard\nun verre à ballon");
    sub= round->getSubRound(1);
    sub->define(sub->getType(), "un bon pour un tour à l’urinoir\nun colonel");
    sub= round->getSubRound(2);
    sub->define(sub->getType(), "un massage vibrant\nune queue de pie");

    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "testGameRound.sdeg";

    evt.exportJSON(file);

    Event evt2;
    evt2.importJSON(file);

    EXPECT_STREQ(evt2.getGameRound(1)->getSubRound(1)->getPrices().c_str(), "un bon pour un tour à l’urinoir\nun colonel");
    fs::remove_all(tmp);
}
