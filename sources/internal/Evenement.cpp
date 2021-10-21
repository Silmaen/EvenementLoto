/**
* @author Silmaen
* @date 20/10/202
*/
#include "core/Evenement.h"

namespace evl::core {

void Evenement::read(std::istream& is) {
    std::string::size_type l, i;
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    nomOrganisateur.resize(l);
    for(i= 0; i < l; ++i) is.read(&(nomOrganisateur[i]), sizeof(std::string::value_type));

    std::string temp;
    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    temp.resize(l);
    for(i= 0; i < l; ++i) is.read(&(temp[i]), sizeof(std::string::value_type));
    logoOrganisateur= temp;

    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    nom.resize(l);
    for(i= 0; i < l; ++i) is.read(&(nom[i]), sizeof(std::string::value_type));

    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    temp.resize(l);
    for(i= 0; i < l; ++i) is.read(&(temp[i]), sizeof(std::string::value_type));
    logo= temp;

    is.read(reinterpret_cast<char*>(&l), sizeof(l));
    lieu.resize(l);
    for(i= 0; i < l; ++i) is.read(&(lieu[i]), sizeof(std::string::value_type));

    std::vector<Partie>::size_type lv, iv;
    is.read(reinterpret_cast<char*>(&lv), sizeof(lv));
    parties.resize(lv);
    for(iv= 0; iv < lv; ++iv) parties[iv].read(is);

    updateStatus();

    is.read(reinterpret_cast<char*>(&start), sizeof(start));
    is.read(reinterpret_cast<char*>(&end), sizeof(end));

    // TODO: gérer les cartons
    updateStatus();
}

void Evenement::write(std::ostream& os) const {
    std::string::size_type l, i;
    l= nomOrganisateur.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(nomOrganisateur[i]), sizeof(std::string::value_type));
    l= logoOrganisateur.string().size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(logoOrganisateur.string()[i]), sizeof(std::string::value_type));
    l= nom.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(nom[i]), sizeof(std::string::value_type));
    l= logo.string().size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(logo.string()[i]), sizeof(std::string::value_type));
    l= lieu.size();
    os.write(reinterpret_cast<char*>(&l), sizeof(l));
    for(i= 0; i < l; ++i) os.write(&(lieu[i]), sizeof(std::string::value_type));

    std::vector<Partie>::size_type lv, iv;
    lv= parties.size();
    os.write(reinterpret_cast<char*>(&lv), sizeof(lv));
    for(iv= 0; iv < lv; ++iv) parties[iv].write(os);

    os.write(reinterpret_cast<const char*>(&start), sizeof(start));
    os.write(reinterpret_cast<const char*>(&end), sizeof(end));

    // TODO: gérer les cartons
}

void Evenement::updateStatus() {
    if(nomOrganisateur.empty() || nom.empty()) {
        status= Status::Invalid;
        return;
    }
    if(parties.empty()) {
        status= Status::MissingParties;
        return;
    }
    std::chrono::time_point<std::chrono::steady_clock> epoch{};
    if(start == epoch) {
        status= Status::Ready;
        return;
    }
    if(end == epoch) {
        status= Status::OnGoing;
        return;
    }
    status= Status::Finished;
}

bool Evenement::isEditable() const {
    return status != Status::OnGoing && status != Status::Finished;
}

void Evenement::setNomOrganisateur(const std::string& _nom) {
    if(!isEditable()) return;
    nomOrganisateur= _nom;
    updateStatus();
}

void Evenement::setNom(const std::string& _nom) {
    if(!isEditable()) return;
    nom= _nom;
    updateStatus();
}

void Evenement::setLieu(const std::string& _lieu) {
    if(!isEditable()) return;
    lieu= _lieu;
    updateStatus();
}

void Evenement::setLogo(const std::filesystem::path& _logo) {
    if(!isEditable()) return;
    logo= _logo;
    updateStatus();
}

void Evenement::setLogoOrganisateur(const std::filesystem::path& _logo) {
    if(!isEditable()) return;
    logoOrganisateur= _logo;
    updateStatus();
}

}// namespace evl::core
