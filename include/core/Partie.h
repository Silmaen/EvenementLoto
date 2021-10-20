/**
* @author Silmaen
* @date 20/10/202
*/
#pragma once
#include "Serializable.h"
#include <chrono>
#include <vector>

namespace evl::core {

class Partie: public Serializable {
public:
    /**
     * @brief Liste des types de parties connus.
     */
    enum struct Type {
        Aucun,      ///< la partie n'a aucun type
        UneQuine,   ///< Le joueur gagne dès qu’il complète une ligne.
        DeuxQuines, ///< Le joueur gagne dès qu’il complète deux lignes.
        CartonPlein,///< Le joueur gagne dès qu’il complète son carton.
        Inverse     ///< Le joueur est éliminé dès qu’un se ses numéros est tiré.
    };
    /**
     * @brief Renvoie une chaine contenant le type de partie.
     * @return Le type de partie.
     */
    [[nodiscard]] std::string getTypeStr() const;
    /**
     * @brief Renvoie le type de partie.
     * @return Le type de partie
     */
    [[nodiscard]] const Type& getType() const { return type; }

    /**
     * @brief Définit le type de partie
     * @param t
     */
    void setType(const Type& t) {
        if(!isEditable()) return;
        type= t;
        updateStatus();
    }

    /**
     * @brief Les status possibles de la partie
     */
    enum struct Status {
        Invalid,///< Le statut d’erreur
        Ready,  ///< La partie est prête à être jouée
        Started,///< La partie est démarrée
        Finished///< La partie est finie
    };
    /**
     * @brief Renvoie une chaine contenant le type de partie.
     * @return Le type de partie.
     */
    [[nodiscard]] std::string getStatusStr() const;
    /**
     * @brief Renvoie le type de partie.
     * @return Le type de partie
     */
    [[nodiscard]] const Status& getStatus() const { return status; }

    /**
     * @brief Débute la partie.
     */
    void startPartie();

    /**
     * @brief Termine la Partie
     */
    void finishPartie();

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

private:
    /**
     * @brief Met à jour le statut.
     */
    void updateStatus() {
        if(type == Type::Aucun) {
            status= Status::Invalid;
            return;
        }
        std::chrono::time_point<std::chrono::steady_clock> epoch{};
        if((start - epoch).count() == 0) {
            status= Status::Ready;
            end   = start;
        } else if((end - epoch).count() == 0) {
            status= Status::Started;
        } else {
            status= Status::Finished;
        }
    }
    /**
     * @brief Determine si la partie peut être éditée.
     * @return True si la partie est éditable
     */
    [[nodiscard]] bool isEditable() const {
        return status == Status::Invalid || status == Status::Ready;
    }

    Type type    = Type::Aucun;                                ///< Le type de partie.
    Status status= Status::Invalid;                            ///< Le type actuel de la partie.
    std::chrono::time_point<std::chrono::steady_clock> start{};///< La date et heure de début de partie
    std::chrono::time_point<std::chrono::steady_clock> end{};  ///< La date et heure de début de partie
    std::vector<uint8_t> Tirages;                              ///< La liste des numéros tirés.
};

}// namespace evl::core
