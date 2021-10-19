/**
* @author Silmaen
* @date 19/10/202
*/
#include "core/PaquetCartons.h"

namespace evl::core {

PaquetCartons::PaquetCartons()= default;

void PaquetCartons::write(std::ostream& bs) const {
    bs << nom << std::endl;
    bs << cartons.size() << std::endl;
    for(auto& carton: cartons) {
        bs.write((const char*)&carton, sizeof(carton));
    }
}

void PaquetCartons::read(std::istream& bs) {
    std::getline(bs, nom);
    std::string st_str;
    std::getline(bs, st_str);
    size_t s;
    s= std::atoi(st_str.c_str());
    cartons.clear();
    cartons.resize(s);
    for(auto& carton: cartons) {
        bs.read((char*)&carton, sizeof(carton));
    }
}

std::vector<Carton> PaquetCartons::getCartonByStatus(const Carton::ResultCarton& st) const {
    std::vector<Carton> resu;
    for(const auto& carton: cartons)
        if(carton.getStatus() == st) resu.push_back(carton);
    return resu;
}

void PaquetCartons::generate(const uint32_t& number) {
    for(uint32_t i= 0; i < number; ++i) {
        Carton a;
        a.generate(cartons.size());
        cartons.push_back(a);
    }
}

}// namespace evl::core
