/**
* @author Silmaen
* @date 20/10/202
*/
#pragma once
#include "PaquetCartons.h"
#include "Partie.h"
#include "Serializable.h"
#include <filesystem>

namespace evl::core {

/**
 * @brief Classe décrivant un événement.
 */
class Evenement: public Serializable {
public:
    /**
     * @brief Lecture depuis un stream
     * @param bs Le stream d’entrée.
     */
    void read(std::istream& bs) override;
    /**
     * @brief Écriture dans un stream.
     * @param bs Le stream où écrire.
     */
    void write(std::ostream& bs) const override;

    /**
     * @brief Liste des statuts possibles.
     */
    enum struct Status {
        Invalid,       ///< N'est pas valide
        MissingParties,///< L’événement est bien défini, mais il n’a pas de parties définies
        Ready,         ///< Prêt à être utilisé
        OnGoing,       ///< Est en cours
        Finished       ///< Est fini
    };

    /**
     * @brief Accès au statut de l’événement.
     * @return Le statut de l’événement.
     */
    const Status& getStatus() const { return status; }

    /**
     * @brief Accès au nom de l’organisateur
     * @return Le nom de l’organisateur
     */
    const std::string& getNomOrganisateur() const { return nomOrganisateur; }

    /**
     * @brief Definition du nom de l’organisateur.
     * @param nom Le nom de l’organisateur.
     */
    void setNomOrganisateur(const std::string& nom);

    /**
     * @brief Accès au nom de l’événement
     * @return Le nom de l’événement
     */
    const std::string& getNom() const { return nom; }

    /**
     * @brief Definition du nom de l’événement.
     * @param nom Le nom de l’événement.
     */
    void setNom(const std::string& nom);

    /**
     * @brief Accès au lieu de l’événement
     * @return Le lieu de l’événement
     */
    const std::string& getLieu() const { return lieu; }

    /**
     * @brief Definition du lieu de l’événement.
     * @param nom Le lieu de l’événement.
     */
    void setLieu(const std::string& lieu);

    /**
     * @brief Accès au logo de l’organisateur
     * @return Le logo de l’organisateur
     */
    const std::filesystem::path& getLogoOrganisateur() const { return logoOrganisateur; }

    /**
     * @brief Definition du logo de l’organisateur.
     * @param nom Le logo de l’organisateur.
     */
    void setLogoOrganisateur(const std::filesystem::path& logo);

    /**
     * @brief Accès au logo de l’événement
     * @return Le logo de l’événement
     */
    const std::filesystem::path& getLogo() const { return logo; }

    /**
     * @brief Definition du logo de l’événement.
     * @param nom Le logo de l’événement.
     */
    void setLogo(const std::filesystem::path& logo);

private:
    /**
     * @brief Met à jour le statut de l’événement.
     */
    void updateStatus();

    /**
     * @brief Vérifie s'il est possible d’éditer l’événement
     * @return True si éditable.
     */
    bool isEditable() const;

    ///< Le statut de l’événement
    Status status;
    ///< Le nom de l’organisateur (requit pour validité)
    std::string nomOrganisateur;
    ///< Logo de l’organisateur.
    std::filesystem::path logoOrganisateur;
    /// Nom de l’événement. (requit pour validité)
    std::string nom;
    ///< Logo de l’événement.
    std::filesystem::path logo;
    /// Lieu de l’événement.
    std::string lieu;
    /// Liste des parties de l’événement.
    std::vector<Partie> parties;
    /// La date et heure de début de l’événement
    std::chrono::time_point<std::chrono::steady_clock> start{};
    /// La date et heure de début de l’événement
    std::chrono::time_point<std::chrono::steady_clock> end{};
    /// Lien vers la liste des cartons
    [[maybe_unused]] PaquetCartons* cartons= nullptr;
};

}// namespace evl::core
