/**
 * @file test_PrizePdf.cpp
 * @author Silmaen
 * @date 25/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

#include "core/Event.h"
#include "core/PrizePdf.h"

#include <fstream>

using namespace evl::core;

namespace {

auto countOf(const std::string& iText, const std::string& iNeedle) -> size_t {
	size_t count = 0;
	for (auto pos = iText.find(iNeedle); pos != std::string::npos; pos = iText.find(iNeedle, pos + 1))
		++count;
	return count;
}

/// Two rounds with prizes and a pause between them.
auto eventWithPrizes() -> Event {
	Event event;
	event.setName("Loto");
	GameRound first{GameRound::Type::OneQuineFullCard};
	first.setId(1);
	Prize jambon{"un jambon (fumé)", 45.0};
	jambon.setDonor("la charcuterie");
	first.getSubRound(0)->setPrizes({jambon});
	first.getSubRound(1)->setPrizes({Prize{"une bouteille", 12.5}});
	event.pushGameRound(first);
	event.pushGameRound(GameRound{GameRound::Type::Pause});
	GameRound second{GameRound::Type::FullCard};
	second.setId(2);
	second.getSubRound(0)->setPrizes({Prize{"un séjour à l'été", 300.0}});
	event.pushGameRound(second);
	return event;
}

}// namespace

TEST(PrizePdf, hasNothingToPrintWithoutPrizes) {
	Event event;
	event.pushGameRound(GameRound{GameRound::Type::OneQuine});
	EXPECT_TRUE(buildPrizePdf(event).empty());
}

TEST(PrizePdf, writesOnePagePerPhase) {
	const auto pdf = buildPrizePdf(eventWithPrizes());
	ASSERT_FALSE(pdf.empty());
	EXPECT_EQ(pdf.rfind("%PDF-1.4", 0), 0U);
	EXPECT_NE(pdf.find("%%EOF"), std::string::npos);
	// Three phases hold prizes; the pause and the empty ones do not make a page.
	EXPECT_EQ(countOf(pdf, "/Type /Page /"), 3U);
	EXPECT_NE(pdf.find("/Count 3"), std::string::npos);
}

TEST(PrizePdf, escapesParenthesesAndEncodesAccents) {
	const auto pdf = buildPrizePdf(eventWithPrizes());
	EXPECT_NE(pdf.find("un jambon \\(fum\xE9\\)"), std::string::npos) << "the string must be escaped";
	// « é » is a single byte in WinAnsi, not the two of UTF-8.
	EXPECT_NE(pdf.find("un s\xE9jour"), std::string::npos);
	EXPECT_EQ(pdf.find("\xC3\xA9"), std::string::npos);
}

TEST(PrizePdf, xrefPointsAtTheObjects) {
	const auto pdf = buildPrizePdf(eventWithPrizes());
	const auto startxref = pdf.rfind("startxref\n");
	ASSERT_NE(startxref, std::string::npos);
	const auto xrefOffset = static_cast<size_t>(std::stoul(pdf.substr(startxref + 10)));
	EXPECT_EQ(pdf.compare(xrefOffset, 4, "xref"), 0);
	// The first object entry gives where « 1 0 obj » starts.
	const auto firstEntry = pdf.find("0000000000 65535 f \n", xrefOffset) + 20;
	const auto offset = static_cast<size_t>(std::stoul(pdf.substr(firstEntry, 10)));
	EXPECT_EQ(pdf.compare(offset, 7, "1 0 obj"), 0);
}

TEST(PrizePdf, aLongListGoesOnToTheNextPage) {
	Event event;
	GameRound round{GameRound::Type::OneQuine};
	round.setId(1);
	std::vector<Prize> prizes;
	for (int i = 0; i < 60; ++i)
		prizes.emplace_back(std::format("lot numéro {} avec une désignation assez longue pour passer à la ligne", i), 10.0);
	round.getSubRound(0)->setPrizes(prizes);
	event.pushGameRound(round);
	const auto pdf = buildPrizePdf(event);
	EXPECT_GT(countOf(pdf, "/Type /Page /"), 1U);
}

TEST(PrizePdf, writesTheFile) {
	const auto path = std::filesystem::temp_directory_path() / "evl_prizes_test.pdf";
	std::filesystem::remove(path);
	EXPECT_TRUE(writePrizePdf(eventWithPrizes(), path));
	EXPECT_TRUE(std::filesystem::exists(path));
	std::filesystem::remove(path);
}

TEST(PrizePdf, usesOneTextSizeForTheWholeDocument) {
	Event light;
	GameRound shortRound{GameRound::Type::OneQuine};
	shortRound.setId(1);
	shortRound.getSubRound(0)->setPrizes({Prize{"un jambon", 45.0}});
	light.pushGameRound(shortRound);
	// Alone, the single prize is written at the biggest size.
	ASSERT_NE(buildPrizePdf(light).find("/F2 40.00 Tf"), std::string::npos);

	Event heavy = light;
	GameRound longRound{GameRound::Type::OneQuine};
	longRound.setId(2);
	std::vector<Prize> prizes;
	for (int i = 0; i < 12; ++i)
		prizes.emplace_back(std::format("lot numéro {}", i), 10.0);
	longRound.getSubRound(0)->setPrizes(prizes);
	heavy.pushGameRound(longRound);
	// With a fuller phase in the document, no page keeps the big size.
	EXPECT_EQ(buildPrizePdf(heavy).find("/F2 40.00 Tf"), std::string::npos);
}

TEST(PrizePdf, aChildRoundIsWrittenSetByWinner) {
	Event event;
	GameRound child{GameRound::Type::Enfant};
	child.setId(1);
	Prize first{"un jouet"};
	first.setSet(1);
	Prize second{"un livre"};
	second.setSet(2);
	child.getSubRound(0)->setPrizes({first, Prize{"un puzzle"}, second});
	event.pushGameRound(child);
	const auto pdf = buildPrizePdf(event);
	EXPECT_NE(pdf.find("(Gagnant 1)"), std::string::npos);
	EXPECT_NE(pdf.find("(Gagnant 2)"), std::string::npos);
	EXPECT_EQ(pdf.find("(Gagnant 3)"), std::string::npos);
}
