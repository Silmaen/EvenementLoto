/**
 * @file Event.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "Event.h"
#include <fstream>
#include <spdlog/spdlog.h>

namespace evl::core {

const std::unordered_map<Event::Status, string> Event::StatusConvert= {
        {Status::Invalid, "invalide"},
        {Status::MissingParties, "manque les parties"},
        {Status::Ready, "prêt"},
        {Status::EventStarting, "démarré"},
        {Status::GameRunning, "en cours"},
        {Status::DisplayRules, "en affichage des règles"},
        {Status::EventEnding, "finalisation"},
        {Status::Finished, "fini"},
};

string Event::getStatusStr() const {
    if(StatusConvert.contains(status)) {
        return StatusConvert.at(status);
    }
    return "inconnu";
}

// ---- Serialisation ----
void Event::read(std::istream& is, int) {
    uint16_t saveVersion;
    is.read(reinterpret_cast<char*>(&saveVersion), sizeof(uint16_t));
    spdlog::debug("Version des données du stream: {}, version courante: {}", saveVersion, currentSaveVersion);
    if(saveVersion > currentSaveVersion) return;// incompatible
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
    if(saveVersion > 2 && saveVersion < 4) {                  //----UNCOVER----
        string srules;                                        //----UNCOVER----
        is.read(reinterpret_cast<char*>(&l), sizeof(l));      //----UNCOVER----
        srules.resize(l);                                     //----UNCOVER----
        for(i= 0; i < l; ++i) is.read(&(srules[i]), charSize);//----UNCOVER----
    }                                                         //----UNCOVER----
    // version 1
    roundsType::size_type lv, iv;
    is.read(reinterpret_cast<char*>(&lv), sizeof(lv));
    gameRounds.resize(lv);
    for(iv= 0; iv < lv; ++iv) gameRounds[iv].read(is, saveVersion);
    spdlog::info("Event lu et contenant {} parties", lv);
    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));
    spdlog::info("Event in state: {}", getStateString());
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
    // version >= 1
    roundsType::size_type lv, iv;
    lv= gameRounds.size();
    os.write(reinterpret_cast<char*>(&lv), sizeof(lv));
    for(iv= 0; iv < lv; ++iv) gameRounds[iv].write(os);

    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));
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

Event::roundsType::const_iterator Event::getCurrentCGameRound() const {
    return std::find_if(gameRounds.begin(), gameRounds.end(), [](const GameRound& gr) { return gr.getStatus() != GameRound::Status::Done; });
}

Event::roundsType::iterator Event::getCurrentGameRound() {
    return std::find_if(gameRounds.begin(), gameRounds.end(), [](const GameRound& gr) { return gr.getStatus() != GameRound::Status::Done; });
}

Event::roundsType::iterator Event::getGameRound(const uint16_t& idx) {
    return std::next(gameRounds.begin(), idx);
}

int Event::getCurrentGameRoundIndex() const {
    uint64_t i= static_cast<uint64_t>(getCurrentCGameRound() - gameRounds.begin());
    if(i >= gameRounds.size())
        return -1;
    return static_cast<int>(i);
}

// ----- Action sur le flow -----

void Event::nextState() {
    auto statusSave= status;
    changed        = false;
    auto sub       = getCurrentGameRound();
    switch(status) {
    case Status::Invalid:
        checkValidConfig();// rien à faire si c'est invalide!
        break;
    case Status::MissingParties:
        checkValidConfig();// rien à faire si c'est invalide!
        break;
    case Status::Ready:
        status= Status::EventStarting;
        start = clock::now();
        break;
    case Status::EventStarting:
        status= Status::GameRunning;
        sub->nextStatus();
        break;
    case Status::DisplayRules:
        status= Status::GameRunning;
        break;
    case Status::GameRunning:
        if(sub == gameRounds.end()) {
            status= Status::EventEnding;
        } else {
            sub->nextStatus();
            if(sub->isFinished()) {
                end= clock::now();
                nextState();
            }
            changed= true;
        }
        break;
    case Status::EventEnding:
        status= Status::Finished;
        break;
    case Status::Finished:
        break;
    }
    if(statusSave != status)
        changed= true;
    if(changed)
        spdlog::info("Event switching to {}", getStateString());
    else
        spdlog::info("Event stay in {}", getStateString());
}

string Event::getStateString() const {
    string result= fmt::format("Event {}", getStatusStr());
    if(status == Status::GameRunning) {
        auto sub= getCurrentCGameRound();
        result+= fmt::format(" - {}", sub->getStateString());
    }
    return result;
}

void Event::addWinnerToCurrentRound(const std::string& win) {
    if(status != Status::GameRunning)
        return;
    auto round= getCurrentGameRound();
    round->addWinner(win);
    if(round->isFinished()) {
        nextState();
    }
}

void Event::displayRules() {
    if(isEditable())
        return;
    changeStatus(Status::DisplayRules);
}

void Event::changeStatus(const Status& newStatus) {
    previousStatus= status;
    status        = newStatus;
}

void Event::restoreStatus() {
    status= previousStatus;
}

Statistics Event::getStats(bool withoutChild) const {
    Statistics stat;
    for(const auto& round: gameRounds) {
        if(round.getType() == GameRound::Type::Pause)
            continue;
        if(round.getStatus() == GameRound::Status::Ready)
            break;
        if(withoutChild && round.getType() == GameRound::Type::Enfant)
            continue;
        stat.pushRound(round);
    }
    return stat;
}

Serializable::~Serializable()= default;

}// namespace evl::core
