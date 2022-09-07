/**
 * @file GameRound.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "GameRound.h"

namespace evl::core {

const std::unordered_map<GameRound::Type, string> GameRound::TypeConvert= {
        {Type::OneQuine, "Simple quine"},
        {Type::TwoQuines, "Double quine"},
        {Type::FullCard, "Gros lot"},
        {Type::OneQuineFullCard, "une quine et carton"},
        {Type::OneTwoQuineFullCard, "normale"},
        {Type::Enfant, "Enfant"},
        {Type::Inverse, "Inverse"},
};

// --- constructeurs ----
GameRound::GameRound(GameRound::Type t) {
    setType(t);
}

// ---- manipulation du type de partie ----
string GameRound::getTypeStr() const {
    if(TypeConvert.contains(type)) {
        return TypeConvert.at(type);
    }
    return "inconnu";
}

void GameRound::setType(const Type& t) {
    if(!isEditable())
        return;
    type= t;
    subGames.clear();
    switch(type) {
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
    }
}

// ---- manipulation du statut ----
string GameRound::getStatusStr() const {
    switch(status) {
    case Status::Ready:
        return "prête";
    case Status::Started:
        return "démarrée";
    case Status::Finished:
        return "finie";
    case Status::DisplayResult:
        return "en affichage";
    }
    return "Statut inconnu";
}

// ---- flux du jeu ----
void GameRound::startGameRound() {
    if(status != Status::Ready)
        return;// start allowed only if ready
    start = clock::now();
    status= Status::Started;
}

void GameRound::addPickedNumber(const uint8_t& num) {
    if(status != Status::Started)
        return;
    draws.push_back(num);
}

void GameRound::removeLastPick() {
    if(status != Status::Started)
        return;
    if(draws.size() == 0)
        return;
    draws.pop_back();
}

void GameRound::addWinner(uint32_t win) {
    if(status != Status::Started)
        return;
    auto i= getCurrentSubRound();
    i->setWinner(win);
    // on vérifie qu’on a encore des sous-parties à faire
    i= getCurrentSubRound();
    if(i == subGames.end()) {
        end   = clock::now();
        status= Status::DisplayResult;
    }
}

void GameRound::closeGameRound() {
    if(status != Status::DisplayResult)
        return;
    status= Status::Finished;
}

// ---- Serialisation ----
void GameRound::read(std::istream& is, int file_version) {
    if(file_version < 3)
        Id= 0;
    else
        is.read(reinterpret_cast<char*>(&Id), sizeof(Id));
    is.read(reinterpret_cast<char*>(&type), sizeof(type));
    is.read(reinterpret_cast<char*>(&status), sizeof(status));
    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));
    drawsType::size_type l;
    is.read(reinterpret_cast<char*>(&l), sizeof(drawsType::size_type));
    draws.resize(l);
    for(drawsType::size_type i= 0; i < l; ++i)
        is.read(reinterpret_cast<char*>(&(draws[i])), sizeof(drawsType::value_type));
    subRoundsType::size_type l2;
    is.read(reinterpret_cast<char*>(&l2), sizeof(subRoundsType::size_type));
    subGames.resize(l2);
    for(subRoundsType::size_type i= 0; i < l2; ++i)
        subGames[i].read(is, file_version);
}

void GameRound::write(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&Id), sizeof(Id));
    os.write(reinterpret_cast<const char*>(&type), sizeof(type));
    os.write(reinterpret_cast<const char*>(&status), sizeof(status));
    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));
    drawsType::size_type l= draws.size();
    os.write(reinterpret_cast<const char*>(&l), sizeof(drawsType::size_type));
    for(drawsType::size_type i= 0; i < l; ++i)
        os.write(reinterpret_cast<const char*>(&(draws[i])), sizeof(drawsType::value_type));
    subRoundsType::size_type l2= subGames.size();
    os.write(reinterpret_cast<const char*>(&l2), sizeof(subRoundsType::size_type));
    for(subRoundsType::size_type i= 0; i < l2; ++i)
        subGames[i].write(os);
}

json GameRound::to_json() const {
    nlohmann::json sub;
    for(auto& game: subGames) {
        sub.push_back(game.to_json());
    }
    return json{{"type", getTypeStr()}, {"Id", Id}, {"subGames", sub}, {"draws", draws}};
}

void GameRound::from_json(const json& j) {
    string srType;
    j.at("type").get_to(srType);
    for(auto& s: TypeConvert) {
        if(s.second == srType) {
            type= s.first;
            break;
        }
    }
    j.at("Id").get_to(Id);
    subGames.clear();
    for(auto& jj: j.at("subGames")) {
        subGames.emplace_back().from_json(jj);
    }
}

bool GameRound::isEditable() const {
    return status == Status::Ready;
}

bool GameRound::isCurrentSubRoundLast() const {
    if(subGames.size() <= 1) return true;
    if(subGames[subGames.size() - 2].getWinner() == 0) return false;
    if(subGames[subGames.size() - 1].getWinner() == 0) return true;
    return false;
}

std::vector<SubGameRound>::iterator GameRound::getCurrentSubRound() {
    return std::find_if(subGames.begin(), subGames.end(), [](const SubGameRound& s) { return s.getWinner() == 0; });
}

std::vector<SubGameRound>::const_iterator GameRound::getCurrentCSubRound() const {
    return std::find_if(subGames.cbegin(), subGames.cend(), [](const SubGameRound& s) { return s.getWinner() == 0; });
}

std::vector<SubGameRound>::iterator GameRound::getSubRound(uint32_t index) {
    return std::next(subGames.begin(), index);
}

string GameRound::getName() const {
    std::stringstream res;
    res << "Partie";
    if(Id > 0)
        res << " " << Id;
    if(type != Type::OneTwoQuineFullCard)
        res << " " << TypeConvert.at(type);
    return res.str();
}

}// namespace evl::core
