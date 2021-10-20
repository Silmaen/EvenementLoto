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
    start= std::chrono::steady_clock::now();
    updateStatus();
}

void Partie::finishPartie() {
    if(status != Status::Started) return;// start allowed only if already started
    end= std::chrono::steady_clock::now();
    updateStatus();
}

void Partie::read(std::istream&) {
}

void Partie::write(std::ostream&) const {
}

}// namespace evl::core
