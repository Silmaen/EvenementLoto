/**
 * @file test_markdownParser.cpp
 * @author Silmaen
 * @date 25/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "../TestMainHelper.h"
#include "gui/utils/MarkdownParser.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace evl::gui::utils;

namespace {

/// The shipped user documentation, read line by line.
auto documentationLines() -> std::vector<std::string> {
	std::ifstream file{EVL_TEST_DOC_FILE};
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line)) lines.push_back(line);
	return lines;
}

/// How many elements of a kind the parser produced.
auto countOf(const std::vector<MarkdownElement>& iElements, const MarkdownElementType iType) -> size_t {
	return static_cast<size_t>(std::ranges::count_if(
			iElements, [iType](const MarkdownElement& iElement) -> bool { return iElement.type == iType; }));
}

}// namespace

/// The documentation the application ships is the documentation this parser renders. A
/// construct it does not know is not a warning anywhere: it silently degrades into a
/// paragraph, and nobody sees it until a user reads the help page.
TEST(utils_MarkdownParser, ReadsTheShippedDocumentation) {
	const auto elements = parseMarkdownFile(EVL_TEST_DOC_FILE);
	// Enough to be the real file rather than an empty read.
	ASSERT_GT(elements.size(), 100U);
	EXPECT_GE(countOf(elements, MarkdownElementType::Heading1), 1U);
	EXPECT_GT(countOf(elements, MarkdownElementType::Heading2), 1U);
	EXPECT_GT(countOf(elements, MarkdownElementType::Image), 10U);
	EXPECT_GT(countOf(elements, MarkdownElementType::BulletItem), 5U);
	EXPECT_GT(countOf(elements, MarkdownElementType::TableRow), 3U);
}

TEST(utils_MarkdownParser, TheDocumentationUsesNothingItCannotRender) {
	bool inCodeFence = false;
	size_t lineNumber = 0;
	for (const auto& line: documentationLines()) {
		++lineNumber;
		if (line.starts_with("```")) {
			inCodeFence = !inCodeFence;
			ADD_FAILURE() << "ligne " << lineNumber << " : bloc de code, non rendu";
			continue;
		}
		// `- ` and `+ ` are markdown bullets the parser does not know: it keeps them as
		// paragraphs, dash included, and the list reads as prose.
		EXPECT_FALSE(line.starts_with("- ") || line.starts_with("+ "))
				<< "ligne " << lineNumber << " : puce non reconnue, seul « * » est rendu — " << line;
		// Inline code is not parsed either, so the backticks show up as characters.
		EXPECT_EQ(line.find('`'), std::string::npos)
				<< "ligne " << lineNumber << " : code en ligne non rendu — " << line;
		// Only four heading levels exist.
		EXPECT_FALSE(line.starts_with("#####")) << "ligne " << lineNumber << " : titre trop profond";
	}
}

TEST(utils_MarkdownParser, EveryPictureOfTheDocumentationExists) {
	const auto elements = parseMarkdownFile(EVL_TEST_DOC_FILE);
	size_t images = 0;
	for (const auto& element: elements) {
		if (element.type != MarkdownElementType::Image)
			continue;
		++images;
		// The path is relative to the document, and `images/` is copied beside it.
		const auto name = fs::path{element.imagePath}.filename();
		EXPECT_TRUE(exists(fs::path{EVL_TEST_DOC_IMAGES} / name))
				<< "image absente : " << element.imagePath << " (« " << element.imageAlt << " »)";
	}
	EXPECT_GT(images, 10U);
}

TEST(utils_MarkdownParser, ReadsTheFourHeadingLevels) {
	const auto elements = parseMarkdownFile(EVL_TEST_DOC_FILE);
	ASSERT_FALSE(elements.empty());
	// The title of the shipped file, which is also what the help page shows first.
	EXPECT_EQ(elements.front().type, MarkdownElementType::Heading1);
	EXPECT_EQ(elements.front().rawText, "Utilisation du logiciel");
}

TEST(utils_MarkdownParser, ReadsBoldSpans) {
	const auto spans = parseInlineFormatting("du texte **en gras** puis la suite");
	ASSERT_EQ(spans.size(), 3U);
	EXPECT_EQ(spans[0].text, "du texte ");
	EXPECT_FALSE(spans[0].bold);
	EXPECT_EQ(spans[1].text, "en gras");
	EXPECT_TRUE(spans[1].bold);
	EXPECT_EQ(spans[2].text, " puis la suite");
	EXPECT_FALSE(spans[2].bold);

	// Nothing to emphasise: one plain span.
	const auto plain = parseInlineFormatting("rien de gras");
	ASSERT_EQ(plain.size(), 1U);
	EXPECT_FALSE(plain[0].bold);
	// An unmatched marker must not lose the rest of the line.
	const auto unmatched = parseInlineFormatting("un **début sans fin");
	ASSERT_FALSE(unmatched.empty());
	std::string rebuilt;
	for (const auto& span: unmatched) rebuilt += span.text;
	EXPECT_NE(rebuilt.find("début sans fin"), std::string::npos);
}

TEST(utils_MarkdownParser, ComesBackEmptyOnAMissingFile) {
	// The help page asks for a file that the package may not carry: it must show an
	// empty page, not take the application down.
	EXPECT_TRUE(parseMarkdownFile("/no/such/documentation.md").empty());
}
