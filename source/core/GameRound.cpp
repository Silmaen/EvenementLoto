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
        {Type::Pause, "Pause"},
};

const std::unordered_map<GameRound::Status, string> GameRound::StatusConvert= {
        {Status::Started, "démarré"},
        {Status::Ready, "prêt"},
        {Status::DisplayResult, "affichage résultat"},
        {Status::Finished, "terminé"},
};
// --- constructeurs ----
GameRound::GameRound(const GameRound::Type& t) {
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
    case Type::Pause:
        break;
    }
}

// ---- manipulation du statut ----
string GameRound::getStatusStr() const {
    if(StatusConvert.contains(status)) {
        return StatusConvert.at(status);
    }
    return "inconnu";
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
    getCurrentSubRound()->addPickedNumber(num);
}

void GameRound::removeLastPick() {
    if(status != Status::Started)
        return;
    getCurrentSubRound()->removeLastPick();
}

void GameRound::addWinner(const std::string& win) {
    if(status != Status::Started)
        return;
    auto i= getCurrentSubRound();
    i->setWinner(win);
    // on vérifie qu’on a encore des sous-parties à faire
    ++i;
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
    if(file_version > currentSaveVersion) return;
    if(file_version < 3)//----UNCOVER----
        Id= 0;          //----UNCOVER----
    else                //----UNCOVER----
        is.read(reinterpret_cast<char*>(&Id), sizeof(Id));
    is.read(reinterpret_cast<char*>(&type), sizeof(type));
    is.read(reinterpret_cast<char*>(&status), sizeof(status));
    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));
    drawsType::size_type l= 0;
    drawsType draws;
    if(file_version < 4) {                                                               //----UNCOVER----
        is.read(reinterpret_cast<char*>(&l), sizeof(drawsType::size_type));              //----UNCOVER----
        draws.resize(l);                                                                 //----UNCOVER----
        for(drawsType::size_type i= 0; i < l; ++i)                                       //----UNCOVER----
            is.read(reinterpret_cast<char*>(&(draws[i])), sizeof(drawsType::value_type));//----UNCOVER----
    }                                                                                    //----UNCOVER----
    subRoundsType::size_type l2;
    is.read(reinterpret_cast<char*>(&l2), sizeof(subRoundsType::size_type));
    subGames.resize(l2);
    for(subRoundsType::size_type i= 0; i < l2; ++i)
        subGames[i].read(is, file_version);
    if(file_version < 4) {//----UNCOVER----
        // faking sub game picking
        uint32_t part= static_cast<uint32_t>(l / l2);    //----UNCOVER----
        for(drawsType::size_type i= 0; i < l; ++i) {     //----UNCOVER----
            subGames[i % part].addPickedNumber(draws[i]);//----UNCOVER----
        }                                                //----UNCOVER----
    }                                                    //----UNCOVER----
}

void GameRound::write(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&Id), sizeof(Id));
    os.write(reinterpret_cast<const char*>(&type), sizeof(type));
    os.write(reinterpret_cast<const char*>(&status), sizeof(status));
    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));
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
    return json{{"type", getTypeStr()}, {"Id", Id}, {"subGames", sub}};
}

void GameRound::from_json(const json& j) {
    string srType;
    j.at("type").get_to(srType);
    auto result= std::find_if(TypeConvert.begin(), TypeConvert.end(), [&srType](const auto& item) { return item.second == srType; });
    if(result != TypeConvert.end())
        type= result->first;
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
    if(subGames[subGames.size() - 2].getWinner().empty()) return false;
    if(subGames[subGames.size() - 1].getWinner().empty()) return true;
    return false;
}

std::vector<SubGameRound>::iterator GameRound::getCurrentSubRound() {
    return std::find_if(subGames.begin(), subGames.end(), [](const SubGameRound& s) { return s.getWinner().empty(); });
}

std::vector<SubGameRound>::const_iterator GameRound::getCurrentCSubRound() const {
    return std::find_if(subGames.cbegin(), subGames.cend(), [](const SubGameRound& s) { return s.getWinner().empty(); });
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

GameRound::drawsType GameRound::getAllDraws() const {
    drawsType displayDraws;
    for(const auto& sub: subGames) {
        displayDraws.insert(displayDraws.end(), sub.getDraws().begin(), sub.getDraws().end());
    }
    return displayDraws;
}

string GameRound::getDrawStr() const {
    string result;
    for(const auto& sub: subGames) {
        if(sub.getDraws().empty()) break;
        result+= fmt::format("{}: {}\n", sub.getTypeStr(), fmt::join(sub.getDraws(), " "));
    }
    return result;
}

string GameRound::getWinnerStr() const {
    string result;
    for(const auto& sub: subGames) {
        if(sub.getWinner().empty()) break;
        result+= fmt::format("{}: {}\n", sub.getTypeStr(), sub.getWinner());
    }
    return result;
}

}// namespace evl::core
