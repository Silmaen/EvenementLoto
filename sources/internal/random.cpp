/**
* @author Silmaen
* @date 18/10/2021
*/
#include "core/random.h"
#include "baseDefine.h"

namespace evl::core {

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

}// namespace evl
