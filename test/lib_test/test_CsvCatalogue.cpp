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
