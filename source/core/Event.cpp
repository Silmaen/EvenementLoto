/**
 * @file Event.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Event.h"

#include "Log.h"

namespace evl::core {

namespace {

const std::unordered_map<Event::Status, const char*> g_statusConvert = {
		{Event::Status::Invalid, "invalide"},
		{Event::Status::MissingParties, "manque les parties"},
		{Event::Status::Ready, "prêt"},
		{Event::Status::EventStarting, "démarré"},
		{Event::Status::GameRunning, "en cours"},
		{Event::Status::DisplayRules, "en affichage des règles"},
		{Event::Status::EventEnding, "finalisation"},
		{Event::Status::Finished, "fini"},
};

}// namespace

auto Event::getStatusStr() const -> std::string { return g_statusConvert.at(m_status); }

// ---- Serialisation ----
void Event::read(std::istream& iBs, int) {
	uint16_t save_version = 0;
	iBs.read(reinterpret_cast<char*>(&save_version), sizeof(uint16_t));
	log_debug("Version des données du stream: {}, version courante: {}", save_version, g_currentSaveVersion);
	if (save_version > g_currentSaveVersion)
		return;// incompatible
	iBs.read(reinterpret_cast<char*>(&m_status), sizeof(m_status));
	std::string::size_type l = 0;
	std::string::size_type i = 0;
	iBs.read(reinterpret_cast<char*>(&l), sizeof(l));
	m_organizerName.resize(l);
	for (i = 0; i < l; ++i) iBs.read(&m_organizerName[i], sizeof(std::string::value_type));
	std::string temp;
	iBs.read(reinterpret_cast<char*>(&l), sizeof(l));
	temp.resize(l);
	for (i = 0; i < l; ++i) iBs.read(&temp[i], sizeof(std::string::value_type));
	m_organizerLogo = temp;
	iBs.read(reinterpret_cast<char*>(&l), sizeof(l));
	m_name.resize(l);
	for (i = 0; i < l; ++i) iBs.read(&m_name[i], sizeof(std::string::value_type));
	iBs.read(reinterpret_cast<char*>(&l), sizeof(l));
	temp.resize(l);
	for (i = 0; i < l; ++i) iBs.read(&temp[i], sizeof(std::string::value_type));
	m_logo = temp;
	iBs.read(reinterpret_cast<char*>(&l), sizeof(l));
	m_location.resize(l);
	for (i = 0; i < l; ++i) iBs.read(&m_location[i], sizeof(std::string::value_type));
	// version 2
	if (save_version > 1) {
		iBs.read(reinterpret_cast<char*>(&l), sizeof(l));
		m_rules.resize(l);
		for (i = 0; i < l; ++i) iBs.read(&m_rules[i], sizeof(std::string::value_type));
	}
	// version 3
	if (save_version > 2 && save_version < 4) {//----UNCOVER----
		std::string srules;//----UNCOVER----
		iBs.read(reinterpret_cast<char*>(&l), sizeof(l));//----UNCOVER----
		srules.resize(l);//----UNCOVER----
		for (i = 0; i < l; ++i) iBs.read(&srules[i], sizeof(std::string::value_type));//----UNCOVER----
	}//----UNCOVER----
	// version 1
	rounds_type::size_type lv = 0;
	iBs.read(reinterpret_cast<char*>(&lv), sizeof(lv));
	m_gameRounds.resize(lv);
	for (rounds_type::size_type iv = 0; iv < lv; ++iv) m_gameRounds[iv].read(iBs, save_version);
	log_info("Event lu et contenant {} parties", lv);
	iBs.read(reinterpret_cast<char*>(&m_start), sizeof(m_start));
	iBs.read(reinterpret_cast<char*>(&m_end), sizeof(m_end));
	log_info("Event in state: {}", getStateString());
}

void Event::write(std::ostream& oBs) const {
	oBs.write(reinterpret_cast<const char*>(&g_currentSaveVersion), sizeof(uint16_t));
	oBs.write(reinterpret_cast<const char*>(&m_status), sizeof(m_status));
	std::string::size_type l = 0;
	std::string::size_type i = 0;
	l = m_organizerName.size();
	oBs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) oBs.write(&m_organizerName[i], sizeof(std::string::value_type));
	l = m_organizerLogo.string().size();
	oBs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) oBs.write(&m_organizerLogo.string()[i], sizeof(std::string::value_type));
	l = m_name.size();
	oBs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) oBs.write(&m_name[i], sizeof(std::string::value_type));
	l = m_logo.string().size();
	oBs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) oBs.write(&m_logo.string()[i], sizeof(std::string::value_type));
	l = m_location.size();
	oBs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) oBs.write(&m_location[i], sizeof(std::string::value_type));
	// version >= 2
	l = m_rules.size();
	oBs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) oBs.write(&m_rules[i], sizeof(std::string::value_type));
	// version >= 1
	rounds_type::size_type lv = 0;
	lv = m_gameRounds.size();
	oBs.write(reinterpret_cast<char*>(&lv), sizeof(lv));
	for (rounds_type::size_type iv = 0; iv < lv; ++iv) m_gameRounds[iv].write(oBs);

	oBs.write(reinterpret_cast<const char*>(&m_start), sizeof(m_start));
	oBs.write(reinterpret_cast<const char*>(&m_end), sizeof(m_end));
}

auto Event::toJson() const -> Json::Value {
	Json::Value sub;
	for (const auto& game: m_gameRounds) { sub.append(game.toJson()); }
	Json::Value result;
	result["rounds"] = sub;
	return result;
}

void Event::fromJson(const Json::Value& iJson) {
	m_gameRounds.clear();
	for (auto& jj: iJson.get("rounds", Json::Value::null)) { m_gameRounds.emplace_back().fromJson(jj); }
}

auto Event::toYaml() const -> YAML::Node {
	YAML::Node node;
	YAML::Node roundsNode;
	for (const auto& game: m_gameRounds) { roundsNode.push_back(game.toYaml()); }
	node["rounds"] = roundsNode;
	return node;
}

void Event::fromYaml(const YAML::Node& iNode) {
	m_gameRounds.clear();
	for (const auto& jj: iNode["rounds"]) {
		GameRound gr;
		gr.fromYaml(jj);
		m_gameRounds.push_back(gr);
	}
}

void Event::exportJSON(const std::filesystem::path& iFile) const {
	std::ofstream file_save;
	file_save.open(iFile, std::ios::out | std::ios::binary);
	file_save << std::setw(4) << toJson();
	file_save.close();
}

void Event::importJSON(const std::filesystem::path& iFile) {
	std::ifstream file_read;
	file_read.open(iFile, std::ios::in | std::ios::binary);
	Json::Value j;
	file_read >> j;
	fromJson(j);
	file_read.close();
}

void Event::exportYaml(const std::filesystem::path& iFile) const {
	YAML::Emitter out;
	out << toYaml();
	std::ofstream fileOut(iFile);
	fileOut << out.c_str();
	fileOut.close();
}

void Event::importYaml(const std::filesystem::path& iFile) {
	const YAML::Node data = YAML::LoadFile(iFile.string());
	fromYaml(data);
}

void Event::checkValidConfig() {
	if (m_organizerName.empty() || m_name.empty()) {
		m_status = Status::Invalid;
		m_previousStatus = m_status;
		return;
	}
	if (m_gameRounds.empty()) {
		m_status = Status::MissingParties;
		m_previousStatus = m_status;
		return;
	}
	m_status = Status::Ready;
	m_previousStatus = m_status;
}

auto Event::isEditable() const -> bool {
	return m_status == Status::Invalid || m_status == Status::MissingParties || m_status == Status::Ready;
}

// ---- manipulation des Données propres ----
void Event::setOrganizerName(const std::string& iName) {
	if (!isEditable())
		return;
	m_organizerName = iName;
	checkValidConfig();
}

void Event::setName(const std::string& iName) {
	if (!isEditable())
		return;
	m_name = iName;
	checkValidConfig();
}

void Event::setLocation(const std::string& iLocation) {
	if (!isEditable())
		return;
	m_location = iLocation;
	checkValidConfig();
}

void Event::setLogo(const std::filesystem::path& iLogo) {
	if (!isEditable())
		return;
	if (iLogo.empty() || iLogo.is_relative() || m_basePath.empty())
		m_logo = iLogo;
	else
		m_logo = relative(iLogo, m_basePath);
	checkValidConfig();
}

void Event::setOrganizerLogo(const std::filesystem::path& iLogo) {
	if (!isEditable())
		return;
	if (iLogo.empty() || iLogo.is_relative() || m_basePath.empty())
		m_organizerLogo = iLogo;
	else
		m_organizerLogo = relative(iLogo, m_basePath);
	checkValidConfig();
}

void Event::setBasePath(const std::filesystem::path& iBasePath) {
	const std::filesystem::path t_logo = getLogoFull();
	const std::filesystem::path t_org_logo = getOrganizerLogoFull();
	if (is_directory(iBasePath))
		m_basePath = iBasePath;
	else
		m_basePath = iBasePath.parent_path();
	if (t_logo.empty() || t_logo.is_relative() || m_basePath.empty())
		m_logo = t_logo;
	else
		m_logo = relative(t_logo, m_basePath);
	if (t_org_logo.empty() || t_org_logo.is_relative() || m_basePath.empty())
		m_organizerLogo = t_org_logo;
	else
		m_organizerLogo = relative(t_org_logo, m_basePath);
}

void Event::setRules(const std::string& iNewRules) {
	if (!isEditable())
		return;
	m_rules = iNewRules;
	checkValidConfig();
}

// ----- Manipulation des rounds ----
void Event::pushGameRound(const GameRound& iRound) {
	if (!isEditable())
		return;
	m_gameRounds.push_back(iRound);
	checkValidConfig();
}

void Event::deleteRoundByIndex(const uint16_t& iIndex) {
	if (!isEditable())
		return;
	m_gameRounds.erase(std::next(m_gameRounds.begin(), iIndex));
}

void Event::swapRoundByIndex(const uint16_t& iIndex, const uint16_t& iIndex2) {
	if (!isEditable())
		return;
	std::swap(m_gameRounds[iIndex], m_gameRounds[iIndex2]);
}

auto Event::getCurrentCGameRound() const -> rounds_type::const_iterator {
	return std::ranges::find_if(
			m_gameRounds, [](const GameRound& iGr) -> bool { return iGr.getStatus() != GameRound::Status::Done; });
}

auto Event::getCurrentGameRound() -> rounds_type::iterator {
	return std::ranges::find_if(
			m_gameRounds, [](const GameRound& iGr) -> bool { return iGr.getStatus() != GameRound::Status::Done; });
}

auto Event::getGameRound(const uint32_t& iIndex) -> rounds_type::iterator {
	return std::next(m_gameRounds.begin(), iIndex);
}

auto Event::getCurrentGameRoundIndex() const -> int {
	const uint64_t i = static_cast<uint64_t>(getCurrentCGameRound() - m_gameRounds.begin());
	if (i >= m_gameRounds.size())
		return -1;
	return static_cast<int>(i);
}

// ----- Action sur le flow -----

//NOLINTBEGIN(misc-no-recursion)
void Event::nextState() {
	const auto status_save = m_status;
	m_changed = false;
	const auto sub = getCurrentGameRound();
	switch (m_status) {
		case Status::Invalid:
		case Status::MissingParties:
			checkValidConfig();// rien à faire si c'est invalide !
			break;
		case Status::Ready:
			m_status = Status::EventStarting;
			m_start = clock::now();
			break;
		case Status::EventStarting:
			m_status = Status::GameRunning;
			sub->nextStatus();
			break;
		case Status::DisplayRules:
			m_status = Status::GameRunning;
			break;
		case Status::GameRunning:
			if (sub == m_gameRounds.end()) {
				m_status = Status::EventEnding;
			} else {
				sub->nextStatus();
				if (sub->isFinished()) {
					m_end = clock::now();
					nextState();
				}
				m_changed = true;
			}
			break;
		case Status::EventEnding:
			m_status = Status::Finished;
			break;
		case Status::Finished:
			break;
	}
	if (status_save != m_status)
		m_changed = true;
	if (m_changed)
		log_info("Event switching to {}", getStateString());
	else
		log_info("Event stay in {}", getStateString());
}
//NOLINTEND(misc-no-recursion)

auto Event::getStateString() const -> std::string {
	std::string result = std::format("Event {}", getStatusStr());
	if (m_status == Status::GameRunning) {
		const auto sub = getCurrentCGameRound();
		result += std::format(" - {}", sub->getStateString());
	}
	return result;
}

void Event::addWinnerToCurrentRound(const std::string& iWin) {
	if (m_status != Status::GameRunning)
		return;
	const auto round = getCurrentGameRound();
	round->addWinner(iWin);
	if (round->isFinished()) {
		nextState();
	}
}

void Event::displayRules() {
	if (isEditable())
		return;
	changeStatus(Status::DisplayRules);
}

void Event::changeStatus(const Status& iNewStatus) {
	m_previousStatus = m_status;
	m_status = iNewStatus;
}

void Event::restoreStatus() { m_status = m_previousStatus; }

auto Event::getStats(const bool iWithoutChild) const -> Statistics {
	Statistics stat;
	for (const auto& round: m_gameRounds) {
		if (round.getType() == GameRound::Type::Pause)
			continue;
		if (round.getStatus() == GameRound::Status::Ready)
			break;
		if (iWithoutChild && round.getType() == GameRound::Type::Enfant)
			continue;
		stat.pushRound(round);
	}
	return stat;
}

Serializable::~Serializable() = default;

}// namespace evl::core
