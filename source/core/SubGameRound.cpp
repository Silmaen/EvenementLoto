/**
* @file SubGameRound.cpp
* @author Silmaen
* @date 26/10/202
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/

#include "SubGameRound.h"

namespace evl::core {

const std::unordered_map<SubGameRound::Type, string> SubGameRound::TypeConvert= {
        {SubGameRound::Type::OneQuine, "simple quine"},
        {SubGameRound::Type::TwoQuines, "double quine"},
        {SubGameRound::Type::FullCard, "carton plein"},
        {SubGameRound::Type::Inverse, "inverse"},
};

const string SubGameRound::getTypeStr() const {
    if(TypeConvert.contains(type)) {
        return TypeConvert.at(type);
    }
    return evl::string("Type Inconnu");
}

void SubGameRound::read(std::istream& bs, int) {
    bs.read(reinterpret_cast<char*>(&type), sizeof(Type));
    bs.read(reinterpret_cast<char*>(&winner), sizeof(uint32_t));
    sizeType l;
    bs.read(reinterpret_cast<char*>(&l), sizeof(sizeType));
    prices.resize(l);
    for(sizeType i= 0; i < l; i++)
        bs.read(reinterpret_cast<char*>(&prices[i]), charSize);
}

void SubGameRound::write(std::ostream& bs) const {
    bs.write(reinterpret_cast<const char*>(&type), sizeof(Type));
    bs.write(reinterpret_cast<const char*>(&winner), sizeof(uint32_t));
    sizeType l= prices.size();
    bs.write(reinterpret_cast<char*>(&l), sizeof(sizeType));
    for(sizeType i= 0; i < l; i++)
        bs.write(reinterpret_cast<const char*>(&prices[i]), charSize);
}

json SubGameRound::to_json() const {
    return json{{"type", getTypeStr()}, {"prices", prices}};
}

void SubGameRound::from_json(const json& j) {
    string srType;
    j.at("type").get_to(srType);
    for(auto& s: TypeConvert) {
        if(s.second == srType) {
            type= s.first;
            break;
        }
    }
    j.at("prices").get_to(prices);
}

}// namespace evl::core
