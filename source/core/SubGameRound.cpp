/**
* @file SubGameRound.cpp
* @author Silmaen
* @date 26/10/202
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "pch.h"

#include "SubGameRound.h"

#include "Log.h"
#include "utilities.h"

#include <utility>

namespace evl::core {
namespace {

const std::unordered_map<SubGameRound::Type, std::string> g_typeConvert = {
		{SubGameRound::Type::OneQuine, "simple quine"},
		{SubGameRound::Type::TwoQuines, "double quine"},
		{SubGameRound::Type::FullCard, "carton plein"},
		{SubGameRound::Type::Inverse, "inverse"},
};

const std::unordered_map<SubGameRound::Status, std::string> g_statusConvert = {
		{SubGameRound::Status::Ready, "prêt"},
		{SubGameRound::Status::PreScreen, "affichage"},
		{SubGameRound::Status::Running, "en cours"},
		{SubGameRound::Status::Done, "fini"},
};

}// namespace

auto SubGameRound::getTypeStr() const -> std::string {
	if (g_typeConvert.contains(m_type)) {
		return g_typeConvert.at(m_type);
	}
	return "inconnu";
}

auto SubGameRound::getStatusStr() const -> std::string {
	if (g_statusConvert.contains(m_status)) {
		return g_statusConvert.at(m_status);
	}
	return "inconnu";
}

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

void SubGameRound::read(std::istream& iBs, const int iFileVersion) {
	if (std::cmp_greater(iFileVersion, getSaveVersion()))
		return;
	iBs.read(reinterpret_cast<char*>(&m_type), sizeof(Type));
	if (iFileVersion >= 4) {
		iBs.read(reinterpret_cast<char*>(&m_status), sizeof(Status));
	}
	if (iFileVersion < 4) {//----UNCOVER----
		uint32_t readTmp = 0;//----UNCOVER----
		iBs.read(reinterpret_cast<char*>(&readTmp), sizeof(uint32_t));//----UNCOVER----
		if (readTmp != 0)//----UNCOVER----
			m_winner = "toto";//----UNCOVER----
		else//----UNCOVER----
			m_winner = "";//----UNCOVER----
	} else {//----UNCOVER----
		iBs.read(reinterpret_cast<char*>(&m_pricesValue), sizeof(double));
		std::string::size_type l = 0;
		iBs.read(reinterpret_cast<char*>(&l), sizeof(std::string::size_type));
		m_winner.resize(l);
		for (std::string::size_type i = 0; i < l; i++)
			iBs.read(reinterpret_cast<char*>(&m_winner[i]), sizeof(std::string::value_type));
	}
	std::string::size_type l = 0;
	iBs.read(reinterpret_cast<char*>(&l), sizeof(std::string::size_type));
	m_prices.resize(l);
	for (std::string::size_type i = 0; i < l; i++)
		iBs.read(reinterpret_cast<char*>(&m_prices[i]), sizeof(std::string::value_type));
	if (iFileVersion > 3) {
		draws_type::size_type ld = 0;
		iBs.read(reinterpret_cast<char*>(&ld), sizeof(draws_type::size_type));
		m_draws.resize(ld);
		for (draws_type::size_type i = 0; i < ld; ++i)
			iBs.read(reinterpret_cast<char*>(&(m_draws[i])), sizeof(draws_type::value_type));
	}
	if (iFileVersion > 5) {
		iBs.read(reinterpret_cast<char*>(&m_start), sizeof(m_start));
		iBs.read(reinterpret_cast<char*>(&m_end), sizeof(m_end));
	}
}

void SubGameRound::write(std::ostream& iBs) const {
	iBs.write(reinterpret_cast<const char*>(&m_type), sizeof(Type));
	iBs.write(reinterpret_cast<const char*>(&m_status), sizeof(Status));
	iBs.write(reinterpret_cast<const char*>(&m_pricesValue), sizeof(double));
	//---------------
	std::string::size_type l0 = m_winner.size();
	iBs.write(reinterpret_cast<char*>(&l0), sizeof(std::string::size_type));
	for (std::string::size_type i = 0; i < l0; i++)
		iBs.write(reinterpret_cast<const char*>(&m_winner[i]), sizeof(std::string::value_type));
	//---------------
	std::string::size_type l = m_prices.size();
	iBs.write(reinterpret_cast<char*>(&l), sizeof(std::string::size_type));
	for (std::string::size_type i = 0; i < l; i++)
		iBs.write(reinterpret_cast<const char*>(&m_prices[i]), sizeof(std::string::value_type));
	// --------------
	draws_type::size_type ld = m_draws.size();
	iBs.write(reinterpret_cast<const char*>(&ld), sizeof(draws_type::size_type));
	for (draws_type::size_type i = 0; i < ld; ++i)
		iBs.write(reinterpret_cast<const char*>(&(m_draws[i])), sizeof(draws_type::value_type));
	iBs.write(reinterpret_cast<const char*>(&m_start), sizeof(m_start));
	iBs.write(reinterpret_cast<const char*>(&m_end), sizeof(m_end));
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
	std::string srType;
	if (const auto val = iJson.get("type", ""); val.isString()) {
		srType = val.asString();
	}
	if (const auto result = std::ranges::find_if(
				g_typeConvert, [&srType](const auto& iItem) -> auto { return iItem.second == srType; });
		result != g_typeConvert.end())
		m_type = result->first;
	if (const auto val = iJson.get("prices", ""); val.isString()) {
		m_prices = val.asString();
	}
	if (const auto val = iJson.get("prices", 0); val.isDouble()) {
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
	auto srType = iNode["type"].as<std::string>();
	if (const auto result = std::ranges::find_if(
				g_typeConvert, [&srType](const auto& iItem) -> auto { return iItem.second == srType; });
		result != g_typeConvert.end())
		m_type = result->first;
	m_prices = iNode["prices"].as<std::string>();
	m_pricesValue = iNode["value"].as<double>();
	m_winner = iNode["winner"].as<std::string>();
	m_draws.clear();
	for (const auto& item: iNode["draws"]) { m_draws.push_back(item.as<uint8_t>()); }
}

}// namespace evl::core
