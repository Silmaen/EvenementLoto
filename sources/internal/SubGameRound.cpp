/**
* @file SubGameRound.cpp
* @author Silmaen
* @date 26/10/202
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/

#include "core/SubGameRound.h"

namespace evl::core {

void SubGameRound::read(std::istream& bs) {
    bs.read((char*)&type, sizeof(Type));
    bs.read((char*)&winner, sizeof(uint32_t));
    sizeType l;
    bs.read((char*)&l, sizeof(sizeType));
    prices.resize(l);
    for(sizeType i= 0; i < l; i++)
        bs.read((char*)&prices[i], charSize);
}

void SubGameRound::write(std::ostream& bs) const {
    bs.write((char*)&type, sizeof(Type));
    bs.write((char*)&winner, sizeof(uint32_t));
    sizeType l= prices.size();
    bs.write((char*)&l, sizeof(sizeType));
    for(sizeType i= 0; i < l; i++)
        bs.write((char*)&prices[i], charSize);
}

}// namespace evl::core
