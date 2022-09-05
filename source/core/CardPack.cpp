/**
 * @file CardPack.cpp
 * @author Silmaen
 * @date 19/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "CardPack.h"

namespace evl::core {


void CardPack::write(std::ostream& bs) const {
    std::string::size_type ss= nom.size();
    bs.write(reinterpret_cast<char*>(&ss), sizeof(ss));
    bs.write(nom.c_str(), static_cast<uint32_t>(nom.size()));
    std::vector<GridCard>::size_type sv= cartons.size();
    bs.write(reinterpret_cast<char*>(&sv), sizeof(sv));
    for(const GridCard& carton: cartons) carton.write(bs);
}

void CardPack::read(std::istream& bs, int file_version) {
    std::string::size_type ss;
    bs.read(reinterpret_cast<char*>(&ss), sizeof(ss));
    nom.resize(ss);
    for(std::string::size_type i= 0; i < ss; ++i)
        bs.read(&(nom[i]), 1);
    std::vector<GridCard>::size_type sv;
    bs.read(reinterpret_cast<char*>(&sv), sizeof(sv));
    cartons.clear();
    cartons.resize(sv);
    for(std::string::size_type i= 0; i < sv; ++i)
        cartons[i].read(bs, file_version);
}

std::vector<GridCard> CardPack::getGridCardsByStatus(const GridCard::Status& st) const {
    std::vector<GridCard> result;
    for(const auto& carton: cartons)
        if(carton.getStatus() == st) result.push_back(carton);
    return result;
}

void CardPack::generate(const uint32_t& number, RandomNumberGenerator* rng) {
    bool cleaning= rng == nullptr;
    if(cleaning)
        rng= new RandomNumberGenerator;
    for(uint32_t i= 0; i < number; ++i) {
        GridCard a;
        a.generate(static_cast<uint32_t>(cartons.size()), rng);
        cartons.push_back(a);
    }
    if(cleaning)
        delete rng;
}

}// namespace evl::core
