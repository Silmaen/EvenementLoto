/**
 * @file GridCard.cpp
 * @author Silmaen
 * @date 17/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "core/GridCard.h"
#include <sstream>
#include <vector>

namespace evl::core {

void GridCard::reset() {
    for(auto& line: lines) {
        for(auto& num: line) {
            num.checked= false;
        }
    }
    updateStatus();
}

void GridCard::deactivate() {
    res= Status::OutGame;
    updateStatus();
}

void GridCard::activate() {
    res= Status::InGame;
    updateStatus();
}

std::string GridCard::asString() const {
    std::stringstream oss;
    oss << IdNumber << ";;";
    for(auto& line: lines) {
        for(auto& num: line) {
            oss << (int)num.number << ";";
        }
        oss << ";";
    }
    return oss.str();
}

void GridCard::fromString(const std::string& s) {
    std::vector<std::string> result;
    size_t pos;
    std::string token{s};
    while((pos= token.find(';')) != std::string::npos) {
        result.push_back(token.substr(0, pos));
        token.erase(0, pos + 1);
    }
    if(result.size() < 1 + nb_ligne * (nb_colones + 1))
        throw std::exception();
    pos     = 0;
    IdNumber= std::atoi(result[pos].c_str());
    pos+= 2;
    for(auto& line: lines) {
        for(auto& num: line) {
            num.number= std::atoi(result[pos].c_str());
            ++pos;
        }
        ++pos;
    }
    reset();
}

void GridCard::playNumber(const uint8_t num) {
    bool change= false;
    for(auto& line: lines) {
        for(auto& _num: line) {
            if(_num.number == num) {
                _num.checked= true;
                change      = true;
                break;
            }
        }
    }
    if(change) updateStatus();
}

void GridCard::unPlayNumber(const uint8_t num) {
    bool change= false;
    for(auto& line: lines) {
        for(auto& _num: line) {
            if(_num.number == num) {
                _num.checked= false;
                change      = true;
                break;
            }
        }
    }
    if(change) updateStatus();
}

void GridCard::updateStatus() {
    if(res == Status::OutGame)
        return;
    res= Status::InGame;
    std::array<uint8_t, nb_ligne> nbCheck;
    auto i_nc= nbCheck.begin();
    for(auto& line: lines) {
        *i_nc= 0;
        for(auto& _num: line) {
            *i_nc+= _num.checked;
        }
        ++i_nc;
    }
    uint8_t nbFullLines      = 0;
    uint8_t nbAlmostFullLines= 0;
    for(auto& ic: nbCheck) {
        if(ic == nb_colones) nbFullLines++;
        if(ic == nb_colones - 1) nbAlmostFullLines++;
    }
    if(nbFullLines == 3) {
        res= Status::Full;
    } else if(nbFullLines == 2) {
        if(nbAlmostFullLines == 1)
            res= Status::AlmostFull;
        else
            res= Status::TwoLines;
    } else if(nbFullLines == 1) {
        if(nbAlmostFullLines >= 1)
            res= Status::AlmostTwoLines;
        else
            res= Status::OneLine;
    } else if(nbAlmostFullLines >= 1) {
        res= Status::AlmostOneLine;
    }
}

void GridCard::generate(const uint32_t& num, RandomNumberGenerator* rng) {
    bool cleaning= rng == nullptr;
    if(cleaning)
        rng= new RandomNumberGenerator;
    else
        rng->resetPick();
    IdNumber= num;
    for(auto& line: lines) {
        auto l= rng->generateLine();
        for(uint8_t i= 0; i < nb_colones; ++i) {
            line[i].number = l[i];
            line[i].checked= false;
        }
    }
    updateStatus();
    if(cleaning)
        delete rng;
}

void GridCard::read(std::istream& is, int) {
    is.read((char*)&IdNumber, 4);
    is.read((char*)&res, sizeof(Status));
    for(const std::array<GCase, nb_colones>& line: lines) {
        for(const GCase& _num: line) {
            is.read((char*)&_num, sizeof(GCase));
        }
    }
}

void GridCard::write(std::ostream& os) const {
    os.write((char*)&IdNumber, 4);
    os.write((char*)&res, sizeof(Status));
    for(const std::array<GCase, nb_colones>& line: lines) {
        for(const GCase& _num: line) {
            os.write((char*)&_num, sizeof(GCase));
        }
    }
}

}// namespace evl::core
