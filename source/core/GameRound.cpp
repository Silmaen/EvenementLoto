/**
 * @file GameRound.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "GameRound.h"
#include <spdlog/spdlog.h>

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
        {Status::Ready, "prêt"},
        {Status::Running, "démarré"},
        {Status::PostScreen, "écran de fin"},
        {Status::Done, "terminé"},
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

void GameRound::nextStatus() {
    auto sub= getCurrentSubRound();
    switch(status) {
    case Status::Ready:
        start = clock::now();
        status= Status::Running;
        sub->nextStatus();
        break;
    case Status::Running:
        if(sub == subGames.end()) {
            end   = clock::now();
            status= Status::PostScreen;
        } else {
            sub->nextStatus();
        }
        break;
    case Status::PostScreen:
        status= Status::Done;
        break;
    case Status::Done:// last status
        break;
    }
}

string GameRound::getStateString() const {
    string result= fmt::format("Partie {} {}", getID(), getName());
    if(type != Type::Pause) {
        result+= fmt::format(" - {}", getStatusStr());
        if(status == Status::Running) {
            auto sub= getCurrentSubRound();
            result+= fmt::format(" - {} {}", sub->getTypeStr(), sub->getStatusStr());
        }
    }
    return result;
}

void GameRound::addPickedNumber(const uint8_t& num) {
    if(status == Status::Running)
        getCurrentSubRound()->addPickedNumber(num);
}

void GameRound::removeLastPick() {
    if(status == Status::Running)
        getCurrentSubRound()->removeLastPick();
}

void GameRound::addWinner(const std::string& win) {
    if(status != Status::Running)
        return;
    auto sub= getCurrentSubRound();
    sub->setWinner(win);
    if(sub->isFinished())
        nextStatus();
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
    diapoPath = path{};
    diapoDelay= 0;
    if(type == Type::Pause && file_version > 4) {
        sizeType len;
        is.read(reinterpret_cast<char*>(&len), sizeof(sizeType));
        string tmp;
        tmp.resize(len);
        for(sizeType i= 0; i < l; i++)
            is.read(reinterpret_cast<char*>(&tmp[i]), charSize);
        diapoPath= tmp;
        is.read(reinterpret_cast<char*>(&diapoDelay), sizeof(double));
    }
}

void GameRound::write(std::ostream& os) const {
    spdlog::debug("Écriture d'un GameRound dans un stream");
    os.write(reinterpret_cast<const char*>(&Id), sizeof(Id));
    os.write(reinterpret_cast<const char*>(&type), sizeof(type));
    os.write(reinterpret_cast<const char*>(&status), sizeof(status));
    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));
    subRoundsType::size_type l2= subGames.size();
    os.write(reinterpret_cast<const char*>(&l2), sizeof(subRoundsType::size_type));
    for(subRoundsType::size_type i= 0; i < l2; ++i)
        subGames[i].write(os);
    if(type == Type::Pause) {
        sizeType l3= diapoPath.string().size();
        os.write(reinterpret_cast<const char*>(&l3), sizeof(l3));
        for(sizeType i= 0; i < l3; ++i) os.write(&(diapoPath.string()[i]), charSize);
        os.write(reinterpret_cast<const char*>(&diapoDelay), sizeof(double));
    }
    spdlog::debug("Fin d'écriture d'un GameRound dans un stream");
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
    if(!subGames[subGames.size() - 2].isFinished()) return false;
    if(!subGames[subGames.size() - 1].isFinished()) return true;
    return false;
}

std::vector<SubGameRound>::iterator GameRound::getCurrentSubRound() {
    return std::find_if(subGames.begin(), subGames.end(), [](const SubGameRound& s) { return !s.isFinished(); });
}

std::vector<SubGameRound>::const_iterator GameRound::getCurrentSubRound() const {
    return std::find_if(subGames.cbegin(), subGames.cend(), [](const SubGameRound& s) { return !s.isFinished(); });
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

void GameRound::setDiapo(const string& path, const double delai) {
    diapoPath = path;
    diapoDelay= delai;
}

std::tuple<path, double> GameRound::getDiapo() const {
    return {diapoPath, diapoDelay};
}

}// namespace evl::core
