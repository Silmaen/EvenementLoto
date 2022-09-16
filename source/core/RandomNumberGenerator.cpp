/**
 * @file RandomNumberGenerator.cpp
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "RandomNumberGenerator.h"
#include "../baseDefine.h"
#include <algorithm>
#include <chrono>

namespace evl::core {

RandomNumberGenerator::RandomNumberGenerator(bool debug) {
    if(debug)
        std::srand(1234);
    else
        std::srand(static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
}

bool RandomNumberGenerator::addPick(const uint8_t& num) {
    if(std::find(alreadyPicked.begin(), alreadyPicked.end(), num) != alreadyPicked.end())
        return false;
    alreadyPicked.push_back(num);
    return true;
}

uint8_t RandomNumberGenerator::pick() {
    if(alreadyPicked.size() >= 90) return 255;
    uint8_t n= static_cast<uint8_t>((std::rand()) % 90 + 1);
    while(std::find(alreadyPicked.begin(), alreadyPicked.end(), n) != alreadyPicked.end())
        n= static_cast<uint8_t>((std::rand()) % 90 + 1);
    alreadyPicked.push_back(n);
    return n;
}

void RandomNumberGenerator::popNum() {
    if(!alreadyPicked.empty())
        alreadyPicked.pop_back();
}

}// namespace evl::core
