/**
 * @file GameRound.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "core/GameRound.h"

namespace evl::core {

std::string GameRound::getTypeStr() const {
    switch(type) {
    case Type::None:
        return "Aucun type défini";
    case Type::OneQuine:
        return "Un quine";
    case Type::TwoQuines:
        return "Deux quines";
    case Type::FullCard:
        return "Carton plein";
    case Type::Inverse:
        return "Inverse";
    }
    return "Type de partie inconnu";
}

std::string GameRound::getStatusStr() {
    switch(status) {
    case Status::Invalid:
        return "Partie invalide";
    case Status::Ready:
        return "Partie prête";
    case Status::Started:
        return "Partie démarrée";
    case Status::Finished:
        return "Partie finie";
    case Status::Paused:
        return "Partie en pause";
    case Status::Result:
        return "Partie en affichage";
    }
    updateStatus();
    return "Statut inconnu";
}

std::string GameRound::getStatusStr() const {
    switch(status) {
    case Status::Invalid:
        return "Partie invalide";
    case Status::Ready:
        return "Partie prête";
    case Status::Started:
        return "Partie démarrée";
    case Status::Finished:
        return "Partie finie";
    case Status::Paused:
        return "Partie en pause";
    case Status::Result:
        return "Partie en affichage";
    }
    return "Statut inconnu";
}

void GameRound::startGameRound() {
    if(status != Status::Ready)
        return;// start allowed only if ready
    start= std::chrono::system_clock::now();
    updateStatus();
}

void GameRound::pauseGameRound() {
    if(status != Status::Started)
        return;
    paused= true;
    updateStatus();
}

void GameRound::resumeGameRound() {
    if(status != Status::Paused)
        return;
    paused= false;
    updateStatus();
}

void GameRound::endGameRound() {
    if(status != Status::Started)
        return;// start allowed only if already started
    end   = std::chrono::system_clock::now();
    paused= true;
    updateStatus();
}

void GameRound::closeGameRound() {
    if(status != Status::Result)
        return;
    paused= false;
    updateStatus();
}

void GameRound::updateStatus() {
    if(type == Type::None) {
        status= Status::Invalid;
        return;
    }
    datetype epoch{};
    if((start - epoch).count() == 0) {
        status= Status::Ready;
        end   = start;
    } else if((end - epoch).count() == 0) {
        status= Status::Started;
        if(paused)
            status= Status::Paused;
    } else {
        status= Status::Finished;
        if(paused)
            status= Status::Result;
    }
}

bool GameRound::isEditable() const {
    return status == Status::Invalid || status == Status::Ready;
}

void GameRound::read(std::istream& is) {
    is.read(reinterpret_cast<char*>(&type), sizeof(type));
    is.read(reinterpret_cast<char*>(&status), sizeof(status));
    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));
    std::vector<uint8_t>::size_type l;
    is.read(reinterpret_cast<char*>(&l), sizeof(std::vector<uint8_t>::size_type));
    Draws.resize(l);
    for(std::vector<uint8_t>::size_type i= 0; i < l; ++i)
        is.read(reinterpret_cast<char*>(&(Draws[i])), sizeof(std::vector<uint8_t>::value_type));
}

void GameRound::write(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&type), sizeof(type));
    os.write(reinterpret_cast<const char*>(&status), sizeof(status));
    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));
    std::vector<uint8_t>::size_type l= Draws.size();
    os.write(reinterpret_cast<const char*>(&l), sizeof(std::vector<uint8_t>::size_type));
    for(std::vector<uint8_t>::size_type i= 0; i < l; ++i)
        os.write(reinterpret_cast<const char*>(&(Draws[i])), sizeof(std::vector<uint8_t>::value_type));
}

void GameRound::addPickedNumber(const uint8_t& num) {
    if(status != Status::Started)
        return;
    Draws.push_back(num);
}

void GameRound::removeLastPick() {
    if(Draws.size() == 0)
        return;
    Draws.pop_back();
}

}// namespace evl::core
