/**
 * @author Silmaen
 * @date 17/10/2021
 *
 */
#include "core/carton.h"
#include "core/random.h"
#include <sstream>
#include <vector>

namespace evl {

void Carton::resetCarton() {
    for(auto& line: lines) {
        for(auto& num: line) {
            num.checked= false;
        }
    }
    updateStatus();
}

std::string Carton::asString() const {
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

void Carton::fromString(const std::string& s) {
    std::vector<std::string> result;
    size_t pos;
    std::string token{s};
    while((pos= token.find(';')) != std::string::npos) {
        result.push_back(token.substr(0, pos));
        token.erase(0, pos + 1);
    }
    if(result.size() < 1 + nb_ligne * (nb_colones + 1))
        throw std::exception();
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

void Carton::playNumber(const uint8_t num) {
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

void Carton::unPlayNumber(const uint8_t num) {
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

void Carton::updateStatus() {
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

void Carton::generate(const uint32_t& num) {
    numero= num;
    RandomNumberGenerator rng;
    for(auto& line: lines) {
        auto l= rng.generateLine();
        for(uint8_t i= 0; i < nb_colones; ++i) {
            line[i].numero = l[i];
            line[i].checked= false;
        }
    }
    updateStatus();
}

void Carton::print(std::ostream& oss) const {
    oss << "Carton: " << numero << std::endl;
    for(auto& line: lines) {
        for(auto& _num: line) {
            oss << (int)_num.numero << " ";
        }
        oss << std::endl;
    }
}

}// namespace evl
