/**
 * @file MarkdownParser.cpp
 * @author Silmaen
 * @date 18/02/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "MarkdownParser.h"

#include <fstream>
#include <sstream>

namespace evl::gui::utils {

auto parseInlineFormatting(const std::string& iText) -> std::vector<TextSpan> {
	std::vector<TextSpan> spans;
	std::string current;
	size_t i = 0;
	while (i < iText.size()) {
		if (i + 1 < iText.size() && iText[i] == '*' && iText[i + 1] == '*') {
			// Flush current non-bold text.
			if (!current.empty()) {
				spans.push_back({.text = current, .bold = false});
				current.clear();
			}
			i += 2;
			// Find closing **.
			const auto end = iText.find("**", i);
			if (end != std::string::npos) {
				spans.push_back({.text = iText.substr(i, end - i), .bold = true});
				i = end + 2;
			} else {
				// No closing **, treat as plain text.
				current += "**";
			}
		} else {
			current += iText[i];
			++i;
		}
	}
	if (!current.empty())
		spans.push_back({.text = current, .bold = false});
	return spans;
}

namespace {

auto tryParseHeading(const std::string& iLine, MarkdownElement& oElement) -> bool {
	if (!iLine.starts_with("#"))
		return false;
	uint8_t level = 0;
	size_t pos = 0;
	while (pos < iLine.size() && iLine[pos] == '#') {
		++level;
		++pos;
	}
	if (level > 4 || pos >= iLine.size() || iLine[pos] != ' ')
		return false;
	const auto text = iLine.substr(pos + 1);
	switch (level) {
		case 1: oElement.type = MarkdownElementType::Heading1; break;
		case 2: oElement.type = MarkdownElementType::Heading2; break;
		case 3: oElement.type = MarkdownElementType::Heading3; break;
		default: oElement.type = MarkdownElementType::Heading4; break;
	}
	oElement.rawText = text;
	oElement.spans = parseInlineFormatting(text);
	return true;
}

auto tryParseImage(const std::string& iLine, MarkdownElement& oElement) -> bool {
	if (!iLine.starts_with("!["))
		return false;
	const auto altEnd = iLine.find("](");
	if (altEnd == std::string::npos)
		return false;
	const auto pathEnd = iLine.find(')', altEnd + 2);
	if (pathEnd == std::string::npos)
		return false;
	oElement.type = MarkdownElementType::Image;
	oElement.imageAlt = iLine.substr(2, altEnd - 2);
	oElement.imagePath = iLine.substr(altEnd + 2, pathEnd - altEnd - 2);
	return true;
}

auto tryParseBullet(const std::string& iLine, MarkdownElement& oElement) -> bool {
	if (!iLine.starts_with("* "))
		return false;
	const auto text = iLine.substr(2);
	oElement.type = MarkdownElementType::BulletItem;
	oElement.rawText = text;
	oElement.spans = parseInlineFormatting(text);
	return true;
}

auto isTableSeparator(const std::string& iLine) -> bool {
	if (!iLine.starts_with("|"))
		return false;
	return std::ranges::all_of(iLine, [](const char c) { return c == '|' || c == '-' || c == ' ' || c == ':'; });
}

auto parseTableCells(const std::string& iLine) -> std::vector<std::vector<TextSpan>> {
	std::vector<std::vector<TextSpan>> cells;
	// Skip leading |.
	size_t start = 0;
	if (!iLine.empty() && iLine[0] == '|')
		start = 1;
	// Skip trailing |.
	size_t end = iLine.size();
	if (end > 0 && iLine[end - 1] == '|')
		--end;
	const auto content = iLine.substr(start, end - start);
	std::istringstream stream(content);
	std::string cell;
	while (std::getline(stream, cell, '|')) {
		// Trim leading/trailing whitespace.
		const auto first = cell.find_first_not_of(' ');
		if (first == std::string::npos) {
			cells.push_back({{.text = "", .bold = false}});
			continue;
		}
		const auto last = cell.find_last_not_of(' ');
		cells.push_back(parseInlineFormatting(cell.substr(first, last - first + 1)));
	}
	return cells;
}

auto tryParseTable(const std::string& iLine, MarkdownElement& oElement, bool iIsFirstTableLine) -> bool {
	if (!iLine.starts_with("|"))
		return false;
	if (isTableSeparator(iLine)) {
		oElement.type = MarkdownElementType::Separator;
		return true;
	}
	oElement.type = iIsFirstTableLine ? MarkdownElementType::TableHeader : MarkdownElementType::TableRow;
	oElement.tableCells = parseTableCells(iLine);
	oElement.rawText = iLine;
	return true;
}

}// namespace

auto parseMarkdownFile(const std::filesystem::path& iFilePath) -> std::vector<MarkdownElement> {
	std::vector<MarkdownElement> elements;
	std::ifstream file(iFilePath);
	if (!file.is_open())
		return elements;

	std::string line;
	std::string paragraphAccum;
	bool inTable = false;

	auto flushParagraph = [&]() {
		if (paragraphAccum.empty())
			return;
		MarkdownElement elem;
		elem.type = MarkdownElementType::Paragraph;
		elem.rawText = paragraphAccum;
		elem.spans = parseInlineFormatting(paragraphAccum);
		elements.push_back(std::move(elem));
		paragraphAccum.clear();
	};

	while (std::getline(file, line)) {
		// Remove trailing whitespace.
		while (!line.empty() && (line.back() == ' ' || line.back() == '\r'))
			line.pop_back();

		// Empty line: flush paragraph.
		if (line.empty()) {
			flushParagraph();
			inTable = false;
			continue;
		}

		// Heading?
		MarkdownElement elem;
		if (tryParseHeading(line, elem)) {
			flushParagraph();
			inTable = false;
			elements.push_back(std::move(elem));
			continue;
		}

		// Image?
		if (tryParseImage(line, elem)) {
			flushParagraph();
			inTable = false;
			elements.push_back(std::move(elem));
			continue;
		}

		// Bullet?
		if (tryParseBullet(line, elem)) {
			flushParagraph();
			inTable = false;
			elements.push_back(std::move(elem));
			continue;
		}

		// Table line?
		if (line.starts_with("|")) {
			flushParagraph();
			const bool isFirstTableLine = !inTable;
			inTable = true;
			if (tryParseTable(line, elem, isFirstTableLine)) {
				if (elem.type != MarkdownElementType::Separator)
					elements.push_back(std::move(elem));
			}
			continue;
		}

		// Regular text: accumulate into paragraph.
		inTable = false;
		if (paragraphAccum.empty())
			paragraphAccum = line;
		else
			paragraphAccum += " " + line;
	}
	flushParagraph();
	return elements;
}

}// namespace evl::gui::utils
