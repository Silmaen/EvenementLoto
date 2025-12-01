/**
 * @file GameRound.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "GameRound.h"
#include "StringUtils.h"

#include <spdlog/spdlog.h>

namespace evl::core {

const std::unordered_map<GameRound::Type, string> GameRound::TypeConvert = {
		{Type::OneQuine, "Simple quine"},
		{Type::TwoQuines, "Double quine"},
		{Type::FullCard, "Gros lot"},
		{Type::OneQuineFullCard, "une quine et carton"},
		{Type::OneTwoQuineFullCard, "normale"},
		{Type::Enfant, "Enfant"},
		{Type::Inverse, "Inverse"},
		{Type::Pause, "Pause"},
};

const std::unordered_map<GameRound::Status, string> GameRound::StatusConvert = {
		{Status::Ready, "prêt"},
		{Status::Running, "démarré"},
		{Status::PostScreen, "écran de fin"},
		{Status::Done, "terminé"},
};

// --- constructeurs ----
GameRound::GameRound(const Type& t) { setType(t); }

// ---- manipulation du type de partie ----
auto GameRound::getTypeStr() const -> string {
	if (TypeConvert.contains(type)) {
		return TypeConvert.at(type);
	}
	return "inconnu";
}

void GameRound::setType(const Type& t) {
	if (!isEditable())
		return;
	type = t;
	subGames.clear();
	switch (type) {
		case Type::OneQuine:
			subGames.emplace_back(SubGameRound::Type::OneQuine);
			break;
		case Type::TwoQuines:
			subGames.emplace_back(SubGameRound::Type::TwoQuines);
			break;
		case Type::FullCard:
			subGames.emplace_back(SubGameRound::Type::FullCard);
			break;
		case Type::OneQuineFullCard:
			subGames.emplace_back(SubGameRound::Type::OneQuine);
			subGames.emplace_back(SubGameRound::Type::FullCard);
			break;
		case Type::OneTwoQuineFullCard:
			subGames.emplace_back(SubGameRound::Type::OneQuine);
			subGames.emplace_back(SubGameRound::Type::TwoQuines);
			subGames.emplace_back(SubGameRound::Type::FullCard);
			break;
		case Type::Enfant:
			subGames.emplace_back(SubGameRound::Type::OneQuine);
			break;
		case Type::Inverse:
			subGames.emplace_back(SubGameRound::Type::FullCard);
			break;
		case Type::Pause:
			break;
		case Type::Invalid:
			spdlog::warn("GameRound set to invalid type!");
			break;
	}
}

// ---- manipulation du statut ----
auto GameRound::getStatusStr() const -> string {
	if (StatusConvert.contains(status)) {
		return StatusConvert.at(status);
	}
	return "inconnu";
}

// ---- flux du jeu ----

void GameRound::nextStatus() {
	const auto sub = getCurrentSubRound();
	switch (status) {
		case Status::Ready:
			start = clock::now();
			status = Status::Running;
			sub->nextStatus();
			break;
		case Status::Running:
			if (sub == subGames.end()) {
				end = clock::now();
				status = Status::PostScreen;
			} else {
				sub->nextStatus();
			}
			break;
		case Status::PostScreen:
			status = Status::Done;
			break;
		case Status::Done:// last status
			break;
		case Status::Invalid:
			spdlog::warn("GameRound in invalid status, cannot advance!");
			break;
	}
}

auto GameRound::getStateString() const -> string {
	string result = std::format("Partie {} {}", getID(), getName());
	if (type != Type::Pause) {
		result += std::format(" - {}", getStatusStr());
		if (status == Status::Running) {
			const auto sub = getCurrentSubRound();
			result += std::format(" - {} {}", sub->getTypeStr(), sub->getStatusStr());
		}
	}
	return result;
}

void GameRound::addPickedNumber(const uint8_t& num) {
	if (status == Status::Running)
		getCurrentSubRound()->addPickedNumber(num);
}

void GameRound::removeLastPick() {
	if (status == Status::Running)
		getCurrentSubRound()->removeLastPick();
}

void GameRound::addWinner(const std::string& win) {
	if (status != Status::Running)
		return;
	const auto sub = getCurrentSubRound();
	sub->setWinner(win);
	if (sub->isFinished())
		nextStatus();
}

// ---- Serialisation ----
void GameRound::read(std::istream& bs, const int file_version) {
	if (std::cmp_greater(file_version, currentSaveVersion))
		return;
	if (file_version < 3)//----UNCOVER----
		Id = 0;//----UNCOVER----
	else//----UNCOVER----
		bs.read(reinterpret_cast<char*>(&Id), sizeof(Id));
	bs.read(reinterpret_cast<char*>(&type), sizeof(type));
	bs.read(reinterpret_cast<char*>(&status), sizeof(status));
	bs.read(reinterpret_cast<char*>(&start), sizeof(start));
	bs.read(reinterpret_cast<char*>(&end), sizeof(end));
	drawsType::size_type l = 0;
	drawsType draws;
	if (file_version < 4) {//----UNCOVER----
		bs.read(reinterpret_cast<char*>(&l), sizeof(drawsType::size_type));//----UNCOVER----
		draws.resize(l);//----UNCOVER----
		for (drawsType::size_type i = 0; i < l; ++i)//----UNCOVER----
			bs.read(reinterpret_cast<char*>(&draws[i]), sizeof(drawsType::value_type));//----UNCOVER----
	}//----UNCOVER----
	subRoundsType::size_type l2 = 0;
	bs.read(reinterpret_cast<char*>(&l2), sizeof(subRoundsType::size_type));
	subGames.resize(l2);
	for (subRoundsType::size_type i = 0; i < l2; ++i) subGames[i].read(bs, file_version);
	if (file_version < 4) {//----UNCOVER----
		// faking sub game picking
		const auto part = static_cast<uint32_t>(l / l2);//----UNCOVER----
		for (drawsType::size_type i = 0; i < l; ++i) {//----UNCOVER----
			if (part == 0)
				break;//----UNCOVER----
			subGames[i % part].addPickedNumber(draws[i]);//----UNCOVER----
		}//----UNCOVER----
	}//----UNCOVER----
	diapoPath = path{};
	diapoDelay = 0;
	if (type == Type::Pause && file_version > 4) {
		sizeType len = 0;
		bs.read(reinterpret_cast<char*>(&len), sizeof(sizeType));
		if (len == 0) {
			diapoPath.clear();
			diapoDelay = 0.0;
		} else {
			string tmp;
			tmp.resize(len);
			for (sizeType i = 0; i < len; i++) bs.read(reinterpret_cast<char*>(&tmp[i]), charSize);
			diapoPath = tmp;
			bs.read(reinterpret_cast<char*>(&diapoDelay), sizeof(double));
		}
	}
}

void GameRound::write(std::ostream& bs) const {
	bs.write(reinterpret_cast<const char*>(&Id), sizeof(Id));
	bs.write(reinterpret_cast<const char*>(&type), sizeof(type));
	bs.write(reinterpret_cast<const char*>(&status), sizeof(status));
	bs.write(reinterpret_cast<const char*>(&start), sizeof(start));
	bs.write(reinterpret_cast<const char*>(&end), sizeof(end));
	const subRoundsType::size_type l2 = subGames.size();
	bs.write(reinterpret_cast<const char*>(&l2), sizeof(subRoundsType::size_type));
	for (subRoundsType::size_type i = 0; i < l2; ++i) subGames[i].write(bs);
	if (type == Type::Pause) {
		const sizeType l3 = diapoPath.string().size();
		bs.write(reinterpret_cast<const char*>(&l3), sizeof(l3));
		if (l3 > 0) {
			for (sizeType i = 0; i < l3; ++i) bs.write(&diapoPath.string()[i], charSize);
			bs.write(reinterpret_cast<const char*>(&diapoDelay), sizeof(double));
		}
	}
}

auto GameRound::to_json() const -> Json::Value {
	Json::Value sub;
	for (const auto& game: subGames) { sub.append(game.to_json()); }
	Json::Value result;
	result["type"] = getTypeStr();
	result["Id"] = Id;
	result["subGames"] = sub;
	return result;
}

void GameRound::from_json(const Json::Value& j) {
	string srType = j.get("type", "").asString();
	if (const auto result = std::ranges::find_if(TypeConvert,
												 [&srType](const auto& item) -> auto { return item.second == srType; });
		result != TypeConvert.end())
		type = result->first;
	Id = j.get("Id", 0).asInt();
	subGames.clear();
	for (auto& jj: j.get("subGames", Json::Value::null)) { subGames.emplace_back().from_json(jj); }
}

auto GameRound::isEditable() const -> bool { return status == Status::Ready; }

auto GameRound::isCurrentSubRoundLast() const -> bool {
	if (subGames.size() <= 1)
		return true;
	if (!subGames[subGames.size() - 2].isFinished())
		return false;
	if (!subGames[subGames.size() - 1].isFinished())
		return true;
	return false;
}

auto GameRound::getCurrentSubRound() -> std::vector<SubGameRound>::iterator {
	return std::ranges::find_if(subGames, [](const SubGameRound& s) -> bool { return !s.isFinished(); });
}

auto GameRound::getCurrentSubRound() const -> std::vector<SubGameRound>::const_iterator {
	return std::ranges::find_if(subGames, [](const SubGameRound& s) -> bool { return !s.isFinished(); });
}

auto GameRound::getSubRound(uint32_t index) -> std::vector<SubGameRound>::iterator {
	return std::next(subGames.begin(), index);
}
auto GameRound::getSubRound(const uint32_t index) const -> std::vector<SubGameRound>::const_iterator {
	return std::next(subGames.begin(), index);
}

auto GameRound::getName() const -> string {
	std::stringstream res;
	res << "Partie";
	if (Id > 0)
		res << " " << Id;
	if (type != Type::OneTwoQuineFullCard)
		res << " " << TypeConvert.at(type);
	return res.str();
}

auto GameRound::getAllDraws() const -> drawsType {
	drawsType displayDraws;
	for (const auto& sub: subGames) {
		displayDraws.insert(displayDraws.end(), sub.getDraws().begin(), sub.getDraws().end());
	}
	return displayDraws;
}

auto GameRound::getDrawStr() const -> string {
	string result;
	for (const auto& sub: subGames) {
		if (sub.getDraws().empty())
			break;
		result += std::format("{}: {}\n", sub.getTypeStr(), join(sub.getDraws(), " "));
	}
	return result;
}

auto GameRound::getWinnerStr() const -> string {
	string result;
	for (const auto& sub: subGames) {
		if (sub.getWinner().empty())
			break;
		result += std::format("{}: {}\n", sub.getTypeStr(), sub.getWinner());
	}
	return result;
}

void GameRound::setDiapo(const string& path, const double delai) {
	diapoPath = path;
	diapoDelay = delai;
}

auto GameRound::getDiapo() const -> std::tuple<path, double> { return {diapoPath, diapoDelay}; }

}// namespace evl::core
