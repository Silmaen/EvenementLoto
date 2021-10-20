/**
* @author Silmaen
* @date 19/10/202
*/
#include "core/PaquetCartons.h"

namespace evl::core {

PaquetCartons::PaquetCartons()= default;

void PaquetCartons::write(std::ostream& bs) const {
    std::string::size_type ss= nom.size();
    bs.write((char*)&ss, sizeof(ss));
    bs.write(nom.c_str(), nom.size());
    std::vector<Carton>::size_type sv= cartons.size();
    bs.write((char*)&sv, sizeof(sv));
    for(const Carton& carton: cartons) carton.write(bs);
}

void PaquetCartons::read(std::istream& bs) {
    std::string::size_type ss;
    bs.read((char*)&ss, sizeof(ss));
    nom.resize(ss);
    for(std::string::size_type i= 0; i < ss; ++i)
        bs.read(&(nom[i]), 1);
    std::vector<Carton>::size_type sv;
    bs.read((char*)&sv, sizeof(sv));
    cartons.clear();
    cartons.resize(sv);
    for(std::string::size_type i= 0; i < sv; ++i)
        cartons[i].read(bs);
}

std::vector<Carton> PaquetCartons::getCartonByStatus(const Carton::ResultCarton& st) const {
    std::vector<Carton> resu;
    for(const auto& carton: cartons)
        if(carton.getStatus() == st) resu.push_back(carton);
    return resu;
}

void PaquetCartons::generate(const uint32_t& number, RandomNumberGenerator* rng) {
    bool cleaning= rng == nullptr;
    if(cleaning)
        rng= new RandomNumberGenerator;
    for(uint32_t i= 0; i < number; ++i) {
        Carton a;
        a.generate(cartons.size(), rng);
        cartons.push_back(a);
    }
    if(cleaning)
        delete rng;
}

}// namespace evl::core
