/**
 * @file CsvCatalogue.cpp
 * @author Silmaen
 * @date 28/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "CsvCatalogue.h"

#include "Log.h"

#include <fstream>

namespace evl::core {

namespace {

/// Les colonnes que l'on sait reconnaître.
enum struct Column : uint8_t { Designation, Donor, Value, Attractiveness, ChildFriendly, Unknown };

/// L'ordre supposé quand aucun en-tête n'est reconnu.
constexpr std::array<Column, 5> g_defaultOrder{
		{Column::Designation, Column::Donor, Column::Value, Column::Attractiveness, Column::ChildFriendly}};

/// Sans accent, sans casse et sans espace : de quoi comparer un en-tête à un mot connu.
auto normalise(std::string_view iText) -> std::string {
	// Les accents sont en UTF-8, donc sur deux octets : la table les remplace par la
	// lettre nue plutôt que de dépendre d'une locale.
	static constexpr std::array<std::pair<std::string_view, char>, 10> accents{{{"é", 'e'},
																				{"è", 'e'},
																				{"ê", 'e'},
																				{"ë", 'e'},
																				{"à", 'a'},
																				{"â", 'a'},
																				{"î", 'i'},
																				{"ï", 'i'},
																				{"ô", 'o'},
																				{"û", 'u'}}};
	std::string result;
	for (std::size_t i = 0; i < iText.size();) {
		bool matched = false;
		for (const auto& [sequence, letter]: accents) {
			if (iText.compare(i, sequence.size(), sequence) == 0) {
				result += letter;
				i += sequence.size();
				matched = true;
				break;
			}
		}
		if (matched)
			continue;
		const auto character = static_cast<unsigned char>(iText[i]);
		if (std::isalnum(character) != 0)
			result += static_cast<char>(std::tolower(character));
		++i;
	}
	return result;
}

/// À quelle colonne un en-tête correspond.
auto columnOf(const std::string_view iHeader) -> Column {
	const auto key = normalise(iHeader);
	static constexpr std::array<std::pair<std::string_view, Column>, 16> names{
			{{"designation", Column::Designation},
			 {"libelle", Column::Designation},
			 {"article", Column::Designation},
			 {"lot", Column::Designation},
			 {"nom", Column::Designation},
			 {"donateur", Column::Donor},
			 {"donor", Column::Donor},
			 {"fournisseur", Column::Donor},
			 {"valeur", Column::Value},
			 {"prix", Column::Value},
			 {"value", Column::Value},
			 {"attrait", Column::Attractiveness},
			 {"attractivite", Column::Attractiveness},
			 {"note", Column::Attractiveness},
			 {"enfant", Column::ChildFriendly},
			 {"enfantcompatible", Column::ChildFriendly}}};
	for (const auto& [name, column]: names) {
		if (key == name)
			return column;
	}
	return Column::Unknown;
}

/// Le séparateur le plus présent dans la première ligne.
auto guessSeparator(const std::string_view iLine) -> char {
	char best = ';';
	std::size_t bestCount = 0;
	for (const char candidate: {';', ',', '\t'}) {
		const auto count = static_cast<std::size_t>(std::ranges::count(iLine, candidate));
		if (count > bestCount) {
			bestCount = count;
			best = candidate;
		}
	}
	return best;
}

/// Découpe une ligne, les guillemets protégeant le séparateur.
auto splitLine(const std::string_view iLine, const char iSeparator) -> std::vector<std::string> {
	std::vector<std::string> fields;
	std::string current;
	bool quoted = false;
	for (std::size_t i = 0; i < iLine.size(); ++i) {
		const char character = iLine[i];
		if (quoted) {
			if (character != '"') {
				current += character;
				continue;
			}
			// Deux guillemets de suite valent un guillemet dans le champ.
			if (i + 1 < iLine.size() && iLine[i + 1] == '"') {
				current += '"';
				++i;
				continue;
			}
			quoted = false;
			continue;
		}
		if (character == '"') {
			quoted = true;
			continue;
		}
		if (character == iSeparator) {
			fields.push_back(current);
			current.clear();
			continue;
		}
		current += character;
	}
	fields.push_back(current);
	return fields;
}

/// Les espaces et le retour chariot en trop, aux deux bouts.
auto trim(std::string_view iText) -> std::string {
	constexpr std::string_view blanks{" \t\r\n"};
	const auto first = iText.find_first_not_of(blanks);
	if (first == std::string_view::npos)
		return {};
	const auto last = iText.find_last_not_of(blanks);
	return std::string{iText.substr(first, last - first + 1)};
}

/// Un prix, la virgule décimale et le symbole euro acceptés.
auto parseValue(const std::string_view iText) -> std::optional<double> {
	std::string cleaned;
	for (const char character: iText) {
		if (character == ',') {
			cleaned += '.';
			continue;
		}
		// Les espaces de milliers, le symbole euro et tout le reste sont écartés.
		if (std::isdigit(static_cast<unsigned char>(character)) != 0 || character == '.' || character == '-')
			cleaned += character;
	}
	if (cleaned.empty())
		return std::nullopt;
	try {
		return std::stod(cleaned);
	} catch (const std::exception&) { return std::nullopt; }
}

/// Une note d'attractivité, le premier entier trouvé.
auto parseAttractiveness(const std::string_view iText) -> std::optional<uint8_t> {
	std::string digits;
	for (const char character: iText) {
		if (std::isdigit(static_cast<unsigned char>(character)) == 0) {
			if (!digits.empty())
				break;
			continue;
		}
		digits += character;
	}
	if (digits.empty())
		return std::nullopt;
	try {
		return static_cast<uint8_t>(std::clamp(std::stoi(digits), 0, static_cast<int>(Prize::g_maxAttractiveness)));
	} catch (const std::exception&) { return std::nullopt; }
}

/// Une case à cocher, dans les mots qu'un tableur y met.
auto parseBool(const std::string_view iText) -> std::optional<bool> {
	const auto key = normalise(iText);
	if (key.empty())
		return std::nullopt;
	static constexpr std::array<std::string_view, 6> yes{{"oui", "o", "y", "yes", "true", "vrai"}};
	static constexpr std::array<std::string_view, 6> no{{"non", "n", "no", "false", "faux", "0"}};
	if (std::ranges::contains(yes, key) || key == "1" || key == "x")
		return true;
	if (std::ranges::contains(no, key))
		return false;
	return std::nullopt;
}

}// namespace

auto parseCatalogueCsv(const std::string_view iContent) -> CsvImport {
	CsvImport result;
	result.read = true;

	std::vector<std::string> lines;
	{
		std::istringstream stream{std::string{iContent}};
		std::string line;
		while (std::getline(stream, line)) lines.push_back(line);
	}
	if (lines.empty()) {
		result.summary = "Le fichier est vide.";
		return result;
	}

	const char separator = guessSeparator(lines.front());
	std::vector<Column> layout;
	std::size_t firstData = 0;
	{
		const auto header = splitLine(lines.front(), separator);
		for (const auto& field: header) layout.push_back(columnOf(trim(field)));
		// Un en-tête n'en est un que s'il nomme au moins une colonne connue ; sinon la
		// première ligne est déjà une donnée et l'ordre par défaut s'applique.
		if (std::ranges::any_of(layout, [](const Column iColumn) -> bool { return iColumn != Column::Unknown; })) {
			firstData = 1;
		} else {
			layout.assign(g_defaultOrder.begin(), g_defaultOrder.end());
		}
	}

	for (std::size_t index = firstData; index < lines.size(); ++index) {
		const auto fields = splitLine(lines[index], separator);
		Prize prize;
		bool hasSomething = false;
		for (std::size_t column = 0; column < fields.size(); ++column) {
			const auto role = column < layout.size() ? layout[column] : Column::Unknown;
			const auto field = trim(fields[column]);
			if (field.empty())
				continue;
			switch (role) {
				case Column::Designation:
					prize.setDesignation(field);
					hasSomething = true;
					break;
				case Column::Donor:
					prize.setDonor(field);
					break;
				case Column::Value:
					if (const auto value = parseValue(field); value.has_value()) {
						prize.setValue(value.value());
						hasSomething = true;
					}
					break;
				case Column::Attractiveness:
					if (const auto note = parseAttractiveness(field); note.has_value())
						prize.setAttractiveness(note.value());
					break;
				case Column::ChildFriendly:
					if (const auto flag = parseBool(field); flag.has_value())
						prize.setChildFriendly(flag.value());
					break;
				case Column::Unknown:
					break;
			}
		}
		if (!hasSomething) {
			++result.skipped;
			continue;
		}
		result.prizes.push_back(prize);
	}

	result.summary = std::format("{} lot(s) lus", result.prizes.size());
	if (result.skipped > 0)
		result.summary += std::format(", {} ligne(s) sans désignation ni valeur ignorée(s)", result.skipped);
	result.summary += ".";
	return result;
}

auto importCatalogueCsv(const std::filesystem::path& iPath) -> CsvImport {
	CsvImport result;
	std::error_code error;
	if (!is_regular_file(iPath, error)) {
		result.summary = std::format("Fichier '{}' introuvable.", iPath.string());
		log_error("{}", result.summary);
		return result;
	}
	std::ifstream file(iPath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		result.summary = std::format("Fichier '{}' illisible.", iPath.string());
		log_error("{}", result.summary);
		return result;
	}
	const std::string content{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
	result = parseCatalogueCsv(content);
	log_info("Import de '{}' : {}", iPath.string(), result.summary);
	return result;
}

}// namespace evl::core
