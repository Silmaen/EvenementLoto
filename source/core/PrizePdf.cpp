/**
 * @file PrizePdf.cpp
 * @author Silmaen
 * @date 25/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "PrizePdf.h"

#include "AtomicFile.h"
#include "Event.h"

#include <algorithm>
#include <array>
#include <fstream>

namespace evl::core {

namespace {

constexpr double g_pageWidth = 595.28;
constexpr double g_pageHeight = 841.89;
constexpr double g_margin = 42.0;
constexpr double g_maxSize = 40.0;
constexpr double g_minSize = 18.0;
constexpr double g_lineFactor = 1.2;
constexpr double g_detailFactor = 0.62;
constexpr double g_setFactor = 0.8;

/// Helvetica advance widths of the characters 32 to 126, in thousandths of an em.
constexpr std::array<int, 95> g_widths = {
		278,  278, 355, 556, 556, 889, 667, 191, 333, 333, 389, 584, 278, 333, 278, 278, 556, 556, 556, 556,
		556,  556, 556, 556, 556, 556, 278, 278, 584, 584, 584, 556, 1015, 667, 667, 722, 722, 667, 611, 778,
		722,  278, 500, 667, 556, 833, 722, 778, 667, 778, 722, 667, 611, 722, 667, 944, 667, 667, 611, 278,
		278,  278, 469, 556, 333, 556, 556, 500, 556, 556, 278, 556, 556, 222, 222, 500, 222, 833, 556, 556,
		556,  556, 333, 500, 278, 556, 500, 722, 500, 500, 500, 334, 260, 334, 584};

/// One character of the WinAnsi encoding, or '?' for what it cannot draw.
auto winAnsi(const char32_t iCode) -> char {
	if (iCode >= 0x20 && iCode < 0x7F)
		return static_cast<char>(iCode);
	if (iCode >= 0xA0 && iCode <= 0xFF)
		return static_cast<char>(iCode);
	switch (iCode) {
		case 0x20AC: return static_cast<char>(0x80);
		case 0x2018: return static_cast<char>(0x91);
		case 0x2019: return static_cast<char>(0x92);
		case 0x201C: return static_cast<char>(0x93);
		case 0x201D: return static_cast<char>(0x94);
		case 0x2022: return static_cast<char>(0x95);
		case 0x2013: return static_cast<char>(0x96);
		case 0x2014: return static_cast<char>(0x97);
		case 0x0152: return static_cast<char>(0x8C);
		case 0x0153: return static_cast<char>(0x9C);
		default: return iCode < 0x20 ? ' ' : '?';
	}
}

/// UTF-8 to the single-byte encoding the standard fonts are declared with.
auto toWinAnsi(const std::string_view iText) -> std::string {
	std::string out;
	for (size_t i = 0; i < iText.size();) {
		const auto lead = static_cast<unsigned char>(iText[i]);
		size_t length = 1;
		char32_t code = lead;
		if (lead >= 0xF0) {
			length = 4;
			code = lead & 0x07U;
		} else if (lead >= 0xE0) {
			length = 3;
			code = lead & 0x0FU;
		} else if (lead >= 0xC0) {
			length = 2;
			code = lead & 0x1FU;
		} else if (lead >= 0x80) {
			code = 0xFFFD;
		}
		for (size_t k = 1; k < length && i + k < iText.size(); ++k)
			code = (code << 6U) | (static_cast<unsigned char>(iText[i + k]) & 0x3FU);
		out += winAnsi(code);
		i += length;
	}
	return out;
}

/// Width of an encoded text in points. Bold is estimated wider, on purpose: too big would overflow.
auto textWidth(const std::string_view iEncoded, const double iSize, const bool iBold) -> double {
	double total = 0.0;
	for (const char c: iEncoded) {
		const auto byte = static_cast<unsigned char>(c);
		if (byte >= 32 && byte <= 126)
			total += g_widths.at(byte - 32U);
		else if (byte == 0x97)
			total += 1000.0;
		else if (byte >= 0xC0 && byte <= 0xDF)
			total += 700.0;
		else
			total += 556.0;
	}
	return total * iSize / 1000.0 * (iBold ? 1.1 : 1.0);
}

/// A text cut into lines no wider than the given width. A word too long is cut where it falls.
auto wrap(const std::string& iEncoded, const double iSize, const bool iBold, const double iWidth)
		-> std::vector<std::string> {
	std::vector<std::string> lines;
	std::string current;
	const auto flush = [&]() -> void {
		if (!current.empty())
			lines.push_back(current);
		current.clear();
	};
	size_t pos = 0;
	while (pos <= iEncoded.size()) {
		auto end = iEncoded.find(' ', pos);
		if (end == std::string::npos)
			end = iEncoded.size();
		std::string word = iEncoded.substr(pos, end - pos);
		pos = end + 1;
		if (word.empty())
			continue;
		while (textWidth(word, iSize, iBold) > iWidth && word.size() > 1) {
			// Too long for a line of its own: break it where it stops fitting.
			size_t cut = word.size() - 1;
			while (cut > 1 && textWidth(word.substr(0, cut), iSize, iBold) > iWidth)
				--cut;
			flush();
			lines.push_back(word.substr(0, cut));
			word.erase(0, cut);
		}
		const std::string candidate = current.empty() ? word : current + ' ' + word;
		if (!current.empty() && textWidth(candidate, iSize, iBold) > iWidth) {
			flush();
			current = word;
		} else {
			current = candidate;
		}
	}
	flush();
	if (lines.empty())
		lines.emplace_back();
	return lines;
}

/// A text as a PDF string literal.
auto literal(const std::string& iEncoded) -> std::string {
	std::string out = "(";
	for (const char c: iEncoded) {
		if (c == '(' || c == ')' || c == '\\')
			out += '\\';
		out += c;
	}
	out += ')';
	return out;
}

auto euros(const double iValue) -> std::string {
	auto text = std::format("{:.2f}", iValue);
	std::ranges::replace(text, '.', ',');
	return text + " €";
}

/// What the presenter says about one prize, ready to be placed on a page.
struct Item {
	std::string head;///< « 3. Designation », wrapped lines come later.
	std::string details;///< Donor and value.
	uint32_t set = 0;///< The winner whose set it belongs to, for the first article of a set only.
};

/// One page being drawn: the PDF operators, and the height already used.
struct Page {
	std::string content;
};

class PdfBuilder {
public:
	void addText(Page& ioPage, const double iX, const double iTop, const double iSize, const bool iBold,
				 const double iGrey, const std::string& iEncoded) const {
		ioPage.content += std::format("BT {:.2f} g /F{} {:.2f} Tf {:.2f} {:.2f} Td {} Tj ET\n", iGrey, iBold ? 2 : 1,
									  iSize, iX, g_pageHeight - iTop, literal(iEncoded));
	}

	void addRule(Page& ioPage, const double iTop) const {
		ioPage.content += std::format("0.6 G 1.5 w {:.2f} {:.2f} m {:.2f} {:.2f} l S\n", g_margin,
									  g_pageHeight - iTop, g_pageWidth - g_margin, g_pageHeight - iTop);
	}

	[[nodiscard]] auto serialize() const -> std::string {
		std::vector<std::string> objects;
		// 1 catalog, 2 pages, 3 and 4 fonts, then a (page, content) pair per page.
		objects.emplace_back("<< /Type /Catalog /Pages 2 0 R >>");
		std::string kids;
		for (size_t i = 0; i < pages.size(); ++i)
			kids += std::format("{} 0 R ", 5 + 2 * i);
		objects.push_back(std::format("<< /Type /Pages /Kids [{}] /Count {} >>", kids, pages.size()));
		objects.emplace_back("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>");
		objects.emplace_back("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold /Encoding /WinAnsiEncoding >>");
		for (size_t i = 0; i < pages.size(); ++i) {
			objects.push_back(std::format("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 {:.2f} {:.2f}] "
										  "/Resources << /Font << /F1 3 0 R /F2 4 0 R >> >> /Contents {} 0 R >>",
										  g_pageWidth, g_pageHeight, 6 + 2 * i));
			objects.push_back(std::format("<< /Length {} >>\nstream\n{}endstream", pages[i].content.size(),
										  pages[i].content));
		}
		std::string out = "%PDF-1.4\n";
		std::vector<size_t> offsets;
		for (size_t i = 0; i < objects.size(); ++i) {
			offsets.push_back(out.size());
			out += std::format("{} 0 obj\n{}\nendobj\n", i + 1, objects[i]);
		}
		const size_t xref = out.size();
		out += std::format("xref\n0 {}\n0000000000 65535 f \n", objects.size() + 1);
		for (const size_t offset: offsets)
			out += std::format("{:010} 00000 n \n", offset);
		out += std::format("trailer\n<< /Size {} /Root 1 0 R >>\nstartxref\n{}\n%%EOF\n", objects.size() + 1, xref);
		return out;
	}

	std::vector<Page> pages;
};

/// Laid-out prize: the wrapped head lines and the details, at one text size.
struct Block {
	std::vector<std::string> head;
	std::string details;
	uint32_t set = 0;
	double height = 0.0;
};

auto layoutBlock(const Item& iItem, const double iSize, const double iWidth) -> Block {
	Block block;
	block.head = wrap(iItem.head, iSize, true, iWidth);
	block.details = iItem.details;
	block.set = iItem.set;
	block.height = static_cast<double>(block.head.size()) * iSize * g_lineFactor;
	if (block.set != 0)
		block.height += iSize * g_setFactor * g_lineFactor + iSize * 0.3;
	if (!block.details.empty())
		block.height += iSize * g_detailFactor * g_lineFactor;
	block.height += iSize * 0.45;// room before the next prize
	return block;
}

/// A titled list of prizes, with the total of its values.
struct Section {
	std::string title;
	std::string subtitle;
	std::vector<Item> items;
	double sum = 0.0;
};

/// Title size and header height of a section: the title shrinks to fit the page width.
struct Header {
	double titleSize = 30.0;
	double height = 0.0;
};

auto headerOf(const std::string& iEncodedTitle) -> Header {
	Header header;
	while (header.titleSize > 14.0 && textWidth(iEncodedTitle, header.titleSize, true) > g_pageWidth - 2 * g_margin)
		header.titleSize -= 1.0;
	header.height = header.titleSize * 1.3 + 22.0 * 1.3 + 18.0;
	return header;
}

constexpr double g_footerHeight = 40.0;

auto layoutAll(const std::vector<Item>& iItems, const double iSize) -> std::vector<Block> {
	std::vector<Block> blocks;
	for (const auto& item: iItems)
		blocks.push_back(layoutBlock({toWinAnsi(item.head), toWinAnsi(item.details), item.set}, iSize,
									 g_pageWidth - 2 * g_margin));
	return blocks;
}

/// The biggest text size at which the whole section fits one page, the minimum when none does.
auto fitSize(const Section& iSection) -> double {
	const double available = g_pageHeight - 2 * g_margin - headerOf(toWinAnsi(iSection.title)).height - g_footerHeight;
	for (double candidate = g_maxSize; candidate > g_minSize; candidate -= 1.0) {
		double sum = 0.0;
		for (const auto& block: layoutAll(iSection.items, candidate))
			sum += block.height;
		if (sum <= available)
			return candidate;
	}
	return g_minSize;
}

/// A section on as many pages as it takes, at the text size shared by the whole document.
void addSection(PdfBuilder& ioPdf, const Section& iSection, const double iSize) {
	const std::string title = toWinAnsi(iSection.title);
	const std::string subtitle = toWinAnsi(iSection.subtitle);
	const auto [titleSize, headerHeight] = headerOf(title);
	const double available = g_pageHeight - 2 * g_margin - headerHeight - g_footerHeight;
	const auto blocks = layoutAll(iSection.items, iSize);

	size_t index = 0;
	size_t pageNumber = 0;
	while (index < blocks.size()) {
		Page page;
		const std::string suffix = pageNumber == 0 ? "" : " (suite)";
		ioPdf.addText(page, g_margin, g_margin + titleSize, titleSize, true, 0.0, title + toWinAnsi(suffix));
		ioPdf.addText(page, g_margin, g_margin + titleSize * 1.3 + 22.0, 22.0, false, 0.35, subtitle);
		ioPdf.addRule(page, g_margin + headerHeight - 8.0);
		double top = g_margin + headerHeight;
		bool first = true;
		while (index < blocks.size() &&
			   (first || top + blocks[index].height <= g_margin + headerHeight + available)) {
			const bool firstOnPage = first;
			first = false;
			const auto& block = blocks[index];
			double lineTop = top;
			if (block.set != 0) {
				// A new set: a title, then a thin rule above it when it is not the page's first.
				if (!firstOnPage)
					ioPdf.addRule(page, lineTop + iSize * 0.05);
				lineTop += iSize * 0.3;
				ioPdf.addText(page, g_margin, lineTop + iSize * g_setFactor, iSize * g_setFactor, true, 0.35,
							  toWinAnsi(std::format("Gagnant {}", block.set)));
				lineTop += iSize * g_setFactor * g_lineFactor;
			}
			for (const auto& line: block.head) {
				ioPdf.addText(page, g_margin, lineTop + iSize, iSize, true, 0.0, line);
				lineTop += iSize * g_lineFactor;
			}
			if (!block.details.empty())
				ioPdf.addText(page, g_margin, lineTop + iSize * g_detailFactor, iSize * g_detailFactor, false, 0.35,
							  block.details);
			top += block.height;
			++index;
		}
		ioPdf.pages.push_back(std::move(page));
		++pageNumber;
	}
	if (iSection.sum > 0.0) {
		// The total goes at the foot of the last page of the phase.
		ioPdf.pages.back().content += std::format("BT 0.35 g /F1 16 Tf {:.2f} {:.2f} Td {} Tj ET\n", g_margin,
												  g_margin + 6.0,
												  literal(toWinAnsi("Valeur totale de la phase : " + euros(iSection.sum))));
	}
}

/// The prizes of a list as what is announced: designation, donor and value.
auto toItems(const std::vector<AnnouncedPrize>& iPrizes) -> std::vector<Item> {
	std::vector<Item> items;
	for (const auto& entry: iPrizes) {
		const auto& prize = *entry.prize;
		std::string details;
		if (!prize.getDonor().empty())
			details = "offert par " + prize.getDonor();
		if (prize.getValue() > 0.0)
			details += std::string(details.empty() ? "" : "  —  ") + "valeur " + euros(prize.getValue());
		items.push_back({prize.getDesignation(), details, entry.firstOfSet ? entry.set : 0U});
	}
	return items;
}

}// namespace

auto buildPrizePdf(const Event& iEvent) -> std::string {
	std::vector<Section> sections;
	for (auto round = iEvent.beginRounds(); round != iEvent.endRounds(); ++round) {
		if (round->getType() == GameRound::Type::Pause)
			continue;
		for (size_t sub = 0; sub < round->sizeSubRound(); ++sub) {
			const auto subRound = round->getSubRound(static_cast<uint32_t>(sub));
			Section section{round->getName(), round->getSubRoundLabel(sub), toItems(announcedPrizes(subRound->getPrizes(), round->getType() == GameRound::Type::Enfant)), 0.0};
			for (const auto& prize: subRound->getPrizes())
				section.sum += prize.getValue();
			sections.push_back(std::move(section));
		}
	}
	std::erase_if(sections, [](const Section& iSection) -> bool { return iSection.items.empty(); });
	if (sections.empty())
		return {};

	// One size for the whole document: the one the fullest phase can bear.
	double size = g_maxSize;
	for (const auto& section: sections)
		size = std::min(size, fitSize(section));
	PdfBuilder pdf;
	for (const auto& section: sections)
		addSection(pdf, section, size);
	return pdf.serialize();
}

auto writePrizePdf(const Event& iEvent, const std::filesystem::path& iPath) -> bool {
	const auto pdf = buildPrizePdf(iEvent);
	if (pdf.empty())
		return false;
	return writeFileAtomically(iPath, [&pdf](std::ostream& ioStream) -> void {
		ioStream.write(pdf.data(), static_cast<std::streamsize>(pdf.size()));
	});
}

}// namespace evl::core
