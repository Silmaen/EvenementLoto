/**
 * @file Event.cpp
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "core/Event.h"

namespace evl::core {

void Event::read(std::istream& is) {
    std::string::size_type l, i;
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    organizerName.resize(l);
    for(i= 0; i < l; ++i) is.read(&(organizerName[i]), sizeof(std::string::value_type));

    std::string temp;
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    temp.resize(l);
    for(i= 0; i < l; ++i) is.read(&(temp[i]), sizeof(std::string::value_type));
    organizerLogo= temp;

    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    name.resize(l);
    for(i= 0; i < l; ++i) is.read(&(name[i]), sizeof(std::string::value_type));

    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    temp.resize(l);
    for(i= 0; i < l; ++i) is.read(&(temp[i]), sizeof(std::string::value_type));
    logo= temp;

    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    location.resize(l);
    for(i= 0; i < l; ++i) is.read(&(location[i]), sizeof(std::string::value_type));

    std::vector<GameRound>::size_type lv, iv;
    is.read(reinterpret_cast<char*>(&lv), sizeof(lv));
    gameRounds.resize(lv);
    for(iv= 0; iv < lv; ++iv) gameRounds[iv].read(is);

    updateStatus();

    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));

    // TODO: gérer les cartons
    updateStatus();
}

void Event::write(std::ostream& os) const {
    std::string::size_type l, i;
    l= organizerName.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(organizerName[i]), sizeof(std::string::value_type));
    l= organizerLogo.string().size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(organizerLogo.string()[i]), sizeof(std::string::value_type));
    l= name.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(name[i]), sizeof(std::string::value_type));
    l= logo.string().size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(logo.string()[i]), sizeof(std::string::value_type));
    l= location.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(location[i]), sizeof(std::string::value_type));

    std::vector<GameRound>::size_type lv, iv;
    lv= gameRounds.size();
    os.write(reinterpret_cast<char*>(&lv), sizeof(lv));
    for(iv= 0; iv < lv; ++iv) gameRounds[iv].write(os);

    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));

    // TODO: gérer les cartons
}

void Event::checkValidConfig() {
    if(!isEditable())
        return;
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

void Event::updateStatus() {
    checkValidConfig();
    if(status == Status::Invalid || status == Status::MissingParties)
        return;
    std::chrono::system_clock::time_point epoch{};
    if(start == epoch)
        return;
    if(paused && (gameRounds.front().getStatus() == GameRound::Status::Ready)) {
        status= Status::EventStarted;
        return;
    }
    itGameround gr= findFirstNotFinished();
    if(gr == getGREnd()) {
        status= Status::Finished;
        if(end == epoch)
            end= std::chrono::system_clock::now();
        return;
    }
    if(end != epoch) {
        status= Status::Finished;
        return;
    }
    if(paused) {
        status= Status::Paused;
        return;
    }
    if(gr->getStatus() == GameRound::Status::Ready) {
        status= Status::GameStart;
    }
    if(gr->getStatus() == GameRound::Status::Started) {
        status= Status::GameRunning;
    }
    if(gr->getStatus() == GameRound::Status::Result) {
        status= Status::GameFinished;
    }
    if(gr->getStatus() == GameRound::Status::Paused) {
        status= Status::GamePaused;
    }
}

bool Event::isEditable() const {
    return status == Status::Invalid || status == Status::MissingParties || status == Status::Ready;
}

void Event::setOrganizerName(const std::string& _name) {
    if(!isEditable())
        return;
    organizerName= _name;
    updateStatus();
}

void Event::setName(const std::string& _name) {
    if(!isEditable())
        return;
    name= _name;
    updateStatus();
}

void Event::setLocation(const std::string& _location) {
    if(!isEditable())
        return;
    location= _location;
    updateStatus();
}

void Event::setLogo(const std::filesystem::path& _logo) {
    if(!isEditable())
        return;
    logo= _logo;
    updateStatus();
}

void Event::setOrganizerLogo(const std::filesystem::path& _logo) {
    if(!isEditable())
        return;
    organizerLogo= _logo;
    updateStatus();
}

void Event::startEvent() {
    if(status != Status::Ready)
        return;
    start= std::chrono::system_clock::now();
    updateStatus();
}

void Event::pushGameRound(const GameRound& round) {
    if(!isEditable())
        return;
    if(round.getStatus() != GameRound::Status::Ready)
        return;
    gameRounds.push_back(round);
    updateStatus();
}

void Event::startCurrentRound() {
    if(status != Status::GameStart)
        return;
    findFirstNotFinished()->startGameRound();
    updateStatus();
}

void Event::endCurrentRound() {
    if(status != Status::GameRunning)
        return;
    findFirstNotFinished()->endGameRound();
    updateStatus();
}
void Event::closeCurrentRound() {
    if(status != Status::GameFinished)
        return;
    findFirstNotFinished()->closeGameRound();
    updateStatus();
}

void Event::pauseEvent() {
    if(status != Status::GameFinished && status != Status::GameRunning)
        return;
    if(status == Status::GameFinished)
        paused= true;
    if(status == Status::GameRunning) {
        findFirstNotFinished()->pauseGameRound();
    }
    updateStatus();
}

void Event::resumeEvent() {
    if(status != Status::Paused && status != Status::EventStarted && status != Status::GamePaused)
        return;
    paused= false;
    if(status == Status::GamePaused) {
        findFirstNotFinished()->resumeGameRound();
    }
    updateStatus();
}

void Event::stopEvent() {
    if(status != Status::GameFinished)
        return;
    end= std::chrono::system_clock::now();
    updateStatus();
}

Event::itGameround Event::findFirstNotFinished() {
    for(itGameround it= gameRounds.begin(); it != gameRounds.end(); ++it) {
        if(it->getStatus() != GameRound::Status::Finished)
            return it;
    }
    return gameRounds.end();
}
}// namespace evl::core
