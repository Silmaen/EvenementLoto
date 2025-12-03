/**
* @author Silmaen
* @date 20/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "../TestMainHelper.h"

#include "core/Event.h"

#include <fstream>

using namespace evl::core;

TEST(Event, DefaultDefines) {
	Event evt;
	EXPECT_EQ(evt.sizeRounds(), 0);
	EXPECT_EQ(evt.getEnding(), g_epoch);
	EXPECT_EQ(evt.getStarting(), g_epoch);
	EXPECT_EQ(evt.getStatus(), Event::Status::Invalid);
	EXPECT_STREQ(evt.getName().c_str(), "");
	EXPECT_STREQ(evt.getOrganizerName().c_str(), "");
	EXPECT_STREQ(evt.getLocation().c_str(), "");
	EXPECT_STREQ(evt.getLogo().string().c_str(), "");
	EXPECT_STREQ(evt.getOrganizerLogo().string().c_str(), "");
	evt.pushGameRound(GameRound());
	EXPECT_EQ(evt.sizeRounds(), 1);
	evt.nextState();
	EXPECT_EQ(evt.getStatus(), Event::Status::Invalid);
#ifdef EVL_DEBUG
	evt.invalidStatus();
	EXPECT_STREQ(evt.getStatusStr().c_str(), "invalide");
	evt.restoreStatusDbg();
#endif
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
	evt.nextState();
	evt.nextState();
	evt.nextState();
	evt.nextState();
	evt.nextState();
	evt.swapRoundByIndex(0, 0);
	evt.pushGameRound(GameRound());
	evt.deleteRoundByIndex(0);
	evt.addWinnerToCurrentRound("156");
	EXPECT_EQ(evt.sizeRounds(), 1);
	EXPECT_EQ(evt.getCurrentGameRoundIndex(), 0);
}

TEST(Event, RoundManipulation) {
	Event evt;
	evt.nextState();
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

TEST(Event, displayScreens) {
	Event evt;
	evt.setName("toto");
	evt.setOrganizerName("toto tata");
	evt.pushGameRound(GameRound(GameRound::Type::Enfant));
	evt.pushGameRound(GameRound(GameRound::Type::Enfant));
	evt.displayRules();
	EXPECT_EQ(evt.getStatus(), Event::Status::Ready);
	evt.nextState();
	evt.displayRules();
	EXPECT_EQ(evt.getStatus(), Event::Status::DisplayRules);
	evt.nextState();
	EXPECT_EQ(evt.getStatus(), Event::Status::GameRunning);
}

TEST(Event, Workflow) {
	Event evt;
	evt.setName("toto");
	evt.setOrganizerName("toto tata");
	evt.pushGameRound(GameRound(GameRound::Type::Enfant));
	evt.pushGameRound(GameRound(GameRound::Type::Enfant));
	EXPECT_EQ(evt.getStatus(), Event::Status::Ready);
	evt.nextState();
	EXPECT_EQ(evt.getStatus(), Event::Status::EventStarting);
	evt.nextState();
	EXPECT_EQ(evt.getStatus(), Event::Status::GameRunning);
	evt.addWinnerToCurrentRound("153");
	evt.nextState();
	EXPECT_TRUE(evt.checkStateChanged());
	EXPECT_FALSE(evt.checkStateChanged());
	EXPECT_EQ(evt.getStatus(), Event::Status::GameRunning);
	evt.addWinnerToCurrentRound("152");
	evt.nextState();
	EXPECT_EQ(evt.getStatus(), Event::Status::EventEnding);
	evt.nextState();
	EXPECT_EQ(evt.getStatus(), Event::Status::Finished);
}

TEST(Event, Serialize) {
	Event evt;
	evt.setName("toto");
	evt.setOrganizerName("toto tata");
	evt.setOrganizerLogo("toto.png");
	evt.setLocation("toto toto");
	evt.setLogo("toto.jpg");
	evt.pushGameRound(GameRound());

	const fs::path tmp = fs::temp_directory_path() / "test";
	create_directories(tmp);
	const fs::path file = tmp / "testGameRound.sdeg";

	std::ofstream fileSave;
	fileSave.open(file, std::ios::out | std::ios::binary);
	evt.write(fileSave);
	fileSave.close();

	Event evt2;
	std::ifstream fileRead;
	fileRead.open(file, std::ios::in | std::ios::binary);
	evt2.read(fileRead, evl::g_currentSaveVersion);
	fileRead.close();

	EXPECT_EQ(evt2.getName(), evt.getName());
	remove_all(tmp);
}

TEST(Event, JSONSerialize) {
	Event evt;
	evt.setName("toto");
	evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
	evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
	auto round = evt.getGameRound(0);
	auto sub = round->getSubRound(0);
	sub->define(sub->getType(), "Un canard en plastique\ndes chaussettes sales");
	sub = round->getSubRound(1);
	sub->define(sub->getType(), "un pistolet à eau\nun saucisson");
	sub = round->getSubRound(2);
	sub->define(sub->getType(), "un vibromasseur\ndes piles");
	round = evt.getGameRound(1);
	sub = round->getSubRound(0);
	sub->define(sub->getType(), "Un bob ricard\nun verre à ballon");
	sub = round->getSubRound(1);
	sub->define(sub->getType(), "un bon pour un tour à l’urinoir\nun colonel");
	sub = round->getSubRound(2);
	sub->define(sub->getType(), "un massage vibrant\nune queue de pie");

	const fs::path tmp = fs::temp_directory_path() / "test";
	create_directories(tmp);
	const fs::path file = tmp / "testGameRound.sdeg";

	evt.exportJSON(file);

	Event evt2;
	evt2.importJSON(file);

	EXPECT_STREQ(evt2.getGameRound(1)->getSubRound(1)->getPrices().c_str(),
				 "un bon pour un tour à l’urinoir\nun colonel");
	remove_all(tmp);
}

TEST(Event, basePath) {
	Event evt;
	evt.setName("toto");
	evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
	evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
	evt.setBasePath("");
	evt.setLogo("");
	evt.setOrganizerLogo("");
	EXPECT_STREQ(evt.getLogo().string().c_str(), "");
}

TEST(Event, YamlSerialize) {
	Event evt;
	evt.setName("toto");
	evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
	evt.pushGameRound(GameRound(GameRound::Type::OneTwoQuineFullCard));
	auto round = evt.getGameRound(0);
	auto sub = round->getSubRound(0);
	sub->define(sub->getType(), "Un canard en plastique\ndes chaussettes sales");
	sub = round->getSubRound(1);
	sub->define(sub->getType(), "un pistolet à eau\nun saucisson");
	sub = round->getSubRound(2);
	sub->define(sub->getType(), "un vibromasseur\ndes piles");
	round = evt.getGameRound(1);
	sub = round->getSubRound(0);
	sub->define(sub->getType(), "Un bob ricard\nun verre à ballon");
	sub = round->getSubRound(1);
	sub->define(sub->getType(), "un bon pour un tour à l’urinoir\nun colonel");
	sub = round->getSubRound(2);
	sub->define(sub->getType(), "un massage vibrant\nune queue de pie");

	const fs::path tmp = fs::temp_directory_path() / "test";
	create_directories(tmp);
	const fs::path file = tmp / "testGameRound.sdeg";

	evt.exportYaml(file);

	Event evt2;
	evt2.importYaml(file);

	EXPECT_STREQ(evt2.getGameRound(1)->getSubRound(1)->getPrices().c_str(),
				 "un bon pour un tour à l’urinoir\nun colonel");
	remove_all(tmp);
}
