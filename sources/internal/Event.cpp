/**
 * @file Event.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "core/Event.h"

namespace evl::core {

// ---- Serialisation ----
void Event::read(std::istream& is) {
    is.read(reinterpret_cast<char*>(&status), sizeof(status));
    sizeType l, i;
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    organizerName.resize(l);
    for(i= 0; i < l; ++i) is.read(&(organizerName[i]), charSize);
    string temp;
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    temp.resize(l);
    for(i= 0; i < l; ++i) is.read(&(temp[i]), charSize);
    organizerLogo= temp;
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    name.resize(l);
    for(i= 0; i < l; ++i) is.read(&(name[i]), charSize);
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    temp.resize(l);
    for(i= 0; i < l; ++i) is.read(&(temp[i]), charSize);
    logo= temp;
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    location.resize(l);
    for(i= 0; i < l; ++i) is.read(&(location[i]), charSize);
    roundsType::size_type lv, iv;
    is.read(reinterpret_cast<char*>(&lv), sizeof(lv));
    gameRounds.resize(lv);
    for(iv= 0; iv < lv; ++iv) gameRounds[iv].read(is);
    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));
}

void Event::write(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&status), sizeof(status));
    sizeType l, i;
    l= organizerName.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(organizerName[i]), charSize);
    l= organizerLogo.string().size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(organizerLogo.string()[i]), charSize);
    l= name.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(name[i]), charSize);
    l= logo.string().size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(logo.string()[i]), charSize);
    l= location.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(location[i]), charSize);

    roundsType::size_type lv, iv;
    lv= gameRounds.size();
    os.write(reinterpret_cast<char*>(&lv), sizeof(lv));
    for(iv= 0; iv < lv; ++iv) gameRounds[iv].write(os);

    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));

    // TODO: gérer les cartons
}

void Event::checkValidConfig() {
    if(organizerName.empty() || name.empty()) {
        status= Status::Invalid;
        return;
    }
    if(gameRounds.empty()) {
        status= Status::MissingParties;
        return;
    }
    status= Status::Ready;
}

bool Event::isEditable() const {
    return status == Status::Invalid || status == Status::MissingParties || status == Status::Ready;
}

// ---- manipulation des Données propres ----
void Event::setOrganizerName(const std::string& _name) {
    if(!isEditable())
        return;
    organizerName= _name;
    checkValidConfig();
}

void Event::setName(const std::string& _name) {
    if(!isEditable())
        return;
    name= _name;
    checkValidConfig();
}

void Event::setLocation(const std::string& _location) {
    if(!isEditable())
        return;
    location= _location;
    checkValidConfig();
}

void Event::setLogo(const std::filesystem::path& _logo) {
    if(!isEditable())
        return;
    logo= _logo;
    checkValidConfig();
}

void Event::setOrganizerLogo(const std::filesystem::path& _logo) {
    if(!isEditable())
        return;
    organizerLogo= _logo;
    checkValidConfig();
}

// ----- Manipulation des rounds ----
void Event::pushGameRound(const GameRound& round) {
    if(!isEditable())
        return;
    gameRounds.push_back(round);
    checkValidConfig();
}

void Event::deleteRoundByIndex(const uint16_t& idx) {
    if(!isEditable())
        return;
    gameRounds.erase(std::next(gameRounds.begin(), idx));
}

void Event::swapRoundByIndex(const uint16_t& idx, const uint16_t& idx2) {
    if(!isEditable())
        return;
    std::swap(gameRounds[idx], gameRounds[idx2]);
}

Event::roundsType::iterator Event::findFirstNotFinished() {
    return std::find_if(gameRounds.begin(), gameRounds.end(), [](GameRound gr) { return gr.getStatus() != GameRound::Status::Finished; });
}

Event::roundsType::iterator Event::getGameRound(const uint16_t& idx) {
    return std::next(gameRounds.begin(), idx);
}

int Event::getCurrentIndex() {
    unsigned int i= findFirstNotFinished() - gameRounds.begin();
    if(i >= gameRounds.size())
        return -1;
    return i;
}

// ----- Action sur le flow -----

void Event::startCurrentRound() {
    if(status != Status::GameStart)
        return;
    auto it= findFirstNotFinished();
    it->startGameRound();
    if(it->getStatus() == GameRound::Status::Started) {
        status= Status::GameRunning;
    }
}

void Event::addWinnerToCurrentRound(const uint32_t w) {
    if(status != Status::GameRunning)
        return;
    auto it= findFirstNotFinished();
    it->addWinner(w);
    if(it->getStatus() == GameRound::Status::DisplayResult) {
        status= Status::GameFinished;
    }
}

void Event::closeCurrentRound() {
    if(status != Status::GameFinished)
        return;
    auto it= findFirstNotFinished();
    it->closeGameRound();
    if(it->getStatus() == GameRound::Status::Finished) {
        status= Status::GameStart;
        ++it;
        if(it == endRounds()) {
            status= Status::Finished;
            end   = clock::now();
        }
    }
}

// ----- Flow de l'événement -----
void Event::startEvent() {
    if(status != Status::Ready)
        return;
    start = clock::now();
    status= Status::EventStarted;
}

void Event::ActiveFirstRound() {
    if(status != Status::EventStarted)
        return;
    status= Status::GameStart;
}

void Event::pauseEvent() {
    if(status == Status::GameFinished)
        status= Status::Paused;
    if(status == Status::GameRunning)
        status= Status::GamePaused;
}

void Event::resumeEvent() {
    if(status == Status::Paused)
        status= Status::GameFinished;
    if(status == Status::GamePaused)
        status= Status::GameRunning;
}

}// namespace evl::core
