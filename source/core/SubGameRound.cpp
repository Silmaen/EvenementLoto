/**
* @file SubGameRound.cpp
* @author Silmaen
* @date 26/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "pch.h"

#include "SubGameRound.h"

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
			if (!m_prices.empty())
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

void SubGameRound::read(std::istream& iBs, const ReadContext& iContext) {
	if (iContext.version > getSaveVersion()) {
		iBs.setstate(std::ios::failbit);
		return;
	}
	if (!readEnum(iBs, m_type, iContext.wideEnums))
		return;
	if (iContext.version >= 4 && !readEnum(iBs, m_status, iContext.wideEnums))
		return;
	if (iContext.version < 4) {
		uint32_t hasWinner = 0;
		if (!readRaw(iBs, hasWinner))
			return;
		m_winner = hasWinner != 0 ? "gagnant" : "";
	} else if (!readRaw(iBs, m_pricesValue) || !readString(iBs, m_winner)) {
		return;
	}
	if (!readString(iBs, m_prices))
		return;
	if (iContext.version > 3 && !readVector(iBs, m_draws))
		return;
	if (iContext.version > 5 && (!readTimePoint(iBs, m_start) || !readTimePoint(iBs, m_end)))
		return;
}

void SubGameRound::write(std::ostream& iBs) const {
	writeEnum(iBs, m_type);
	writeEnum(iBs, m_status);
	writeRaw(iBs, m_pricesValue);
	writeString(iBs, m_winner);
	writeString(iBs, m_prices);
	writeVector(iBs, m_draws);
	writeTimePoint(iBs, m_start);
	writeTimePoint(iBs, m_end);
}

auto SubGameRound::toJson() const -> Json::Value {
	Json::Value value;
	value["type"] = getTypeStr();
	value["prices"] = m_prices;
	value["value"] = m_pricesValue;
	value["winner"] = m_winner;
	Json::Value drawsArray(Json::arrayValue);
	for (const auto& d: m_draws) { drawsArray.append(d); }
	value["draws"] = drawsArray;
	return value;
}

void SubGameRound::fromJson(const Json::Value& iJson) {
	if (const auto val = iJson.get("type", ""); val.isString()) {
		m_type = enumFromLabel(g_typeLabels, val.asString(), m_type);
	}
	if (const auto val = iJson.get("prices", ""); val.isString()) {
		m_prices = val.asString();
	}
	if (const auto val = iJson.get("value", 0); val.isDouble()) {
		m_pricesValue = val.asDouble();
	}
	if (const auto val = iJson.get("winner", ""); val.isString()) {
		m_winner = val.asString();
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
	node["prices"] = m_prices;
	node["value"] = m_pricesValue;
	node["winner"] = m_winner;
	YAML::Node drawsNode;
	for (const auto& d: m_draws) { drawsNode.push_back(d); }
	node["draws"] = drawsNode;
	return node;
}

void SubGameRound::fromYaml(const YAML::Node& iNode) {
	m_type = enumFromLabel(g_typeLabels, iNode["type"].as<std::string>(), m_type);
	m_prices = iNode["prices"].as<std::string>();
	m_pricesValue = iNode["value"].as<double>();
	m_winner = iNode["winner"].as<std::string>();
	m_draws.clear();
	for (const auto& item: iNode["draws"]) { m_draws.push_back(item.as<uint8_t>()); }
}

}// namespace evl::core
