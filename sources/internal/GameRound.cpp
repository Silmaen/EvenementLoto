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
    case Type::None: return "Aucun type défini";
    case Type::OneQuine: return "Une Quine";
    case Type::TwoQuines: return "Deux Quine";
    case Type::FullCard: return "Carton Plein";
    case Type::Inverse: return "Inverse";
    }
    return "Type de partie inconnu";
}

std::string GameRound::getStatusStr() const {
    switch(status) {
    case Status::Invalid: return "Partie invalide";
    case Status::Ready: return "Partie Prête";
    case Status::Started: return "Partie Démarrée";
    case Status::Finished: return "Partie finie";
    }
    return "Statut inconnu";
}

void GameRound::startGameRound() {
    if(status != Status::Ready) return;// start allowed only if ready
    start= std::chrono::system_clock::now();
    updateStatus();
}

void GameRound::endGameRound() {
    if(status != Status::Started) return;// start allowed only if already started
    end= std::chrono::system_clock::now();
    updateStatus();
}

void GameRound::read(std::istream& is) {
    is.read(reinterpret_cast<char*>(&type), sizeof(type));
    is.read(reinterpret_cast<char*>(&status), sizeof(status));
    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));
    std::vector<uint8_t>::size_type l;
    is.read(reinterpret_cast<char*>(&l), sizeof(std::vector<uint8_t>::size_type));
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

}// namespace evl::core
