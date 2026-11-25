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
#include "YamlRead.h"

namespace evl::core {

void Prize::read(std::istream& iBs, const ReadContext& iContext) {
	// version 10 : l'identifiant. Un article plus ancien en reçoit un à la lecture de
	// l'événement, qui est seul à savoir lesquels sont déjà pris.
	m_id = 0;
	if (iContext.version > 9 && !readRaw(iBs, m_id))
		return;
	if (!readString(iBs, m_designation) || !readString(iBs, m_donor) || !readRaw(iBs, m_value))
		return;
	uint8_t attractiveness = 0;
	uint8_t childFriendly = 0;
	if (!readRaw(iBs, attractiveness) || !readRaw(iBs, childFriendly))
		return;
	// version 14 : le set de lots du gagnant.
	uint8_t set = 0;
	if (iContext.version > 13 && !readRaw(iBs, set))
		return;
	setSet(set);
	// Bornée à la lecture : un fichier abîmé ne doit pas faire sortir la note de
	// l'échelle sur laquelle l'interface la dessine.
	setAttractiveness(attractiveness);
	m_childFriendly = childFriendly != 0;
	m_value = std::max(m_value, 0.0);
}

void Prize::write(std::ostream& iBs) const {
	writeRaw(iBs, m_id);
	writeString(iBs, m_designation);
	writeString(iBs, m_donor);
	writeRaw(iBs, m_value);
	writeRaw(iBs, m_attractiveness);
	writeRaw(iBs, static_cast<uint8_t>(m_childFriendly ? 1 : 0));
	writeRaw(iBs, m_set);
}

auto Prize::toJson() const -> Json::Value {
	Json::Value value;
	value["id"] = m_id;
	value["designation"] = m_designation;
	value["donor"] = m_donor;
	value["value"] = m_value;
	value["attractiveness"] = m_attractiveness;
	value["child_friendly"] = m_childFriendly;
	value["set"] = m_set;
	return value;
}

void Prize::fromJson(const Json::Value& iJson) {
	if (const auto val = iJson.get("id", 0); val.isNumeric())
		m_id = static_cast<uint32_t>(std::max(0, val.asInt()));
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
	if (const auto val = iJson.get("set", 0); val.isNumeric())
		setSet(static_cast<uint8_t>(std::clamp(val.asInt(), 0, static_cast<int>(g_maxSet))));
}

auto Prize::toYaml() const -> YAML::Node {
	YAML::Node node;
	node["id"] = m_id;
	node["designation"] = m_designation;
	node["donor"] = m_donor;
	node["value"] = m_value;
	node["attractiveness"] = static_cast<int>(m_attractiveness);
	node["child_friendly"] = m_childFriendly;
	node["set"] = static_cast<int>(m_set);
	return node;
}

void Prize::fromYaml(const YAML::Node& iNode) {
	m_id = static_cast<uint32_t>(std::max(0, yamlValue(iNode, "id", 0)));
	m_designation = yamlValue(iNode, "designation", std::string{});
	m_donor = yamlValue(iNode, "donor", std::string{});
	setValue(yamlValue(iNode, "value", 0.0));
	setAttractiveness(static_cast<uint8_t>(
			std::clamp(yamlValue(iNode, "attractiveness", 0), 0, static_cast<int>(g_maxAttractiveness))));
	m_childFriendly = yamlValue(iNode, "child_friendly", true);
	setSet(static_cast<uint8_t>(std::clamp(yamlValue(iNode, "set", 0), 0, static_cast<int>(g_maxSet))));
}

auto announcedPrizes(const prizes_type& iPrizes, const bool iGroupBySets) -> std::vector<AnnouncedPrize> {
	std::vector<AnnouncedPrize> announced;
	uint32_t current = 1;
	for (const auto& prize: iPrizes) {
		// The set is tracked on every article, named or not: an empty line keeps the run going.
		if (prize.getSet() > 0)
			current = prize.getSet();
		if (prize.getDesignation().empty())
			continue;
		announced.push_back({.prize = &prize, .set = iGroupBySets ? current : 0U, .firstOfSet = false});
	}
	if (!iGroupBySets)
		return announced;
	std::ranges::stable_sort(announced, [](const AnnouncedPrize& iLeft, const AnnouncedPrize& iRight) -> bool {
		return iLeft.set < iRight.set;
	});
	for (size_t i = 0; i < announced.size(); ++i)
		announced[i].firstOfSet = i == 0 || announced[i].set != announced[i - 1].set;
	return announced;
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
