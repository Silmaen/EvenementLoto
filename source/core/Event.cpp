/**
 * @file Event.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "Event.h"
#include <fstream>

namespace evl::core {

// ---- Serialisation ----
void Event::read(std::istream& is, int) {
    uint16_t saveVersion;
    is.read(reinterpret_cast<char*>(&saveVersion), sizeof(uint16_t));
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
    // version 2
    if(saveVersion > 1) {
        is.read(reinterpret_cast<char*>(&l), sizeof(l));
        rules.resize(l);
        for(i= 0; i < l; ++i) is.read(&(rules[i]), charSize);
    }
    // version 3
    if(saveVersion > 2) {
        is.read(reinterpret_cast<char*>(&l), sizeof(l));
        sanityRules.resize(l);
        for(i= 0; i < l; ++i) is.read(&(sanityRules[i]), charSize);
    }
    // version 1
    roundsType::size_type lv, iv;
    is.read(reinterpret_cast<char*>(&lv), sizeof(lv));
    gameRounds.resize(lv);
    for(iv= 0; iv < lv; ++iv) gameRounds[iv].read(is, saveVersion);
    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));
}

void Event::write(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&currentSaveVersion), sizeof(uint16_t));
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
    // version >= 2
    l= rules.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(rules[i]), charSize);
    // version >= 3
    l= sanityRules.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(sanityRules[i]), charSize);
    // version >= 1
    roundsType::size_type lv, iv;
    lv= gameRounds.size();
    os.write(reinterpret_cast<char*>(&lv), sizeof(lv));
    for(iv= 0; iv < lv; ++iv) gameRounds[iv].write(os);

    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));

    // TODO: gérer les cartons
}

json Event::to_json() const {
    nlohmann::json sub;
    for(auto& game: gameRounds) {
        sub.push_back(game.to_json());
    }
    return json{{"rounds", sub}};
}

void Event::from_json(const json& j) {
    gameRounds.clear();
    for(auto& jj: j.at("rounds")) {
        gameRounds.emplace_back().from_json(jj);
    }
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
    json j;
    fileRead >> j;
    from_json(j);
    fileRead.close();
}

void Event::checkValidConfig() {
    if(organizerName.empty() || name.empty()) {
        status        = Status::Invalid;
        previousStatus= status;
        return;
    }
    if(gameRounds.empty()) {
        status        = Status::MissingParties;
        previousStatus= status;
        return;
    }
    status        = Status::Ready;
    previousStatus= status;
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

void Event::setLogo(const path& _logo) {
    if(!isEditable())
        return;
    if(_logo.empty() || _logo.is_relative() || basePath.empty())
        logo= _logo;
    else
        logo= relative(_logo, basePath);
    checkValidConfig();
}

void Event::setOrganizerLogo(const path& _logo) {
    if(!isEditable())
        return;
    if(_logo.empty() || _logo.is_relative() || basePath.empty())
        organizerLogo= _logo;
    else
        organizerLogo= relative(_logo, basePath);
    checkValidConfig();
}

void Event::setBasePath(const path& p) {
    path tLogo   = getLogoFull();
    path tOrgLogo= getOrganizerLogoFull();
    if(is_directory(p))
        basePath= p;
    else
        basePath= p.parent_path();
    if(tLogo.empty() || tLogo.is_relative() || basePath.empty())
        logo= tLogo;
    else
        logo= relative(tLogo, basePath);
    if(tOrgLogo.empty() || tOrgLogo.is_relative() || basePath.empty())
        organizerLogo= tOrgLogo;
    else
        organizerLogo= relative(tOrgLogo, basePath);
}

void Event::setRules(const string& newRules) {
    if(!isEditable())
        return;
    rules= newRules;
    checkValidConfig();
}

void Event::setSanityRules(const string& newRules) {
    if(!isEditable())
        return;
    sanityRules= newRules;
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
    return std::find_if(gameRounds.begin(), gameRounds.end(), [](const GameRound& gr) { return gr.getStatus() != GameRound::Status::Finished; });
}

Event::roundsType::iterator Event::getGameRound(const uint16_t& idx) {
    return std::next(gameRounds.begin(), idx);
}

int Event::getCurrentIndex() {
    uint64_t i= static_cast<uint64_t>(findFirstNotFinished() - gameRounds.begin());
    if(i >= gameRounds.size())
        return -1;
    return static_cast<int>(i);
}

// ----- Action sur le flow -----

void Event::startCurrentRound() {
    if(status != Status::GameStart)
        return;
    auto it= findFirstNotFinished();
    it->startGameRound();
    if(it->getStatus() == GameRound::Status::Started) {
        changeStatus(Status::GameRunning);
    }
}

void Event::addWinnerToCurrentRound(const uint32_t w) {
    if(status != Status::GameRunning)
        return;
    auto it= findFirstNotFinished();
    it->addWinner(w);
    if(it->getStatus() == GameRound::Status::DisplayResult) {
        changeStatus(Status::GameFinished);
    }
}

void Event::closeCurrentRound() {
    if(status != Status::GameFinished)
        return;
    auto it= findFirstNotFinished();
    it->closeGameRound();
    if(it->getStatus() == GameRound::Status::Finished) {
        changeStatus(Status::GameStart);
        ++it;
        if(it == endRounds()) {
            changeStatus(Status::Finished);
            end= clock::now();
        }
    }
}

// ----- Flow de l'événement -----
void Event::startEvent() {
    if(status != Status::Ready)
        return;
    start= clock::now();
    changeStatus(Status::EventStarted);
}

void Event::ActiveFirstRound() {
    if(status != Status::EventStarted)
        return;
    changeStatus(Status::GameStart);
}

void Event::pauseEvent() {
    if(status == Status::GameFinished || status == Status::GameRunning)
        changeStatus(Status::Paused);
}

void Event::displayRules() {
    if(isEditable())
        return;
    changeStatus(Status::DisplayRules);
}

void Event::displaySanity() {
    if(isEditable())
        return;
    changeStatus(Status::DisplaySanity);
}

void Event::resumeEvent() {
    if(status == Status::Paused || status == Status::DisplayRules || status == Status::DisplaySanity)
        restoreStatus();
}

void Event::changeStatus(const Status& newStatus) {
    previousStatus= status;
    status        = newStatus;
}

void Event::restoreStatus() {
    status= previousStatus;
}

Serializable::~Serializable()= default;

}// namespace evl::core
