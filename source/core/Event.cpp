/**
 * @file Event.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Event.h"
#include <spdlog/spdlog.h>

namespace evl::core {

const std::unordered_map<Event::Status, string> Event::StatusConvert = {
		{Status::Invalid, "invalide"},
		{Status::MissingParties, "manque les parties"},
		{Status::Ready, "prêt"},
		{Status::EventStarting, "démarré"},
		{Status::GameRunning, "en cours"},
		{Status::DisplayRules, "en affichage des règles"},
		{Status::EventEnding, "finalisation"},
		{Status::Finished, "fini"},
};

auto Event::getStatusStr() const -> string {
	if (StatusConvert.contains(status)) {
		return StatusConvert.at(status);
	}
	return "inconnu";
}

// ---- Serialisation ----
void Event::read(std::istream& bs, int) {
	uint16_t saveVersion = 0;
	bs.read(reinterpret_cast<char*>(&saveVersion), sizeof(uint16_t));
	spdlog::debug("Version des données du stream: {}, version courante: {}", saveVersion, currentSaveVersion);
	if (saveVersion > currentSaveVersion)
		return;// incompatible
	bs.read(reinterpret_cast<char*>(&status), sizeof(status));
	sizeType l = 0;
	sizeType i = 0;
	bs.read(reinterpret_cast<char*>(&l), sizeof(l));
	organizerName.resize(l);
	for (i = 0; i < l; ++i) bs.read(&organizerName[i], charSize);
	string temp;
	bs.read(reinterpret_cast<char*>(&l), sizeof(l));
	temp.resize(l);
	for (i = 0; i < l; ++i) bs.read(&temp[i], charSize);
	organizerLogo = temp;
	bs.read(reinterpret_cast<char*>(&l), sizeof(l));
	name.resize(l);
	for (i = 0; i < l; ++i) bs.read(&name[i], charSize);
	bs.read(reinterpret_cast<char*>(&l), sizeof(l));
	temp.resize(l);
	for (i = 0; i < l; ++i) bs.read(&temp[i], charSize);
	logo = temp;
	bs.read(reinterpret_cast<char*>(&l), sizeof(l));
	location.resize(l);
	for (i = 0; i < l; ++i) bs.read(&location[i], charSize);
	// version 2
	if (saveVersion > 1) {
		bs.read(reinterpret_cast<char*>(&l), sizeof(l));
		rules.resize(l);
		for (i = 0; i < l; ++i) bs.read(&rules[i], charSize);
	}
	// version 3
	if (saveVersion > 2 && saveVersion < 4) {//----UNCOVER----
		string srules;//----UNCOVER----
		bs.read(reinterpret_cast<char*>(&l), sizeof(l));//----UNCOVER----
		srules.resize(l);//----UNCOVER----
		for (i = 0; i < l; ++i) bs.read(&srules[i], charSize);//----UNCOVER----
	}//----UNCOVER----
	// version 1
	roundsType::size_type lv = 0;
	bs.read(reinterpret_cast<char*>(&lv), sizeof(lv));
	gameRounds.resize(lv);
	for (roundsType::size_type iv = 0; iv < lv; ++iv) gameRounds[iv].read(bs, saveVersion);
	spdlog::info("Event lu et contenant {} parties", lv);
	bs.read(reinterpret_cast<char*>(&start), sizeof(start));
	bs.read(reinterpret_cast<char*>(&end), sizeof(end));
	spdlog::info("Event in state: {}", getStateString());
}

void Event::write(std::ostream& bs) const {
	bs.write(reinterpret_cast<const char*>(&currentSaveVersion), sizeof(uint16_t));
	bs.write(reinterpret_cast<const char*>(&status), sizeof(status));
	sizeType l = 0;
	sizeType i = 0;
	l = organizerName.size();
	bs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) bs.write(&organizerName[i], charSize);
	l = organizerLogo.string().size();
	bs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) bs.write(&organizerLogo.string()[i], charSize);
	l = name.size();
	bs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) bs.write(&name[i], charSize);
	l = logo.string().size();
	bs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) bs.write(&logo.string()[i], charSize);
	l = location.size();
	bs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) bs.write(&location[i], charSize);
	// version >= 2
	l = rules.size();
	bs.write(reinterpret_cast<char*>(&l), sizeof(l));
	for (i = 0; i < l; ++i) bs.write(&rules[i], charSize);
	// version >= 1
	roundsType::size_type lv = 0;
	lv = gameRounds.size();
	bs.write(reinterpret_cast<char*>(&lv), sizeof(lv));
	for (roundsType::size_type iv = 0; iv < lv; ++iv) gameRounds[iv].write(bs);

	bs.write(reinterpret_cast<const char*>(&start), sizeof(start));
	bs.write(reinterpret_cast<const char*>(&end), sizeof(end));
}

auto Event::to_json() const -> Json::Value {
	Json::Value sub;
	for (const auto& game: gameRounds) { sub.append(game.to_json()); }
	Json::Value result;
	result["rounds"] = sub;
	return result;
}

void Event::from_json(const Json::Value& j) {
	gameRounds.clear();
	for (auto& jj: j.get("rounds", Json::Value::null)) { gameRounds.emplace_back().from_json(jj); }
}

void Event::exportJSON(const path& file) const {
	std::ofstream fileSave;
	fileSave.open(file, std::ios::out | std::ios::binary);
	fileSave << std::setw(4) << to_json();
	fileSave.close();
}

void Event::importJSON(const path& file) {
	std::ifstream fileRead;
	fileRead.open(file, std::ios::in | std::ios::binary);
	Json::Value j;
	fileRead >> j;
	from_json(j);
	fileRead.close();
}

void Event::checkValidConfig() {
	if (organizerName.empty() || name.empty()) {
		status = Status::Invalid;
		previousStatus = status;
		return;
	}
	if (gameRounds.empty()) {
		status = Status::MissingParties;
		previousStatus = status;
		return;
	}
	status = Status::Ready;
	previousStatus = status;
}

auto Event::isEditable() const -> bool {
	return status == Status::Invalid || status == Status::MissingParties || status == Status::Ready;
}

// ---- manipulation des Données propres ----
void Event::setOrganizerName(const std::string& _name) {
	if (!isEditable())
		return;
	organizerName = _name;
	checkValidConfig();
}

void Event::setName(const std::string& _name) {
	if (!isEditable())
		return;
	name = _name;
	checkValidConfig();
}

void Event::setLocation(const std::string& _location) {
	if (!isEditable())
		return;
	location = _location;
	checkValidConfig();
}

void Event::setLogo(const path& _logo) {
	if (!isEditable())
		return;
	if (_logo.empty() || _logo.is_relative() || basePath.empty())
		logo = _logo;
	else
		logo = relative(_logo, basePath);
	checkValidConfig();
}

void Event::setOrganizerLogo(const path& _logo) {
	if (!isEditable())
		return;
	if (_logo.empty() || _logo.is_relative() || basePath.empty())
		organizerLogo = _logo;
	else
		organizerLogo = relative(_logo, basePath);
	checkValidConfig();
}

void Event::setBasePath(const path& p) {
	const path tLogo = getLogoFull();
	const path tOrgLogo = getOrganizerLogoFull();
	if (is_directory(p))
		basePath = p;
	else
		basePath = p.parent_path();
	if (tLogo.empty() || tLogo.is_relative() || basePath.empty())
		logo = tLogo;
	else
		logo = relative(tLogo, basePath);
	if (tOrgLogo.empty() || tOrgLogo.is_relative() || basePath.empty())
		organizerLogo = tOrgLogo;
	else
		organizerLogo = relative(tOrgLogo, basePath);
}

void Event::setRules(const string& newRules) {
	if (!isEditable())
		return;
	rules = newRules;
	checkValidConfig();
}

// ----- Manipulation des rounds ----
void Event::pushGameRound(const GameRound& round) {
	if (!isEditable())
		return;
	gameRounds.push_back(round);
	checkValidConfig();
}

void Event::deleteRoundByIndex(const uint16_t& idx) {
	if (!isEditable())
		return;
	gameRounds.erase(std::next(gameRounds.begin(), idx));
}

void Event::swapRoundByIndex(const uint16_t& idx, const uint16_t& idx2) {
	if (!isEditable())
		return;
	std::swap(gameRounds[idx], gameRounds[idx2]);
}

auto Event::getCurrentCGameRound() const -> roundsType::const_iterator {
	return std::ranges::find_if(gameRounds,
								[](const GameRound& gr) -> bool { return gr.getStatus() != GameRound::Status::Done; });
}

auto Event::getCurrentGameRound() -> roundsType::iterator {
	return std::ranges::find_if(gameRounds,
								[](const GameRound& gr) -> bool { return gr.getStatus() != GameRound::Status::Done; });
}

auto Event::getGameRound(const uint32_t& idx) -> roundsType::iterator { return std::next(gameRounds.begin(), idx); }

auto Event::getCurrentGameRoundIndex() const -> int {
	const uint64_t i = static_cast<uint64_t>(getCurrentCGameRound() - gameRounds.begin());
	if (i >= gameRounds.size())
		return -1;
	return static_cast<int>(i);
}

// ----- Action sur le flow -----

//NOLINTBEGIN(misc-no-recursion)
void Event::nextState() {
	const auto statusSave = status;
	changed = false;
	const auto sub = getCurrentGameRound();
	switch (status) {
		case Status::Invalid:
		case Status::MissingParties:
			checkValidConfig();// rien à faire si c'est invalide !
			break;
		case Status::Ready:
			status = Status::EventStarting;
			start = clock::now();
			break;
		case Status::EventStarting:
			status = Status::GameRunning;
			sub->nextStatus();
			break;
		case Status::DisplayRules:
			status = Status::GameRunning;
			break;
		case Status::GameRunning:
			if (sub == gameRounds.end()) {
				status = Status::EventEnding;
			} else {
				sub->nextStatus();
				if (sub->isFinished()) {
					end = clock::now();
					nextState();
				}
				changed = true;
			}
			break;
		case Status::EventEnding:
			status = Status::Finished;
			break;
		case Status::Finished:
			break;
	}
	if (statusSave != status)
		changed = true;
	if (changed)
		spdlog::info("Event switching to {}", getStateString());
	else
		spdlog::info("Event stay in {}", getStateString());
}
//NOLINTEND(misc-no-recursion)

auto Event::getStateString() const -> string {
	string result = std::format("Event {}", getStatusStr());
	if (status == Status::GameRunning) {
		const auto sub = getCurrentCGameRound();
		result += std::format(" - {}", sub->getStateString());
	}
	return result;
}

void Event::addWinnerToCurrentRound(const std::string& win) {
	if (status != Status::GameRunning)
		return;
	const auto round = getCurrentGameRound();
	round->addWinner(win);
	if (round->isFinished()) {
		nextState();
	}
}

void Event::displayRules() {
	if (isEditable())
		return;
	changeStatus(Status::DisplayRules);
}

void Event::changeStatus(const Status& newStatus) {
	previousStatus = status;
	status = newStatus;
}

void Event::restoreStatus() { status = previousStatus; }

auto Event::getStats(const bool withoutChild) const -> Statistics {
	Statistics stat;
	for (const auto& round: gameRounds) {
		if (round.getType() == GameRound::Type::Pause)
			continue;
		if (round.getStatus() == GameRound::Status::Ready)
			break;
		if (withoutChild && round.getType() == GameRound::Type::Enfant)
			continue;
		stat.pushRound(round);
	}
	return stat;
}

Serializable::~Serializable() = default;

}// namespace evl::core
