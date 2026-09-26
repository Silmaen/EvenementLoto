/**
 * @file Prize.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Prize.h"

#include "StreamRead.h"
#include "StreamWrite.h"

namespace evl::core {

void Prize::read(std::istream& iBs, const ReadContext& /*iContext*/) {
	if (!readString(iBs, m_designation) || !readString(iBs, m_donor) || !readRaw(iBs, m_value))
		return;
	uint8_t attractiveness = 0;
	uint8_t childFriendly = 0;
	if (!readRaw(iBs, attractiveness) || !readRaw(iBs, childFriendly))
		return;
	// Bornée à la lecture : un fichier abîmé ne doit pas faire sortir la note de
	// l'échelle sur laquelle l'interface la dessine.
	setAttractiveness(attractiveness);
	m_childFriendly = childFriendly != 0;
	m_value = std::max(m_value, 0.0);
}

void Prize::write(std::ostream& iBs) const {
	writeString(iBs, m_designation);
	writeString(iBs, m_donor);
	writeRaw(iBs, m_value);
	writeRaw(iBs, m_attractiveness);
	writeRaw(iBs, static_cast<uint8_t>(m_childFriendly ? 1 : 0));
}

auto Prize::toJson() const -> Json::Value {
	Json::Value value;
	value["designation"] = m_designation;
	value["donor"] = m_donor;
	value["value"] = m_value;
	value["attractiveness"] = m_attractiveness;
	value["child_friendly"] = m_childFriendly;
	return value;
}

void Prize::fromJson(const Json::Value& iJson) {
	if (const auto val = iJson.get("designation", ""); val.isString())
		m_designation = val.asString();
	if (const auto val = iJson.get("donor", ""); val.isString())
		m_donor = val.asString();
	if (const auto val = iJson.get("value", 0.0); val.isNumeric())
		setValue(val.asDouble());
	if (const auto val = iJson.get("attractiveness", 0); val.isNumeric())
		setAttractiveness(static_cast<uint8_t>(std::clamp(val.asInt(), 0, static_cast<int>(g_maxAttractiveness))));
	if (const auto val = iJson.get("child_friendly", true); val.isBool())
		m_childFriendly = val.asBool();
}

auto Prize::toYaml() const -> YAML::Node {
	YAML::Node node;
	node["designation"] = m_designation;
	node["donor"] = m_donor;
	node["value"] = m_value;
	node["attractiveness"] = static_cast<int>(m_attractiveness);
	node["child_friendly"] = m_childFriendly;
	return node;
}

void Prize::fromYaml(const YAML::Node& iNode) {
	m_designation = iNode["designation"].as<std::string>("");
	m_donor = iNode["donor"].as<std::string>("");
	setValue(iNode["value"].as<double>(0.0));
	setAttractiveness(static_cast<uint8_t>(
			std::clamp(iNode["attractiveness"].as<int>(0), 0, static_cast<int>(g_maxAttractiveness))));
	m_childFriendly = iNode["child_friendly"].as<bool>(true);
}

auto totalValue(const prizes_type& iPrizes) -> double {
	return std::accumulate(iPrizes.begin(), iPrizes.end(), 0.0,
						   [](const double iAccu, const Prize& iPrize) -> double { return iAccu + iPrize.getValue(); });
}

auto designations(const prizes_type& iPrizes) -> std::string {
	std::string joined;
	for (const auto& prize: iPrizes) {
		if (prize.getDesignation().empty())
			continue;
		if (!joined.empty())
			joined += '\n';
		joined += prize.getDesignation();
	}
	return joined;
}

auto prizesFromLegacy(const std::string& iPrices, const double iValue) -> prizes_type {
	prizes_type prizes;
	std::istringstream stream{iPrices};
	std::string line;
	while (std::getline(stream, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			continue;
		prizes.emplace_back(line);
	}
	if (prizes.empty()) {
		if (iValue == 0.0)
			return prizes;
		prizes.emplace_back();
	}
	prizes.front().setValue(iValue);
	return prizes;
}

}// namespace evl::core
