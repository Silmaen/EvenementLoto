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
const std::unordered_map<SubGameRound::Status, string> SubGameRound::StatusConvert= {
        {SubGameRound::Status::Ready, "prêt"},
        {SubGameRound::Status::PreScreen, "affichage"},
        {SubGameRound::Status::Running, "en cours"},
        {SubGameRound::Status::Done, "fini"},
};

const string SubGameRound::getTypeStr() const {
    if(TypeConvert.contains(type)) {
        return TypeConvert.at(type);
    }
    return evl::string("inconnu");
}

const string SubGameRound::getStatusStr() const {
    if(StatusConvert.contains(status)) {
        return StatusConvert.at(status);
    }
    return evl::string("inconnu");
}

void SubGameRound::nextStatus() {
    switch(status) {
    case Status::Ready:
        start= clock::now();
        if(!prices.empty())
            status= Status::PreScreen;
        else
            status= Status::Running;
        break;
    case Status::PreScreen:
        status= Status::Running;
        break;
    case Status::Running:
        if(!winner.empty()) {
            status= Status::Done;
            end   = clock::now();
        }
        break;
    case Status::Done:// the last status!
        break;
    }
}

void SubGameRound::read(std::istream& bs, int file_version) {
    if(file_version > currentSaveVersion) return;
    bs.read(reinterpret_cast<char*>(&type), sizeof(Type));
    if(file_version >= 4) {
        bs.read(reinterpret_cast<char*>(&status), sizeof(Status));
    }
    if(file_version < 4) {                                           //----UNCOVER----
        uint32_t readTmp;                                            //----UNCOVER----
        bs.read(reinterpret_cast<char*>(&readTmp), sizeof(uint32_t));//----UNCOVER----
        if(readTmp != 0)                                             //----UNCOVER----
            winner= "toto";                                          //----UNCOVER----
        else                                                         //----UNCOVER----
            winner= "";                                              //----UNCOVER----
    } else {                                                         //----UNCOVER----
        bs.read(reinterpret_cast<char*>(&pricesValue), sizeof(double));
        sizeType l;
        bs.read(reinterpret_cast<char*>(&l), sizeof(sizeType));
        winner.resize(l);
        for(sizeType i= 0; i < l; i++)
            bs.read(reinterpret_cast<char*>(&winner[i]), charSize);
    }
    sizeType l;
    bs.read(reinterpret_cast<char*>(&l), sizeof(sizeType));
    prices.resize(l);
    for(sizeType i= 0; i < l; i++)
        bs.read(reinterpret_cast<char*>(&prices[i]), charSize);
    if(file_version > 3) {
        drawsType::size_type ld;
        bs.read(reinterpret_cast<char*>(&ld), sizeof(drawsType::size_type));
        draws.resize(ld);
        for(drawsType::size_type i= 0; i < ld; ++i)
            bs.read(reinterpret_cast<char*>(&(draws[i])), sizeof(drawsType::value_type));
    }
    if(file_version > 5) {
        bs.read(reinterpret_cast<char*>(&start), sizeof(start));
        bs.read(reinterpret_cast<char*>(&end), sizeof(end));
    }
}

void SubGameRound::write(std::ostream& bs) const {
    bs.write(reinterpret_cast<const char*>(&type), sizeof(Type));
    bs.write(reinterpret_cast<const char*>(&status), sizeof(Status));
    bs.write(reinterpret_cast<const char*>(&pricesValue), sizeof(double));
    //---------------
    sizeType l0= winner.size();
    bs.write(reinterpret_cast<char*>(&l0), sizeof(sizeType));
    for(sizeType i= 0; i < l0; i++)
        bs.write(reinterpret_cast<const char*>(&winner[i]), charSize);
    //---------------
    sizeType l= prices.size();
    bs.write(reinterpret_cast<char*>(&l), sizeof(sizeType));
    for(sizeType i= 0; i < l; i++)
        bs.write(reinterpret_cast<const char*>(&prices[i]), charSize);
    // --------------
    drawsType::size_type ld= draws.size();
    bs.write(reinterpret_cast<const char*>(&ld), sizeof(drawsType::size_type));
    for(drawsType::size_type i= 0; i < ld; ++i)
        bs.write(reinterpret_cast<const char*>(&(draws[i])), sizeof(drawsType::value_type));
    bs.write(reinterpret_cast<const char*>(&start), sizeof(start));
    bs.write(reinterpret_cast<const char*>(&end), sizeof(end));
}

json SubGameRound::to_json() const {
    return json{
            {"type", getTypeStr()},
            {"prices", prices},
            {"value", pricesValue},
            {"winner", winner},
            {"draws", draws}};
}

void SubGameRound::from_json(const json& j) {
    string srType;
    j.at("type").get_to(srType);
    auto result= std::find_if(TypeConvert.begin(), TypeConvert.end(), [&srType](const auto& item) { return item.second == srType; });
    if(result != TypeConvert.end())
        type= result->first;
    j.at("prices").get_to(prices);
    j.at("value").get_to(pricesValue);
    j.at("winner").get_to(winner);
    j.at("draws").get_to(draws);
}

}// namespace evl::core
