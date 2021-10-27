/**
 * @file GameRound.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "core/GameRound.h"

namespace evl::core {

// --- constructeurs ----
GameRound::GameRound(GameRound::Type t) {
    setType(t);
}

// ---- manipulation du type de partie ----
string GameRound::getTypeStr() const {
    switch(type) {
    case Type::Normal:
        return "Normal";
    case Type::Enfant:
        return "Enfant";
    case Type::Inverse:
        return "Inverse";
    }
    return "inconnu";
}

void GameRound::setType(const Type& t) {
    if(!isEditable())
        return;
    type= t;
    subGames.clear();
    switch(type) {
    case Type::Normal:
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
void GameRound::read(std::istream& is) {
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
        subGames[i].read(is);
}

void GameRound::write(std::ostream& os) const {
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

bool GameRound::isEditable() const {
    return status == Status::Ready;
}

std::vector<SubGameRound>::iterator GameRound::getCurrentSubRound() {
    return std::find_if(subGames.begin(), subGames.end(), [](const SubGameRound& s) { return s.getWinner() == 0; });
}

std::vector<SubGameRound>::const_iterator GameRound::getCurrentCSubRound() const {
    return std::find_if(subGames.cbegin(), subGames.cend(), [](const SubGameRound& s) { return s.getWinner() == 0; });
}

}// namespace evl::core
