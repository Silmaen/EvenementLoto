/**
 * @file RandomNumberGenerator.cpp
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "core/RandomNumberGenerator.h"
#include "baseDefine.h"
#include <chrono>

namespace evl::core {

RandomNumberGenerator::RandomNumberGenerator(bool debug) {
    if(debug)
        std::srand(1234);
    else
        std::srand(std::chrono::steady_clock::now().time_since_epoch().count());
}

bool RandomNumberGenerator::addPick(const uint8_t& num) {
    if(std::find(alreadyPicked.begin(), alreadyPicked.end(), num) != alreadyPicked.end())
        return false;
    alreadyPicked.push_back(num);
    return true;
}

uint8_t RandomNumberGenerator::pick() {
    if(alreadyPicked.size() >= 90) return 255;
    uint8_t n= (std::rand()) % 90 + 1;
    while(std::find(alreadyPicked.begin(), alreadyPicked.end(), n) != alreadyPicked.end())
        n= (std::rand()) % 90 + 1;
    alreadyPicked.push_back(n);
    return n;
}

std::vector<uint8_t> RandomNumberGenerator::generateLine() {
    std::vector<uint8_t> res;
    std::vector<uint8_t> dizaine;
    for(uint8_t i= 0; i < nb_colones; ++i) {
        while(true) {
            uint8_t n  = (std::rand()) % 90 + 1;
            uint8_t diz= n / 10;
            if(diz == 9) diz= 8;// cas du 90 qui doit être placé dans la colonne '8'
            if(std::find(dizaine.begin(), dizaine.end(), diz) != dizaine.end())
                continue;
            if(std::find(alreadyPicked.begin(), alreadyPicked.end(), n) != alreadyPicked.end())
                continue;
            dizaine.push_back(diz);
            res.push_back(n);
            alreadyPicked.push_back(n);
            break;
        }
    }
    std::sort(res.begin(), res.end());
    return res;
}

void RandomNumberGenerator::popNum() {
    if(!alreadyPicked.empty())
        alreadyPicked.pop_back();
}

}// namespace evl::core
