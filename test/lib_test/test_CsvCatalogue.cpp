/**
 * @file test_CsvCatalogue.cpp
 * @author Silmaen
 * @date 28/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

#include "core/CsvCatalogue.h"

#include <filesystem>
#include <fstream>

using namespace evl::core;

TEST(CsvCatalogue, aFrenchSpreadsheetExport) {
	// Ce qu'un tableur français produit : point-virgule, virgule décimale, accents.
	const auto result = parseCatalogueCsv("Désignation;Donateur;Valeur;Attrait;Enfant\n"
										  "un jambon;la charcuterie;45,50;5;non\n"
										  "un jeu de société;;15,00;3;oui\n");
	ASSERT_TRUE(result.read);
	ASSERT_EQ(result.prizes.size(), 2);
	EXPECT_STREQ(result.prizes[0].getDesignation().c_str(), "un jambon");
	EXPECT_STREQ(result.prizes[0].getDonor().c_str(), "la charcuterie");
	EXPECT_NEAR(result.prizes[0].getValue(), 45.5, 0.001);
	EXPECT_EQ(result.prizes[0].getAttractiveness(), 5);
	EXPECT_FALSE(result.prizes[0].isChildFriendly());
	EXPECT_TRUE(result.prizes[1].getDonor().empty());
	EXPECT_TRUE(result.prizes[1].isChildFriendly());
}

TEST(CsvCatalogue, commasAndTabsAlsoWork) {
	const auto comma = parseCatalogueCsv("designation,value\nune moto,1500.00\n");
	ASSERT_EQ(comma.prizes.size(), 1);
	EXPECT_NEAR(comma.prizes[0].getValue(), 1500.0, 0.001);

	const auto tabs = parseCatalogueCsv("designation\tvalue\nun casque\t120\n");
	ASSERT_EQ(tabs.prizes.size(), 1);
	EXPECT_NEAR(tabs.prizes[0].getValue(), 120.0, 0.001);
}

TEST(CsvCatalogue, columnsMayComeInAnyOrderOrBeMissing) {
	const auto result = parseCatalogueCsv("Prix;Article\n30;un panier garni\n");
	ASSERT_EQ(result.prizes.size(), 1);
	EXPECT_STREQ(result.prizes[0].getDesignation().c_str(), "un panier garni");
	EXPECT_NEAR(result.prizes[0].getValue(), 30.0, 0.001);
	// Rien d'autre n'était dit, donc les valeurs par défaut tiennent.
	EXPECT_EQ(result.prizes[0].getAttractiveness(), 0);
	EXPECT_TRUE(result.prizes[0].isChildFriendly());
}

TEST(CsvCatalogue, anUnknownColumnIsIgnoredWithoutShiftingTheOthers) {
	const auto result = parseCatalogueCsv("Désignation;Référence;Valeur\nun jambon;REF-42;45\n");
	ASSERT_EQ(result.prizes.size(), 1);
	EXPECT_STREQ(result.prizes[0].getDesignation().c_str(), "un jambon");
	EXPECT_NEAR(result.prizes[0].getValue(), 45.0, 0.001);
}

TEST(CsvCatalogue, withoutAHeaderTheUsualOrderIsAssumed) {
	// Aucun nom de colonne reconnu : la première ligne est déjà une donnée.
	const auto result = parseCatalogueCsv("un jambon;la charcuterie;45;4;non\n");
	ASSERT_EQ(result.prizes.size(), 1);
	EXPECT_STREQ(result.prizes[0].getDesignation().c_str(), "un jambon");
	EXPECT_STREQ(result.prizes[0].getDonor().c_str(), "la charcuterie");
	EXPECT_NEAR(result.prizes[0].getValue(), 45.0, 0.001);
	EXPECT_EQ(result.prizes[0].getAttractiveness(), 4);
	EXPECT_FALSE(result.prizes[0].isChildFriendly());
}

TEST(CsvCatalogue, quotesProtectTheSeparator) {
	const auto result = parseCatalogueCsv("Désignation;Valeur\n"
										  "\"un lot; et un autre\";20\n"
										  "\"un lot \"\"spécial\"\"\";10\n");
	ASSERT_EQ(result.prizes.size(), 2);
	EXPECT_STREQ(result.prizes[0].getDesignation().c_str(), "un lot; et un autre");
	EXPECT_STREQ(result.prizes[1].getDesignation().c_str(), "un lot \"spécial\"");
}

TEST(CsvCatalogue, pricesAreReadAsPeopleWriteThem) {
	const auto result = parseCatalogueCsv("Désignation;Valeur\n"
										  "avec le symbole;45,50 €\n"
										  "avec un espace;1 500,00\n"
										  "sans rien;\n");
	ASSERT_EQ(result.prizes.size(), 3);
	EXPECT_NEAR(result.prizes[0].getValue(), 45.5, 0.001);
	EXPECT_NEAR(result.prizes[1].getValue(), 1500.0, 0.001);
	EXPECT_NEAR(result.prizes[2].getValue(), 0.0, 0.001);
}

TEST(CsvCatalogue, theChildColumnTakesTheWordsAspreadsheetPutsThere) {
	const auto result = parseCatalogueCsv("Désignation;Enfant\n"
										  "a;oui\nb;non\nc;x\nd;VRAI\ne;FALSE\nf;n'importe quoi\n");
	ASSERT_EQ(result.prizes.size(), 6);
	EXPECT_TRUE(result.prizes[0].isChildFriendly());
	EXPECT_FALSE(result.prizes[1].isChildFriendly());
	EXPECT_TRUE(result.prizes[2].isChildFriendly());
	EXPECT_TRUE(result.prizes[3].isChildFriendly());
	EXPECT_FALSE(result.prizes[4].isChildFriendly());
	// Illisible : la valeur par défaut reste, plutôt que d'inventer un refus.
	EXPECT_TRUE(result.prizes[5].isChildFriendly());
}

TEST(CsvCatalogue, emptyLinesAreSkippedNotRejected) {
	// Un export de tableur finit souvent par des lignes vides ; ce n'est pas une erreur.
	const auto result = parseCatalogueCsv("Désignation;Valeur\nun jambon;45\n;\n;\n");
	ASSERT_TRUE(result.read);
	EXPECT_EQ(result.prizes.size(), 1);
	EXPECT_EQ(result.skipped, 2);
	EXPECT_NE(result.summary.find("ignorée"), std::string::npos);
}

TEST(CsvCatalogue, anEmptyFileIsReadAndSaysSo) {
	const auto result = parseCatalogueCsv("");
	EXPECT_TRUE(result.read);
	EXPECT_TRUE(result.prizes.empty());
	EXPECT_NE(result.summary.find("vide"), std::string::npos);
}

TEST(CsvCatalogue, anAttractivenessOutOfScaleIsBroughtBack) {
	const auto result = parseCatalogueCsv("Désignation;Attrait\nun lot;42\nun autre;3/5\n");
	ASSERT_EQ(result.prizes.size(), 2);
	EXPECT_EQ(result.prizes[0].getAttractiveness(), Prize::g_maxAttractiveness);
	EXPECT_EQ(result.prizes[1].getAttractiveness(), 3);
}

TEST(CsvCatalogue, aMissingFileIsReportedNotCrashed) {
	const auto result = importCatalogueCsv("/n/existe/pas.csv");
	EXPECT_FALSE(result.read);
	EXPECT_TRUE(result.prizes.empty());
	EXPECT_NE(result.summary.find("introuvable"), std::string::npos);
}

TEST(CsvCatalogue, aDirectoryIsNotAFile) {
	const auto result = importCatalogueCsv(std::filesystem::temp_directory_path());
	EXPECT_FALSE(result.read);
	EXPECT_NE(result.summary.find("introuvable"), std::string::npos);
}

TEST(CsvCatalogue, aRealFileOnDisk) {
	const auto tmp = std::filesystem::temp_directory_path() / "evl-csv-test";
	create_directories(tmp);
	const auto file = tmp / "lots.csv";
	{
		std::ofstream out(file);
		out << "Désignation;Donateur;Valeur\nun jambon;la charcuterie;45,50\n";
	}
	const auto result = importCatalogueCsv(file);
	ASSERT_TRUE(result.read);
	ASSERT_EQ(result.prizes.size(), 1);
	EXPECT_NEAR(result.prizes[0].getValue(), 45.5, 0.001);
	remove_all(tmp);
}

TEST(CsvCatalogue, binaryRubbishIsReadWithoutDamage) {
	// Un fichier qui n'est pas du texte : on n'en tire rien de sensé, mais on n'en meurt
	// pas non plus.
	std::string rubbish;
	for (int i = 0; i < 512; ++i) rubbish += static_cast<char>((i * 37) % 256);
	const auto result = parseCatalogueCsv(rubbish);
	EXPECT_TRUE(result.read);
}

TEST(CsvCatalogue, aDistributedFileKeepsItsPlacesAndItsOrder) {
	const auto result = parseCatalogueCsv("Désignation;Valeur;Partie;Phase\n"
										  "jambon;40;1;1\n"
										  "bon d'achat;100;1;3\n"
										  "téléviseur;300;5;3\n"
										  "vase;5;;\n"
										  "ailleurs;9;7;1\n");
	ASSERT_EQ(result.prizes.size(), 5);
	ASSERT_EQ(result.placements.size(), 4);
	EXPECT_EQ(result.placements[1].prize, 1);
	EXPECT_EQ(result.placements[1].round, 1);
	EXPECT_EQ(result.placements[1].phase, 3);

	// « Partie 5 » is the game numbered 5, whatever its rank, and a pause never answers.
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	GameRound first{GameRound::Type::OneTwoQuineFullCard};
	first.setId(1);
	event.pushGameRound(first);
	GameRound pause{GameRound::Type::Pause};
	pause.setId(5);
	event.pushGameRound(pause);
	GameRound fifth{GameRound::Type::OneTwoQuineFullCard};
	fifth.setId(5);
	event.pushGameRound(fifth);
	event.setCatalogue(result.prizes);
	const auto placed = placeImported(event, 0, result);
	EXPECT_EQ(placed.placed, 3);
	EXPECT_EQ(placed.refused, 1);// no game is numbered 7
	const auto second = std::next(event.beginRounds(), 2);
	ASSERT_EQ(second->getSubRound(2)->getPrizes().size(), 1);
	EXPECT_STREQ(second->getSubRound(2)->getPrizes()[0].getDesignation().c_str(), "téléviseur");
	EXPECT_EQ(event.getGameRound(0)->getSubRound(0)->getPrizes().size(), 1);
}

TEST(CsvCatalogue, linesOfAPhaseKeepTheirOrderOfAppearance) {
	const auto result = parseCatalogueCsv("Désignation;Partie;Phase\nC;1;1\nA;1;1\nB;1;1\n");
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	GameRound only{GameRound::Type::OneTwoQuineFullCard};
	only.setId(1);
	event.pushGameRound(only);
	event.setCatalogue(result.prizes);
	EXPECT_EQ(placeImported(event, 0, result).placed, 3);
	const auto& prizes = event.getGameRound(0)->getSubRound(0)->getPrizes();
	ASSERT_EQ(prizes.size(), 3);
	EXPECT_STREQ(prizes[0].getDesignation().c_str(), "C");
	EXPECT_STREQ(prizes[1].getDesignation().c_str(), "A");
	EXPECT_STREQ(prizes[2].getDesignation().c_str(), "B");
}

TEST(CsvCatalogue, emptyingTheCatalogueAlsoEmptiesTheEditablePhases) {
	const auto result = parseCatalogueCsv("Désignation;Partie;Phase\nA;1;1\nB;1;2\nC\n");
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	GameRound only{GameRound::Type::OneTwoQuineFullCard};
	only.setId(1);
	event.pushGameRound(only);
	event.setCatalogue(result.prizes);
	ASSERT_EQ(placeImported(event, 0, result).placed, 2);
	EXPECT_EQ(event.clearCatalogue(), 3);
	EXPECT_TRUE(event.getCatalogue().empty());
	EXPECT_TRUE(event.getGameRound(0)->getSubRound(0)->getPrizes().empty());
	EXPECT_TRUE(event.getGameRound(0)->getSubRound(1)->getPrizes().empty());
}

TEST(CsvCatalogue, inAChildRoundThePhaseIsTheWinnerNumber) {
	const auto result = parseCatalogueCsv("Désignation;Partie;Phase\nA;1;2\nB;1;1\nC;1;2\n");
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	GameRound child{GameRound::Type::Enfant};
	child.setId(1);
	event.pushGameRound(child);
	event.setCatalogue(result.prizes);
	EXPECT_EQ(placeImported(event, 0, result).placed, 3);
	const auto& prizes = event.getGameRound(0)->getSubRound(0)->getPrizes();
	ASSERT_EQ(prizes.size(), 3);
	EXPECT_EQ(prizes[0].getSet(), 2);
	EXPECT_EQ(prizes[1].getSet(), 1);
	// The catalogue keeps it too, so a new distribution does not lose it.
	EXPECT_EQ(event.getCatalogue()[0].getSet(), 2);
	// Grouped by winner for the announcement: B first, then A and C.
	const auto announced = announcedPrizes(prizes, true);
	ASSERT_EQ(announced.size(), 3);
	EXPECT_STREQ(announced[0].prize->getDesignation().c_str(), "B");
	EXPECT_TRUE(announced[0].firstOfSet);
	EXPECT_STREQ(announced[1].prize->getDesignation().c_str(), "A");
	EXPECT_TRUE(announced[1].firstOfSet);
	EXPECT_FALSE(announced[2].firstOfSet);
}

TEST(CsvCatalogue, aWinnerColumnGivesTheSetDirectly) {
	const auto result = parseCatalogueCsv("Désignation;Gagnant\nA;3\n");
	ASSERT_EQ(result.prizes.size(), 1);
	EXPECT_EQ(result.prizes[0].getSet(), 3);
}
