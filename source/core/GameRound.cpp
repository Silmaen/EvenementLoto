/**
 * @file GameRound.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "GameRound.h"

#include "EnumLabel.h"
#include "StreamRead.h"

#include "Log.h"
#include "StringUtils.h"
#include "utilities.h"

namespace evl::core {

namespace {
constexpr std::array<std::pair<GameRound::Type, std::string_view>, 8> g_typeLabels{{
		{GameRound::Type::OneQuine, "Simple quine"},
		{GameRound::Type::TwoQuines, "Double quine"},
		{GameRound::Type::FullCard, "Gros lot"},
		{GameRound::Type::OneQuineFullCard, "une quine et carton"},
		{GameRound::Type::OneTwoQuineFullCard, "normale"},
		{GameRound::Type::Enfant, "Enfant"},
		{GameRound::Type::Inverse, "Inverse"},
		{GameRound::Type::Pause, "Pause"},
}};

constexpr std::array<std::pair<GameRound::Status, std::string_view>, 4> g_statusLabels{{
		{GameRound::Status::Ready, "prêt"},
		{GameRound::Status::Running, "démarré"},
		{GameRound::Status::PostScreen, "écran de fin"},
		{GameRound::Status::Done, "terminé"},
}};
}// namespace

// --- constructeurs ----
GameRound::GameRound(const Type& iType) { setType(iType); }

// ---- manipulation du type de partie ----
auto GameRound::getTypeStr() const -> std::string { return std::string(enumLabel(g_typeLabels, m_type)); }

void GameRound::setType(const Type& iType) {
	if (!isEditable()) {
		log_warn("Impossible de modifier le type d'une partie non éditable");
		return;
	}
	m_type = iType;
	m_subGames.clear();
	switch (m_type) {
		case Type::OneQuine:
			m_subGames.emplace_back(SubGameRound::Type::OneQuine);
			break;
		case Type::TwoQuines:
			m_subGames.emplace_back(SubGameRound::Type::TwoQuines);
			break;
		case Type::FullCard:
			m_subGames.emplace_back(SubGameRound::Type::FullCard);
			break;
		case Type::OneQuineFullCard:
			m_subGames.emplace_back(SubGameRound::Type::OneQuine);
			m_subGames.emplace_back(SubGameRound::Type::FullCard);
			break;
		case Type::OneTwoQuineFullCard:
			m_subGames.emplace_back(SubGameRound::Type::OneQuine);
			m_subGames.emplace_back(SubGameRound::Type::TwoQuines);
			m_subGames.emplace_back(SubGameRound::Type::FullCard);
			break;
		case Type::Enfant:
			m_subGames.emplace_back(SubGameRound::Type::OneQuine);
			break;
		case Type::Inverse:
			m_subGames.emplace_back(SubGameRound::Type::FullCard);
			break;
		case Type::Pause:
			break;
		case Type::Invalid:
			log_warn("GameRound set to invalid type!");
			break;
	}
}

// ---- manipulation du statut ----
auto GameRound::getStatusStr() const -> std::string { return std::string(enumLabel(g_statusLabels, m_status)); }

// ---- flux du jeu ----

void GameRound::nextStatus() {
	const auto sub = getCurrentSubRound();
	switch (m_status) {
		case Status::Ready:
			m_start = clock::now();
			m_status = Status::Running;
			sub->nextStatus();
			break;
		case Status::Running:
			if (sub == m_subGames.end()) {
				m_end = clock::now();
				m_status = Status::PostScreen;
			} else {
				sub->nextStatus();
			}
			break;
		case Status::PostScreen:
			m_status = Status::Done;
			break;
		case Status::Done:// last status
			break;
		case Status::Invalid:
			log_warn("GameRound in invalid status, cannot advance!");
			break;
	}
}

auto GameRound::getStateString() const -> std::string {
	std::string result = std::format("Partie {} ({})", getId(), getTypeStr());
	if (m_type != Type::Pause) {
		result += std::format(" - {}", getStatusStr());
		if (m_status == Status::Running) {
			const auto sub = getCurrentSubRound();
			if (sub != m_subGames.cend())
				result += std::format(" - {} - {}", sub->getTypeStr(), sub->getStatusStr());
		}
	}
	return result;
}

void GameRound::addPickedNumber(const uint8_t& iNumber) {
	if (m_status == Status::Running)
		getCurrentSubRound()->addPickedNumber(iNumber);
}

void GameRound::removeLastPick() {
	if (m_status == Status::Running)
		getCurrentSubRound()->removeLastPick();
}

void GameRound::addWinner(const std::string& iWinner) {
	if (m_status != Status::Running)
		return;
	const auto sub = getCurrentSubRound();
	sub->setWinner(iWinner);
	if (sub->isFinished())
		nextStatus();
}

// ---- Serialisation ----
void GameRound::read(std::istream& iBs, const int iFileVersion) {
	if (std::cmp_greater(iFileVersion, getSaveVersion())) {
		iBs.setstate(std::ios::failbit);
		return;
	}
	if (iFileVersion < 3) {//----UNCOVER----
		m_id = 0;//----UNCOVER----
	} else if (!readRaw(iBs, m_id)) {
		return;
	}
	if (!readEnum(iBs, m_type) || !readEnum(iBs, m_status))
		return;
	if (!readRaw(iBs, m_start) || !readRaw(iBs, m_end))
		return;
	draws_type draws;
	if (iFileVersion < 4 && !readVector(iBs, draws))//----UNCOVER----
		return;//----UNCOVER----
	sub_rounds_type::size_type subCount = 0;
	if (!readLength(iBs, subCount, g_maxSerializedCount))
		return;
	m_subGames.clear();
	m_subGames.resize(subCount);
	for (auto& sub: m_subGames) {
		sub.read(iBs, iFileVersion);
		if (!iBs.good())
			return;
	}
	if (iFileVersion < 4 && subCount != 0) {//----UNCOVER----
		// faking sub game picking
		const auto part = static_cast<uint32_t>(draws.size() / subCount);//----UNCOVER----
		for (draws_type::size_type i = 0; i < draws.size(); ++i) {//----UNCOVER----
			if (part == 0)
				break;//----UNCOVER----
			m_subGames[i % part].addPickedNumber(draws[i]);//----UNCOVER----
		}//----UNCOVER----
	}//----UNCOVER----
	m_diapoPath = std::filesystem::path{};
	m_diapoDelay = 0;
	if (m_type == Type::Pause && iFileVersion > 4) {
		std::string diapo;
		if (!readString(iBs, diapo))
			return;
		if (diapo.empty())
			return;
		m_diapoPath = diapo;
		if (!readRaw(iBs, m_diapoDelay))
			return;
	}
}

void GameRound::write(std::ostream& iBs) const {
	iBs.write(reinterpret_cast<const char*>(&m_id), sizeof(m_id));
	iBs.write(reinterpret_cast<const char*>(&m_type), sizeof(m_type));
	iBs.write(reinterpret_cast<const char*>(&m_status), sizeof(m_status));
	iBs.write(reinterpret_cast<const char*>(&m_start), sizeof(m_start));
	iBs.write(reinterpret_cast<const char*>(&m_end), sizeof(m_end));
	const sub_rounds_type::size_type l2 = m_subGames.size();
	iBs.write(reinterpret_cast<const char*>(&l2), sizeof(sub_rounds_type::size_type));
	for (sub_rounds_type::size_type i = 0; i < l2; ++i) m_subGames[i].write(iBs);
	if (m_type == Type::Pause) {
		const std::string::size_type l3 = m_diapoPath.string().size();
		iBs.write(reinterpret_cast<const char*>(&l3), sizeof(l3));
		if (l3 > 0) {
			for (std::string::size_type i = 0; i < l3; ++i)
				iBs.write(&m_diapoPath.string()[i], sizeof(std::string::value_type));
			iBs.write(reinterpret_cast<const char*>(&m_diapoDelay), sizeof(double));
		}
	}
}

auto GameRound::toJson() const -> Json::Value {
	Json::Value sub;
	for (const auto& game: m_subGames) { sub.append(game.toJson()); }
	Json::Value result;
	result["type"] = getTypeStr();
	result["Id"] = m_id;
	result["subGames"] = sub;
	return result;
}

void GameRound::fromJson(const Json::Value& iJson) {
	m_type = enumFromLabel(g_typeLabels, iJson.get("type", "").asString(), m_type);
	m_id = iJson.get("Id", 0).asInt();
	m_subGames.clear();
	for (auto& jj: iJson.get("subGames", Json::Value::null)) { m_subGames.emplace_back().fromJson(jj); }
}

auto GameRound::toYaml() const -> YAML::Node {
	YAML::Node node;
	node["type"] = getTypeStr();
	node["Id"] = m_id;
	YAML::Node subGamesNode;
	for (const auto& game: m_subGames) { subGamesNode.push_back(game.toYaml()); }
	node["subGames"] = subGamesNode;
	return node;
}

void GameRound::fromYaml(const YAML::Node& iNode) {
	m_type = enumFromLabel(g_typeLabels, iNode["type"].as<std::string>(), m_type);
	m_id = iNode["Id"].as<int>();
	m_subGames.clear();
	for (const auto& jj: iNode["subGames"]) {
		SubGameRound sgr;
		sgr.fromYaml(jj);
		m_subGames.push_back(sgr);
	}
}

auto GameRound::isEditable() const -> bool { return m_status == Status::Ready; }

auto GameRound::isCurrentSubRoundLast() const -> bool {
	if (m_subGames.size() <= 1)
		return true;
	if (!m_subGames[m_subGames.size() - 2].isFinished())
		return false;
	if (!m_subGames[m_subGames.size() - 1].isFinished())
		return true;
	return false;
}

auto GameRound::getCurrentSubRound() -> std::vector<SubGameRound>::iterator {
	return std::ranges::find_if(m_subGames,
								[](const SubGameRound& iSubGameRound) -> bool { return !iSubGameRound.isFinished(); });
}

auto GameRound::getCurrentSubRound() const -> std::vector<SubGameRound>::const_iterator {
	return std::ranges::find_if(m_subGames,
								[](const SubGameRound& iSubGameRound) -> bool { return !iSubGameRound.isFinished(); });
}

auto GameRound::getSubRound(const uint32_t iIndex) -> std::vector<SubGameRound>::iterator {
	if (iIndex >= m_subGames.size()) {
		log_warn("getSubRound: index {} out of bounds (size {})", iIndex, m_subGames.size());
		return m_subGames.end();
	}
	return std::next(m_subGames.begin(), iIndex);
}
auto GameRound::getSubRound(const uint32_t iIndex) const -> std::vector<SubGameRound>::const_iterator {
	if (iIndex >= m_subGames.size()) {
		log_warn("getSubRound: index {} out of bounds (size {})", iIndex, m_subGames.size());
		return m_subGames.cend();
	}
	return std::next(m_subGames.cbegin(), iIndex);
}

auto GameRound::getName() const -> std::string {
	std::stringstream res;
	res << "Partie";
	if (m_id > 0)
		res << " " << m_id;
	if (m_type != Type::OneTwoQuineFullCard)
		res << " " << getTypeStr();
	return res.str();
}

auto GameRound::getAllDraws() const -> draws_type {
	draws_type displayDraws;
	for (const auto& sub: m_subGames) {
		displayDraws.insert(displayDraws.end(), sub.getDraws().begin(), sub.getDraws().end());
	}
	return displayDraws;
}

auto GameRound::getDrawStr() const -> std::string {
	std::string result;
	for (const auto& sub: m_subGames) {
		if (sub.getDraws().empty())
			break;
		result += std::format("{}: {}\n", sub.getTypeStr(), join(sub.getDraws(), " "));
	}
	return result;
}

auto GameRound::getWinnerStr() const -> std::string {
	std::string result;
	for (const auto& sub: m_subGames) {
		if (sub.getWinner().empty())
			break;
		result += std::format("{}: {}\n", sub.getTypeStr(), sub.getWinner());
	}
	return result;
}

void GameRound::setDiapo(const std::string& iPath, const double iDelai) {
	if (!isEditable()) {
		log_warn("Impossible de modifier le diaporama d'une partie non éditable");
		return;
	}
	m_diapoPath = iPath;
	m_diapoDelay = iDelai;
}

auto GameRound::getDiapo() const -> std::tuple<std::filesystem::path, double> { return {m_diapoPath, m_diapoDelay}; }

}// namespace evl::core
