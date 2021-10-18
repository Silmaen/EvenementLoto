/**
 * \author Silmaen
 * \date 17/10/2021
 *
 */
#include "carton.h"
#include <sstream>
#include <vector>

namespace evl {

void carton::resetCarton() {
    for(auto& line: lines) {
        for(auto& num: line) {
            num.checked= false;
        }
    }
    updateStatus();
}

std::string carton::asString() const {
    std::stringstream oss;
    oss << numero << ";;";
    for(auto& line: lines) {
        for(auto& num: line) {
            oss << (int)num.numero << ";";
        }
        oss << ";";
    }
    return oss.str();
}

void carton::fromString(std::string& s) {
    std::vector<std::string> result;
    size_t pos;
    std::string token{s};
    while((pos= s.find(';')) != std::string::npos) {
        token= s.substr(0, pos);
        result.push_back(token.substr(0, pos));
        token.erase(0, pos + 1);
    }
    pos   = 0;
    numero= std::atoi(result[pos].c_str());
    pos+= 2;
    for(auto& line: lines) {
        for(auto& num: line) {
            num.numero= std::atoi(result[pos].c_str());
            ++pos;
        }
        ++pos;
    }
    resetCarton();
}

void carton::playNumber(const uint8_t num) {
    bool change= false;
    for(auto& line: lines) {
        for(auto& _num: line) {
            if(_num.numero == num) {
                _num.checked= true;
                change      = true;
                break;
            }
        }
    }
    if(change) updateStatus();
}

void carton::unPlayNumber(const uint8_t num) {
    bool change= false;
    for(auto& line: lines) {
        for(auto& _num: line) {
            if(_num.numero == num) {
                _num.checked= false;
                change      = true;
                break;
            }
        }
    }
    if(change) updateStatus();
}

void carton::updateStatus() {
    if(res == ResultCarton::HorsJeu)
        return;
    res= ResultCarton::EnCours;
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
        res= ResultCarton::CartonPlein;
    } else if(nbFullLines == 2) {
        if(nbAlmostFullLines == 1)
            res= ResultCarton::PresqueCartonPlein;
        else
            res= ResultCarton::DeuxLignes;
    } else if(nbFullLines == 1) {
        if(nbAlmostFullLines >= 1)
            res= ResultCarton::PresqueDeuxLignes;
        else
            res= ResultCarton::UneLigne;
    } else if(nbAlmostFullLines >= 1) {
        res= ResultCarton::PresqueUneLigne;
    }
}

}// namespace evl
