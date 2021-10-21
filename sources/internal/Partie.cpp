/**
* @author Silmaen
* @date 20/10/202
*/
#include "core/Partie.h"

namespace evl::core {

std::string Partie::getTypeStr() const {
    switch(type) {
    case Type::Aucun: return "Aucun type défini";
    case Type::UneQuine: return "Une Quine";
    case Type::DeuxQuines: return "Deux Quine";
    case Type::CartonPlein: return "Carton Plein";
    case Type::Inverse: return "Inverse";
    }
    return "Type de partie inconnu";
}

std::string Partie::getStatusStr() const {
    switch(status) {
    case Status::Invalid: return "Partie invalide";
    case Status::Ready: return "Partie Prête";
    case Status::Started: return "Partie Démarrée";
    case Status::Finished: return "Partie finie";
    }
    return "Statut inconnu";
}

void Partie::startPartie() {
    if(status != Status::Ready) return;// start allowed only if ready
    start= std::chrono::system_clock::now();
    updateStatus();
}

void Partie::finishPartie() {
    if(status != Status::Started) return;// start allowed only if already started
    end= std::chrono::system_clock::now();
    updateStatus();
}

void Partie::read(std::istream& is) {
    is.read(reinterpret_cast<char*>(&type), sizeof(type));
    is.read(reinterpret_cast<char*>(&status), sizeof(status));
    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));
    std::vector<uint8_t>::size_type l;
    is.read(reinterpret_cast<char*>(&l), sizeof(std::vector<uint8_t>::size_type));
    for(std::vector<uint8_t>::size_type i= 0; i < l; ++i)
        is.read(reinterpret_cast<char*>(&(Tirages[i])), sizeof(std::vector<uint8_t>::value_type));
}

void Partie::write(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&type), sizeof(type));
    os.write(reinterpret_cast<const char*>(&status), sizeof(status));
    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));
    std::vector<uint8_t>::size_type l= Tirages.size();
    os.write(reinterpret_cast<const char*>(&l), sizeof(std::vector<uint8_t>::size_type));
    for(std::vector<uint8_t>::size_type i= 0; i < l; ++i)
        os.write(reinterpret_cast<const char*>(&(Tirages[i])), sizeof(std::vector<uint8_t>::value_type));
}

}// namespace evl::core
