/**
 * @file Event.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Event.h"

#include "EnumLabel.h"
#include "FileFormat.h"
#include "Log.h"
#include "StreamRead.h"
#include "StreamWrite.h"
#include "utilities.h"

namespace evl::core {

namespace {

constexpr std::array<std::pair<Event::Status, std::string_view>, 8> g_statusLabels{{
		{Event::Status::Invalid, "invalide"},
		{Event::Status::MissingParties, "manque les parties"},
		{Event::Status::Ready, "prêt"},
		{Event::Status::EventStarting, "démarré"},
		{Event::Status::GameRunning, "en cours"},
		{Event::Status::DisplayRules, "en affichage des règles"},
		{Event::Status::EventEnding, "finalisation"},
		{Event::Status::Finished, "fini"},
}};

}// namespace

auto Event::getStatusStr() const -> std::string { return std::string(enumLabel(g_statusLabels, m_status)); }

// ---- Serialisation ----
void Event::read(std::istream& iBs, const ReadContext&) {
	const auto frame = readFrame(iBs);
	if (!frame.valid) {
		log_warn("Fichier illisible : {}.", frame.framed ? "somme de contrôle invalide" : "contenu trop court");
		iBs.setstate(std::ios::failbit);
		return;
	}
	uint16_t saveVersion = frame.version;
	if (!frame.framed) {
		// An older file carries its version as the first field of the body instead of
		// in a header.
		std::istringstream head(frame.body, std::ios::in | std::ios::binary);
		if (!readRaw(head, saveVersion)) {
			iBs.setstate(std::ios::failbit);
			return;
		}
	}
	log_debug("Version des données du stream: {}, version courante: {}", saveVersion, getSaveVersion());
	if (saveVersion > getSaveVersion()) {
		log_warn("Version des données du stream ({}) supérieure à la version courante ({}), lecture impossible",
				 saveVersion, getSaveVersion());
		iBs.setstate(std::ios::failbit);
		return;
	}

	if (!readWith(frame, {.version = saveVersion, .wideEnums = false})) {
		// Version 6 and below is ambiguous: the serialized enumerations were narrowed to
		// `: uint8_t` without the save version being bumped, so two layouts share that
		// number. A second pass with four-byte enumerations is what brings the events
		// archived before that change back — and it costs nothing, the body is already
		// in memory.
		if (saveVersion >= g_firstFramedVersion || !readWith(frame, {.version = saveVersion, .wideEnums = true})) {
			iBs.setstate(std::ios::failbit);
			return;
		}
		log_info("Fichier de version {} lu avec des énumérations sur quatre octets.", saveVersion);
	}
	// The whole file was consumed on purpose, so the caller's stream is put back in a
	// good state: `good()` is how every call site tells a complete read from a partial
	// one.
	iBs.clear();
	log_info("Event in state: {}", getStateString());
}

auto Event::readWith(const FileFrame& iFrame, const ReadContext& iContext) -> bool {
	std::istringstream body(iFrame.body, std::ios::in | std::ios::binary);
	if (!iFrame.framed) {
		uint16_t ignored = 0;
		if (!readRaw(body, ignored))
			return false;
	}
	return readBody(body, iContext);
}

auto Event::readBody(std::istream& iBs, const ReadContext& iContext) -> bool {
	if (!readEnum(iBs, m_status, iContext.wideEnums))
		return false;
	std::string temp;
	if (!readString(iBs, m_organizerName) || !readString(iBs, temp))
		return false;
	m_organizerLogo = temp;
	if (!readString(iBs, m_name) || !readString(iBs, temp))
		return false;
	m_logo = temp;
	if (!readString(iBs, m_location))
		return false;
	// version 2
	if (iContext.version > 1 && !readString(iBs, m_rules))
		return false;
	// version 3
	if (iContext.version > 2 && iContext.version < 4) {
		std::string obsoleteRules;
		if (!readString(iBs, obsoleteRules))
			return false;
	}
	// version 1
	rounds_type::size_type roundCount = 0;
	if (!readLength(iBs, roundCount, g_maxSerializedCount))
		return false;
	m_gameRounds.clear();
	m_gameRounds.resize(roundCount);
	for (auto& round: m_gameRounds) {
		round.read(iBs, iContext);
		if (!iBs.good())
			return false;
	}
	log_info("Event lu et contenant {} parties", roundCount);
	return readTimePoint(iBs, m_start) && readTimePoint(iBs, m_end);
}


void Event::write(std::ostream& oBs) const {
	std::ostringstream body(std::ios::out | std::ios::binary);
	writeEnum(body, m_status);
	writeString(body, m_organizerName);
	writeString(body, m_organizerLogo.string());
	writeString(body, m_name);
	writeString(body, m_logo.string());
	writeString(body, m_location);
	// version >= 2
	writeString(body, m_rules);
	// version >= 1
	writeLength(body, m_gameRounds.size());
	for (const auto& round: m_gameRounds) round.write(body);
	writeTimePoint(body, m_start);
	writeTimePoint(body, m_end);
	writeFrame(oBs, getSaveVersion(), body.str());
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
	std::ofstream file_save(iFile, std::ios::out | std::ios::binary);
	if (!file_save.is_open()) {
		log_warn("Failed to open file '{}' for JSON export.", iFile.string());
		return;
	}
	file_save << std::setw(4) << toJson();
}

void Event::importJSON(const std::filesystem::path& iFile) {
	std::ifstream file_read(iFile, std::ios::in | std::ios::binary);
	if (!file_read.is_open()) {
		log_warn("Failed to open file '{}' for JSON import.", iFile.string());
		return;
	}
	Json::Value j;
	file_read >> j;
	fromJson(j);
}

void Event::exportYaml(const std::filesystem::path& iFile) const {
	YAML::Emitter out;
	out << toYaml();
	std::ofstream fileOut(iFile);
	if (!fileOut.is_open()) {
		log_warn("Failed to open file '{}' for YAML export.", iFile.string());
		return;
	}
	fileOut << out.c_str();
}

void Event::importYaml(const std::filesystem::path& iFile) {
	if (!exists(iFile)) {
		log_warn("File '{}' does not exist for YAML import.", iFile.string());
		return;
	}
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
	if (isFinished()) {
		log_warn("Impossible d'ajouter un round d'un événement terminé");
		return;
	}
	m_gameRounds.push_back(iRound);
	checkValidConfig();
}

void Event::deleteRoundByIndex(const uint16_t& iIndex) {
	if (isFinished()) {
		log_warn("Impossible de supprimer un round d'un événement terminé");
		return;
	}
	if (iIndex >= m_gameRounds.size()) {
		log_warn("Index de round à supprimer hors limite");
		return;
	}
	if (!m_gameRounds[iIndex].isEditable()) {
		log_warn("Impossible de supprimer un round non éditable");
		return;
	}
	m_gameRounds.erase(std::next(m_gameRounds.begin(), iIndex));
}

void Event::swapRoundByIndex(const uint16_t& iIndex, const uint16_t& iIndex2) {
	if (isFinished()) {
		log_warn("Impossible de permuter des rounds d'un événement terminé");
		return;
	}
	if (iIndex >= m_gameRounds.size() || iIndex2 >= m_gameRounds.size()) {
		log_warn("Index de round à permuter hors limite");
		return;
	}
	if (!m_gameRounds[iIndex].isEditable() || !m_gameRounds[iIndex2].isEditable()) {
		log_warn("Impossible de permuter des rounds non éditables");
		return;
	}
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

auto Event::getNextCGameRound() const -> rounds_type::const_iterator {
	const auto it = getCurrentCGameRound();
	if (it == m_gameRounds.cend())
		return m_gameRounds.cend();
	return std::next(it);
}

auto Event::getNextGameRound() -> rounds_type::iterator {
	const auto it = getCurrentGameRound();
	if (it == m_gameRounds.end())
		return m_gameRounds.end();
	return std::next(it);
}

// ----- Action sur le flow -----

void Event::nextState() {
	bool keepGoing = true;
	while (keepGoing) {
		keepGoing = false;
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
						keepGoing = true;
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
}

auto Event::getStateString() const -> std::string {
	std::string result = std::format("Event '{}'", getStatusStr());
	if (m_status == Status::GameRunning) {
		const auto sub = getCurrentCGameRound();
		if (sub != m_gameRounds.cend())
			result = std::format("{} - {}", result, sub->getStateString());
	}
	return result;
}

void Event::addWinnerToCurrentRound(const std::string& iWin) {
	if (m_status != Status::GameRunning)
		return;
	const auto round = getCurrentGameRound();
	if (round == m_gameRounds.end())
		return;
	round->addWinner(iWin);
	if (round->isFinished()) {
		nextState();
	}
}

void Event::displayRules() {
	if (isEditable())
		return;
	if (m_status == Status::DisplayRules) {
		restoreStatus();
		log_info("Event restoring to {}", getStateString());
		return;
	}
	changeStatus(Status::DisplayRules);
	log_info("Event switching to {}", getStateString());
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

auto Event::getProgression() const -> float {
	// total number of sub-rounds to play (excluding pauses)
	uint32_t totalSubRounds = 0;
	uint32_t completedSubRounds = 0;
	for (const auto& round: m_gameRounds) {
		if (round.getType() == GameRound::Type::Pause)
			continue;
		for (auto subRound = round.beginSubRound(); subRound != round.endSubRound(); ++subRound) {
			++totalSubRounds;
			if (subRound->isFinished())
				++completedSubRounds;
		}
	}
	if (totalSubRounds == 0)
		return 0.0f;
	return static_cast<float>(completedSubRounds) / static_cast<float>(totalSubRounds);
}

auto Event::canDraw() const -> bool {
	if (m_status != Status::GameRunning)
		return false;
	const auto round = getCurrentCGameRound();
	if (round == m_gameRounds.end())
		return false;
	if (round->getStatus() != GameRound::Status::Running)
		return false;
	const auto subRound = round->getCurrentSubRound();
	if (subRound == round->endSubRound())
		return false;
	if (subRound->isFinished())
		return false;
	return true;
}

Serializable::~Serializable() = default;

}// namespace evl::core
