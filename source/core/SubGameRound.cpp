/**
* @file SubGameRound.cpp
* @author Silmaen
* @date 26/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "pch.h"

#include "SubGameRound.h"

#include "YamlRead.h"

#include "StreamWrite.h"

#include "EnumLabel.h"
#include "StreamRead.h"

#include "Log.h"
#include "utilities.h"

#include <utility>

namespace evl::core {
namespace {

constexpr std::array<std::pair<SubGameRound::Type, std::string_view>, 4> g_typeLabels{{
		{SubGameRound::Type::OneQuine, "simple quine"},
		{SubGameRound::Type::TwoQuines, "double quine"},
		{SubGameRound::Type::FullCard, "carton plein"},
		{SubGameRound::Type::Inverse, "inverse"},
}};

constexpr std::array<std::pair<SubGameRound::Status, std::string_view>, 4> g_statusLabels{{
		{SubGameRound::Status::Ready, "prêt"},
		{SubGameRound::Status::PreScreen, "affichage"},
		{SubGameRound::Status::Running, "en cours"},
		{SubGameRound::Status::Done, "fini"},
}};

}// namespace

auto SubGameRound::getTypeStr() const -> std::string { return std::string(enumLabel(g_typeLabels, m_type)); }

auto SubGameRound::getStatusStr() const -> std::string { return std::string(enumLabel(g_statusLabels, m_status)); }

void SubGameRound::nextStatus() {
	switch (m_status) {
		case Status::Ready:
			m_start = clock::now();
			if (!m_prizes.empty())
				m_status = Status::PreScreen;
			else
				m_status = Status::Running;
			break;
		case Status::PreScreen:
			m_status = Status::Running;
			break;
		case Status::Running:
			if (!m_winner.empty()) {
				m_status = Status::Done;
				m_end = clock::now();
			}
			break;
		case Status::Done:// the last status!
			break;
		case Status::Invalid:
			log_warn("SubGameRound in invalid status, cannot advance!");
			break;
	}
}

void SubGameRound::setWinner(const std::string& iWinner) {
	if (m_status != Status::Running)
		return;
	// Settled, with a name or without: a round nobody claims ends all the same. Going
	// through nextStatus() kept it running as long as the name was empty, which made
	// « Passer » do nothing.
	m_winner = iWinner;
	m_status = Status::Done;
	m_end = clock::now();
}

void SubGameRound::mergeChildPhases(const std::vector<SubGameRound>& iPhases) {
	if (iPhases.empty())
		return;
	const auto unfinished = std::ranges::find_if(iPhases, [](const SubGameRound& iPhase) -> bool { return !iPhase.isFinished(); });
	const auto& reference = unfinished != iPhases.end() ? *unfinished : iPhases.back();
	auto merged = iPhases.front();
	merged.m_prizes.clear();
	uint32_t finished = 0;
	for (const auto& phase: iPhases) {
		merged.m_prizes.insert(merged.m_prizes.end(), phase.m_prizes.begin(), phase.m_prizes.end());
		if (phase.isFinished())
			++finished;
	}
	merged.m_status = reference.m_status;
	merged.m_draws = reference.m_draws;
	merged.m_end = reference.m_end;
	merged.m_winnerTarget = static_cast<uint8_t>(std::clamp(static_cast<uint32_t>(iPhases.size()), 1U, g_maxWinners));
	merged.m_winnersFound = static_cast<uint8_t>(std::min<uint32_t>(finished, merged.m_winnerTarget));
	merged.m_winner = unfinished == iPhases.end() ? std::format("{} gagnant(s)", merged.m_winnerTarget) : std::string{};
	*this = merged;
}

void SubGameRound::setWinnerTarget(const uint32_t iCount) {
	if (!isEditable()) {
		log_warn("Impossible de modifier le nombre de gagnants d'une manche non éditable");
		return;
	}
	m_winnerTarget = static_cast<uint8_t>(std::clamp(iCount, 1U, g_maxWinners));
}

auto SubGameRound::awardWinners(const uint32_t iCount) -> bool {
	if (m_status != Status::Running)
		return false;
	m_winnersFound = static_cast<uint8_t>(std::min<uint32_t>(m_winnersFound + iCount, m_winnerTarget));
	if (m_winnersFound < m_winnerTarget)
		return false;
	setWinner(std::format("{} gagnant(s)", m_winnerTarget));
	return true;
}

void SubGameRound::read(std::istream& iBs, const ReadContext& iContext) {
	if (iContext.version > getSaveVersion()) {
		iBs.setstate(std::ios::failbit);
		return;
	}
	if (!readEnum(iBs, m_type, iContext.wideEnums))
		return;
	if (iContext.version >= 4 && !readEnum(iBs, m_status, iContext.wideEnums))
		return;
	if (iContext.version >= 8) {
		if (!readString(iBs, m_winner) || !readPrizes(iBs, iContext))
			return;
	} else {
		// Avant la version 8 les lots sont une chaine multiligne et une valeur unique.
		double legacyValue = 0.0;
		if (iContext.version < 4) {
			uint32_t hasWinner = 0;
			if (!readRaw(iBs, hasWinner))
				return;
			m_winner = hasWinner != 0 ? "gagnant" : "";
		} else if (!readRaw(iBs, legacyValue) || !readString(iBs, m_winner)) {
			return;
		}
		std::string legacyPrices;
		if (!readString(iBs, legacyPrices))
			return;
		m_prizes = prizesFromLegacy(legacyPrices, legacyValue);
	}
	if (iContext.version > 3 && !readVector(iBs, m_draws))
		return;
	if (iContext.version > 5 && (!readTimePoint(iBs, m_start) || !readTimePoint(iBs, m_end)))
		return;
	m_winnerTarget = 1;
	m_winnersFound = 0;
	if (iContext.version > 12) {
		uint8_t target = 1;
		uint8_t found = 0;
		if (!readRaw(iBs, target) || !readRaw(iBs, found))
			return;
		if (target < 1 || target > g_maxWinners || found > target) {
			iBs.setstate(std::ios::failbit);
			return;
		}
		m_winnerTarget = target;
		m_winnersFound = found;
	}
}

auto SubGameRound::readPrizes(std::istream& iBs, const ReadContext& iContext) -> bool {
	std::size_t count = 0;
	if (!readLength(iBs, count, g_maxSerializedCount))
		return false;
	m_prizes.assign(count, Prize{});
	for (auto& prize: m_prizes) {
		prize.read(iBs, iContext);
		if (!iBs.good())
			return false;
	}
	return true;
}

void SubGameRound::write(std::ostream& iBs) const {
	writeEnum(iBs, m_type);
	writeEnum(iBs, m_status);
	writeString(iBs, m_winner);
	writeLength(iBs, m_prizes.size());
	for (const auto& prize: m_prizes) { prize.write(iBs); }
	writeVector(iBs, m_draws);
	writeTimePoint(iBs, m_start);
	writeTimePoint(iBs, m_end);
	writeRaw(iBs, m_winnerTarget);
	writeRaw(iBs, m_winnersFound);
}

auto SubGameRound::toJson() const -> Json::Value {
	Json::Value value;
	value["type"] = getTypeStr();
	value["winner"] = m_winner;
	value["winnerTarget"] = m_winnerTarget;
	value["winnersFound"] = m_winnersFound;
	Json::Value prizesArray(Json::arrayValue);
	for (const auto& prize: m_prizes) { prizesArray.append(prize.toJson()); }
	value["prizes"] = prizesArray;
	Json::Value drawsArray(Json::arrayValue);
	for (const auto& d: m_draws) { drawsArray.append(d); }
	value["draws"] = drawsArray;
	return value;
}

void SubGameRound::fromJson(const Json::Value& iJson) {
	if (const auto val = iJson.get("type", ""); val.isString()) {
		m_type = enumFromLabel(g_typeLabels, val.asString(), m_type);
	}
	if (const auto val = iJson.get("winner", ""); val.isString()) {
		m_winner = val.asString();
	}
	m_winnerTarget = static_cast<uint8_t>(std::clamp(iJson.get("winnerTarget", 1).asInt(), 1, static_cast<int>(g_maxWinners)));
	m_winnersFound = static_cast<uint8_t>(std::clamp(iJson.get("winnersFound", 0).asInt(), 0, static_cast<int>(m_winnerTarget)));
	if (const auto val = iJson.get("prizes", ""); val.isArray()) {
		m_prizes.clear();
		for (const auto& item: val) {
			Prize prize;
			prize.fromJson(item);
			m_prizes.push_back(prize);
		}
	} else {
		// Un export d'avant la version 8 : la chaine multiligne et sa valeur unique.
		const auto prices = iJson.get("prices", "");
		const auto value = iJson.get("value", 0);
		m_prizes = prizesFromLegacy(prices.isString() ? prices.asString() : std::string{},
									value.isNumeric() ? value.asDouble() : 0.0);
	}
	if (const auto val = iJson.get("draws", ""); val.isArray()) {
		m_draws.clear();
		for (const auto& item: val) {
			if (item.isUInt()) {
				m_draws.push_back(static_cast<uint8_t>(item.asUInt()));
			}
		}
	}
}

auto SubGameRound::toYaml() const -> YAML::Node {
	YAML::Node node;
	node["type"] = getTypeStr();
	node["winner"] = m_winner;
	node["winnerTarget"] = static_cast<int>(m_winnerTarget);
	node["winnersFound"] = static_cast<int>(m_winnersFound);
	YAML::Node prizesNode;
	for (const auto& prize: m_prizes) { prizesNode.push_back(prize.toYaml()); }
	node["prizes"] = prizesNode;
	YAML::Node drawsNode;
	for (const auto& d: m_draws) { drawsNode.push_back(d); }
	node["draws"] = drawsNode;
	return node;
}

void SubGameRound::fromYaml(const YAML::Node& iNode) {
	m_type = enumFromLabel(g_typeLabels, yamlValue(iNode, "type", std::string{}), m_type);
	m_winner = yamlValue(iNode, "winner", std::string{});
	m_winnerTarget = static_cast<uint8_t>(std::clamp(yamlValue(iNode, "winnerTarget", 1), 1, static_cast<int>(g_maxWinners)));
	m_winnersFound = static_cast<uint8_t>(std::clamp(yamlValue(iNode, "winnersFound", 0), 0, static_cast<int>(m_winnerTarget)));
	m_prizes.clear();
	if (const auto prizesNode = yamlChild(iNode, "prizes"); prizesNode.IsSequence()) {
		for (const auto& item: prizesNode) {
			Prize prize;
			prize.fromYaml(item);
			m_prizes.push_back(prize);
		}
	} else {
		// Un export d'avant la version 8 : la chaine multiligne et sa valeur unique.
		m_prizes = prizesFromLegacy(yamlValue(iNode, "prices", std::string{}), yamlValue(iNode, "value", 0.0));
	}
	m_draws.clear();
	if (const auto draws = yamlChild(iNode, "draws"); draws.IsSequence()) {
		for (const auto& item: draws) { m_draws.push_back(item.as<uint8_t>(0)); }
	}
}

}// namespace evl::core
