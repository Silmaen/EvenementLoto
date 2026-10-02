/**
* @author Silmaen
* @date 20/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "../TestMainHelper.h"

#include "core/Event.h"

#include <core/utilities.h>
#include <fstream>
#include <set>

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
	EXPECT_EQ(evt.sizeRounds(), 2);
	EXPECT_EQ(evt.getCurrentGameRoundIndex(), 0);
}

TEST(Event, ImprovisedRoundGoesAfterTheRunningOne) {
	Event evt;
	evt.setName("loto");
	evt.setOrganizerName("amicale");
	evt.pushGameRound(GameRound(GameRound::Type::OneQuine));
	evt.pushGameRound(GameRound(GameRound::Type::FullCard));
	// Anywhere is allowed while nothing has started.
	EXPECT_EQ(evt.firstInsertableIndex(), 0);
	EXPECT_EQ(evt.insertGameRound(0, GameRound(GameRound::Type::Enfant)), 0);
	EXPECT_EQ(evt.sizeRounds(), 3);
	EXPECT_EQ(evt.getGameRound(0)->getType(), GameRound::Type::Enfant);

	// Once the event is running, the first round is in play and out of reach.
	evt.nextState();
	evt.nextState();
	ASSERT_EQ(evt.getStatus(), Event::Status::GameRunning);
	ASSERT_EQ(evt.getCurrentGameRoundIndex(), 0);
	EXPECT_EQ(evt.firstInsertableIndex(), 1);
	// A position before the running round is clamped, not refused: the round is added
	// where it can be, right behind the one being played.
	EXPECT_EQ(evt.insertGameRound(0, GameRound(GameRound::Type::TwoQuines)), 1);
	EXPECT_EQ(evt.getGameRound(1)->getType(), GameRound::Type::TwoQuines);
	// And the round being played is still the one being played.
	EXPECT_EQ(evt.getCurrentGameRoundIndex(), 0);
	EXPECT_EQ(evt.getGameRound(0)->getType(), GameRound::Type::Enfant);

	// At the end of the list is always allowed.
	const auto last = static_cast<uint32_t>(evt.sizeRounds());
	EXPECT_EQ(evt.insertGameRound(last + 10, GameRound(GameRound::Type::FullCard)), last);
}

TEST(Event, ImprovisedRoundTakesTheNextFreeNumber) {
	Event evt;
	evt.setName("loto");
	evt.setOrganizerName("amicale");
	EXPECT_EQ(evt.nextFreeRoundId(), 1);
	GameRound first;
	first.setId(3);
	evt.pushGameRound(first);
	EXPECT_EQ(evt.nextFreeRoundId(), 4);
}

TEST(Event, PausesAreNumberedApartFromGames) {
	Event evt;
	evt.setName("loto");
	evt.setOrganizerName("amicale");
	GameRound first;
	first.setId(1);
	evt.pushGameRound(first);
	GameRound second;
	second.setId(2);
	evt.pushGameRound(second);
	// La première pause de l'après-midi est « Pause 1 », même après deux parties.
	EXPECT_EQ(evt.nextFreeRoundId(true), 1);
	GameRound pause{GameRound::Type::Pause};
	pause.setId(evt.nextFreeRoundId(true));
	evt.pushGameRound(pause);
	EXPECT_STREQ(evt.getGameRound(2)->getName().c_str(), "Pause 1");
	// Et les deux séries avancent chacune de leur côté.
	EXPECT_EQ(evt.nextFreeRoundId(false), 3);
	EXPECT_EQ(evt.nextFreeRoundId(true), 2);
}

TEST(Event, APrizeTypedInARoundJoinsTheCatalogue) {
	Event evt;
	evt.setName("loto");
	evt.setOrganizerName("amicale");
	GameRound round;
	round.getSubRound(0)->setPrizes({Prize{"un jambon", 45.0}});
	evt.pushGameRound(round);
	evt.pushGameRound(GameRound{GameRound::Type::Pause});
	ASSERT_TRUE(evt.getCatalogue().empty());

	// Saisi dans le réglage des parties, il n'avait aucune raison de rester invisible.
	EXPECT_EQ(evt.adoptOrphanPrizes(), 1);
	ASSERT_EQ(evt.getCatalogue().size(), 1);
	const auto id = evt.getCatalogue().front().getId();
	EXPECT_NE(id, 0U);
	// Et le lien vaut dans les deux sens : la copie en jeu porte le même identifiant.
	ASSERT_TRUE(evt.findPrizeSlot(id).has_value());
	EXPECT_EQ(evt.findPrizeSlot(id)->round, 0U);

	// Rejoué, il n'y a plus rien à adopter et rien n'est doublé.
	EXPECT_EQ(evt.adoptOrphanPrizes(), 0);
	EXPECT_EQ(evt.getCatalogue().size(), 1);
}

TEST(Event, AdoptionDoesNotReuseAnIdentifierAlreadyInPlay) {
	Event evt;
	evt.setName("loto");
	evt.setOrganizerName("amicale");
	// Un lot déjà identifié dans une manche, mais absent du catalogue : son identifiant
	// est pris et ne doit pas être redonné.
	Prize inPlay{"une tondeuse", 200.0};
	inPlay.setId(7);
	GameRound round;
	round.getSubRound(0)->setPrizes({inPlay, Prize{"un jambon", 45.0}});
	evt.pushGameRound(round);

	EXPECT_EQ(evt.adoptOrphanPrizes(), 2);
	ASSERT_EQ(evt.getCatalogue().size(), 2);
	std::set<uint32_t> ids;
	for (const auto& prize: evt.getCatalogue()) ids.insert(prize.getId());
	EXPECT_EQ(ids.size(), 2);
	EXPECT_TRUE(ids.contains(7));
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
	evt2.read(fileRead, {.version = getSaveVersion(), .wideEnums = false});
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

	ASSERT_TRUE(evt.exportJSON(file));

	Event evt2;
	ASSERT_TRUE(evt2.importJSON(file));

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

TEST(Event, ImportInvalidPaths) {
	Event evt;
	// Un échec annoncé, et non seulement une absence de plantage : c'est ce qui permet à
	// l'interface de dire à l'organisateur que son fichier n'en est pas un.
	EXPECT_FALSE(evt.importJSON("/nonexistent/path/does_not_exist.json"));
	EXPECT_EQ(evt.sizeRounds(), 0);
	EXPECT_FALSE(evt.importYaml("/nonexistent/path/does_not_exist.yaml"));
	EXPECT_EQ(evt.sizeRounds(), 0);
	// Un répertoire n'est pas un fichier.
	EXPECT_FALSE(evt.importYaml(fs::temp_directory_path()));
	EXPECT_FALSE(evt.importJSON(fs::temp_directory_path()));
}

TEST(Event, ExportInvalidPaths) {
	Event evt;
	evt.setName("toto");
	evt.setOrganizerName("tata");
	evt.pushGameRound(GameRound());
	// Un répertoire qui n'existe pas : l'échec est dit, pas subi.
	EXPECT_FALSE(evt.exportJSON("/nonexistent_dir_abc123/test.json"));
	EXPECT_FALSE(evt.exportYaml("/nonexistent_dir_abc123/test.yaml"));
}

TEST(Event, AFileThatIsNotAnEventIsRefusedNotThrown) {
	const fs::path tmp = fs::temp_directory_path() / "evl-import-test";
	create_directories(tmp);

	Event reference;
	reference.setName("intact");
	reference.setOrganizerName("amicale");
	reference.pushGameRound(GameRound());

	// Du YAML valide, mais qui ne décrit pas un événement. C'était la seule voie par
	// laquelle une exception remontait jusqu'à la boucle de rendu.
	const auto wrongShape = tmp / "autre.yml";
	{
		std::ofstream out(wrongShape);
		out << "ceci: n'est pas un evenement\nautre: 12\n";
	}
	Event evt = reference;
	EXPECT_FALSE(evt.importYaml(wrongShape));
	// Et l'événement en place n'a pas été à moitié écrasé.
	EXPECT_STREQ(evt.getName().c_str(), "intact");
	EXPECT_EQ(evt.sizeRounds(), 1);

	// Du YAML qui n'est même pas du YAML.
	const auto notYaml = tmp / "cassé.yml";
	{
		std::ofstream out(notYaml);
		out << "[[[ ceci ne ferme jamais\n\t- et mélange les indentations\n";
	}
	evt = reference;
	EXPECT_FALSE(evt.importYaml(notYaml));
	EXPECT_STREQ(evt.getName().c_str(), "intact");

	// Du JSON qui n'en est pas.
	const auto notJson = tmp / "cassé.json";
	{
		std::ofstream out(notJson);
		out << "ceci n'est pas du json {{{";
	}
	evt = reference;
	EXPECT_FALSE(evt.importJSON(notJson));
	EXPECT_STREQ(evt.getName().c_str(), "intact");

	// Du JSON valide, mais qui ne décrit pas un événement : refusé aussi, sinon il
	// effacerait le programme en place au profit de rien.
	const auto emptyJson = tmp / "vide.json";
	{
		std::ofstream out(emptyJson);
		out << "{\"nimporte\": 42}";
	}
	evt = reference;
	EXPECT_FALSE(evt.importJSON(emptyJson));
	EXPECT_EQ(evt.sizeRounds(), 1);

	// Un événement vide de parties, en revanche, est un événement : accepté.
	const auto emptyEvent = tmp / "sans-partie.yml";
	{
		std::ofstream out(emptyEvent);
		out << "rounds: []\n";
	}
	evt = reference;
	EXPECT_TRUE(evt.importYaml(emptyEvent));
	EXPECT_EQ(evt.sizeRounds(), 0);

	remove_all(tmp);
}

TEST(Event, AnIncompleteYamlRoundStillGivesAPlayableRound) {
	const fs::path tmp = fs::temp_directory_path() / "evl-import-partial";
	create_directories(tmp);
	const auto file = tmp / "partiel.yml";
	{
		// Une partie sans ses sous-parties : lue, elle doit rester jouable plutôt que de
		// donner une partie vide qui bloquerait l'après-midi.
		std::ofstream out(file);
		out << "rounds:\n  - type: partie normale\n    Id: 2\n";
	}
	ASSERT_TRUE(is_regular_file(file)) << file.string();
	Event evt;
	ASSERT_TRUE(evt.importYaml(file)) << "contenu : " << [&file]() -> std::string {
		std::ifstream in(file);
		return std::string{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
	}();
	ASSERT_EQ(evt.sizeRounds(), 1);
	EXPECT_GT(evt.getGameRound(0)->sizeSubRound(), 0);
	remove_all(tmp);
}

TEST(Event, FullWorkflowWithAutoAdvance) {
	// Test the iterative nextState when a round finishes and triggers auto-advance
	Event evt;
	evt.setName("Full test");
	evt.setOrganizerName("Org");
	GameRound gr1{GameRound::Type::OneQuine};
	gr1.getSubRound(0)->define(SubGameRound::Type::OneQuine, "prix1", 100);
	GameRound gr2{GameRound::Type::OneQuine};
	gr2.getSubRound(0)->define(SubGameRound::Type::OneQuine, "prix2", 200);
	evt.pushGameRound(gr1);
	evt.pushGameRound(gr2);
	EXPECT_EQ(evt.getStatus(), Event::Status::Ready);
	evt.nextState();// Ready -> EventStarting
	EXPECT_EQ(evt.getStatus(), Event::Status::EventStarting);
	evt.nextState();// EventStarting -> GameRunning (round 1 starts)
	EXPECT_EQ(evt.getStatus(), Event::Status::GameRunning);
	EXPECT_EQ(evt.getCurrentGameRoundIndex(), 0);
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

	ASSERT_TRUE(evt.exportYaml(file));

	Event evt2;
	ASSERT_TRUE(evt2.importYaml(file));

	EXPECT_STREQ(evt2.getGameRound(1)->getSubRound(1)->getPrices().c_str(),
				 "un bon pour un tour à l’urinoir\nun colonel");
	remove_all(tmp);
}
