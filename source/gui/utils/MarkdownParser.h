/**
 * @file MarkdownParser.h
 * @author Silmaen
 * @date 18/02/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace evl::gui::utils {

/**
 * @brief Types of markdown elements.
 */
enum struct MarkdownElementType : uint8_t {
	Heading1,
	Heading2,
	Heading3,
	Heading4,
	Paragraph,
	BulletItem,
	Image,
	TableHeader,
	TableRow,
	Separator,
};

/**
 * @brief A span of text with optional bold formatting.
 */
struct TextSpan {
	/// The text content.
	std::string text;
	/// Whether the text is bold.
	bool bold = false;
};

/**
 * @brief A parsed markdown element.
 */
struct MarkdownElement {
	/// The element type.
	MarkdownElementType type = MarkdownElementType::Paragraph;
	/// The raw text content.
	std::string rawText;
	/// Parsed text spans (for paragraphs, bullets, table cells with bold).
	std::vector<TextSpan> spans;
	/// Image path (relative, for Image elements).
	std::string imagePath;
	/// Image alt text (for Image elements).
	std::string imageAlt;
	/// Table cells (for TableHeader and TableRow elements).
	std::vector<std::vector<TextSpan>> tableCells;
};

/**
 * @brief Parse a markdown file into a list of elements.
 * @param iFilePath The path to the markdown file.
 * @return The list of parsed elements.
 */
auto parseMarkdownFile(const std::filesystem::path& iFilePath) -> std::vector<MarkdownElement>;

/**
 * @brief Parse inline bold markers (**text**) into text spans.
 * @param iText The raw text to parse.
 * @return The list of text spans.
 */
auto parseInlineFormatting(const std::string& iText) -> std::vector<TextSpan>;

}// namespace evl::gui::utils
