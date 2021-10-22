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

void Event::updateStatus() {
    if(organizerName.empty() || name.empty()) {
        status= Status::Invalid;
        return;
    }
    if(gameRounds.empty()) {
        status= Status::MissingParties;
        return;
    }
    std::chrono::time_point<std::chrono::steady_clock> epoch{};
    if(start == epoch) {
        status= Status::Ready;
        return;
    }
    if(end == epoch) {
        status= Status::OnGoing;
        return;
    }
    status= Status::Finished;
}

bool Event::isEditable() const {
    return status != Status::OnGoing && status != Status::Finished;
}

void Event::setOrganizerName(const std::string& _name) {
    if(!isEditable()) return;
    organizerName= _name;
    updateStatus();
}

void Event::setName(const std::string& _name) {
    if(!isEditable()) return;
    name= _name;
    updateStatus();
}

void Event::setLocation(const std::string& _location) {
    if(!isEditable()) return;
    location= _location;
    updateStatus();
}

void Event::setLogo(const std::filesystem::path& _logo) {
    if(!isEditable()) return;
    logo= _logo;
    updateStatus();
}

void Event::setOrganizerLogo(const std::filesystem::path& _logo) {
    if(!isEditable()) return;
    organizerLogo= _logo;
    updateStatus();
}

void Event::startEvent() {
    status= Status::OnGoing;
}

void Event::stopEvent() {
    status= Status::Finished;
}

}// namespace evl::core
