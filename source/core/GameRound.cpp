/**
 * @file GameRound.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "GameRound.h"

#include "Log.h"
#include "StringUtils.h"
#include "utilities.h"

namespace evl::core {

namespace {
const std::unordered_map<GameRound::Type, const char*> g_typeConvert = {
		{GameRound::Type::OneQuine, "Simple quine"},
		{GameRound::Type::TwoQuines, "Double quine"},
		{GameRound::Type::FullCard, "Gros lot"},
		{GameRound::Type::OneQuineFullCard, "une quine et carton"},
		{GameRound::Type::OneTwoQuineFullCard, "normale"},
		{GameRound::Type::Enfant, "Enfant"},
		{GameRound::Type::Inverse, "Inverse"},
		{GameRound::Type::Pause, "Pause"},
};

const std::unordered_map<GameRound::Status, const char*> g_statusConvert = {
		{GameRound::Status::Ready, "prêt"},
		{GameRound::Status::Running, "démarré"},
		{GameRound::Status::PostScreen, "écran de fin"},
		{GameRound::Status::Done, "terminé"},
};
}// namespace

// --- constructeurs ----
GameRound::GameRound(const Type& iType) { setType(iType); }

// ---- manipulation du type de partie ----
auto GameRound::getTypeStr() const -> std::string {
	if (g_typeConvert.contains(m_type)) {
		return g_typeConvert.at(m_type);
	}
	return "inconnu";
}

void GameRound::setType(const Type& iType) {
	if (!isEditable())
		return;
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
auto GameRound::getStatusStr() const -> std::string {
	if (g_statusConvert.contains(m_status)) {
		return g_statusConvert.at(m_status);
	}
	return "inconnu";
}

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
	std::string result = std::format("Partie {} {}", getId(), getName());
	if (m_type != Type::Pause) {
		result += std::format(" - {}", getStatusStr());
		if (m_status == Status::Running) {
			const auto sub = getCurrentSubRound();
			result += std::format(" - {} {}", sub->getTypeStr(), sub->getStatusStr());
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
	if (std::cmp_greater(iFileVersion, getSaveVersion()))
		return;
	if (iFileVersion < 3)//----UNCOVER----
		m_id = 0;//----UNCOVER----
	else//----UNCOVER----
		iBs.read(reinterpret_cast<char*>(&m_id), sizeof(m_id));
	iBs.read(reinterpret_cast<char*>(&m_type), sizeof(m_type));
	iBs.read(reinterpret_cast<char*>(&m_status), sizeof(m_status));
	iBs.read(reinterpret_cast<char*>(&m_start), sizeof(m_start));
	iBs.read(reinterpret_cast<char*>(&m_end), sizeof(m_end));
	draws_type::size_type l = 0;
	draws_type draws;
	if (iFileVersion < 4) {//----UNCOVER----
		iBs.read(reinterpret_cast<char*>(&l), sizeof(draws_type::size_type));//----UNCOVER----
		draws.resize(l);//----UNCOVER----
		for (draws_type::size_type i = 0; i < l; ++i)//----UNCOVER----
			iBs.read(reinterpret_cast<char*>(&draws[i]), sizeof(draws_type::value_type));//----UNCOVER----
	}//----UNCOVER----
	sub_rounds_type::size_type l2 = 0;
	iBs.read(reinterpret_cast<char*>(&l2), sizeof(sub_rounds_type::size_type));
	m_subGames.resize(l2);
	for (sub_rounds_type::size_type i = 0; i < l2; ++i) m_subGames[i].read(iBs, iFileVersion);
	if (iFileVersion < 4) {//----UNCOVER----
		// faking sub game picking
		const auto part = static_cast<uint32_t>(l / l2);//----UNCOVER----
		for (draws_type::size_type i = 0; i < l; ++i) {//----UNCOVER----
			if (part == 0)
				break;//----UNCOVER----
			m_subGames[i % part].addPickedNumber(draws[i]);//----UNCOVER----
		}//----UNCOVER----
	}//----UNCOVER----
	m_diapoPath = std::filesystem::path{};
	m_diapoDelay = 0;
	if (m_type == Type::Pause && iFileVersion > 4) {
		std::string::size_type len = 0;
		iBs.read(reinterpret_cast<char*>(&len), sizeof(std::string::size_type));
		if (len == 0) {
			m_diapoPath.clear();
			m_diapoDelay = 0.0;
		} else {
			std::string tmp;
			tmp.resize(len);
			for (std::string::size_type i = 0; i < len; i++)
				iBs.read(reinterpret_cast<char*>(&tmp[i]), sizeof(std::string::value_type));
			m_diapoPath = tmp;
			iBs.read(reinterpret_cast<char*>(&m_diapoDelay), sizeof(double));
		}
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
	std::string srType = iJson.get("type", "").asString();
	if (const auto result = std::ranges::find_if(
				g_typeConvert, [&srType](const auto& iItem) -> auto { return iItem.second == srType; });
		result != g_typeConvert.end())
		m_type = result->first;
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
	auto srType = iNode["type"].as<std::string>();
	if (const auto result = std::ranges::find_if(
				g_typeConvert, [&srType](const auto& iItem) -> auto { return iItem.second == srType; });
		result != g_typeConvert.end())
		m_type = result->first;
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
	return std::next(m_subGames.begin(), iIndex);
}
auto GameRound::getSubRound(const uint32_t iIndex) const -> std::vector<SubGameRound>::const_iterator {
	return std::next(m_subGames.begin(), iIndex);
}

auto GameRound::getName() const -> std::string {
	std::stringstream res;
	res << "Partie";
	if (m_id > 0)
		res << " " << m_id;
	if (m_type != Type::OneTwoQuineFullCard)
		res << " " << g_typeConvert.at(m_type);
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
	m_diapoPath = iPath;
	m_diapoDelay = iDelai;
}

auto GameRound::getDiapo() const -> std::tuple<std::filesystem::path, double> { return {m_diapoPath, m_diapoDelay}; }

}// namespace evl::core
