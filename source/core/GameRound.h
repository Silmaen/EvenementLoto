/**
 * @file GameRound.h
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "Serializable.h"
#include "SubGameRound.h"
#include "timeFunctions.h"
#include <vector>

namespace evl::core {

/**
 * @brief Classe définissant une partie.
 */
class GameRound: public Serializable {
public:
    // ---- définition des types ----
    /// Le type utilisé pour représenter la liste des tirages.
    using drawsType    = std::vector<uint8_t>;
    using subRoundsType= std::vector<SubGameRound>;
    /**
     * @brief Liste des types de parties connus.
     */
    enum struct Type {
        OneQuine,           ///< Une partie standard en remplissant une ligne seulement avant reset.
        TwoQuines,          ///< Une partie standard en remplissant deux lignes seulement.
        FullCard,           ///< Une partie standard en remplissant le carton seulement.
        OneQuineFullCard,   ///< Une partie standard en remplissant une ligne, puis le carton.
        OneTwoQuineFullCard,///< Une partie standard en remplissant une ligne, puis deux, puis le carton.
        Enfant,             ///< Une partie pour les enfants, similaire à OneQuine.
        Inverse,            ///< Le joueur est éliminé dès qu’un se ses numéros est tiré.
    };
    static const std::unordered_map<Type, string> TypeConvert;

    /**
     * @brief Les status possibles de la partie
     */
    enum struct Status {
        Ready,        ///< La partie est prête à être jouée
        Started,      ///< La partie est démarrée
        DisplayResult,///< Est en affichage de résultat.
        Finished      ///< La partie est finie
    };

    // --- constructeurs ----
    /**
     * @brief Constructeur
     * @param t Le type de partie
     */
    GameRound(Type t= Type::OneTwoQuineFullCard);

    // ---- manipulation du type de partie ----
    /**
     * @brief Renvoie une chaine contenant le type de partie.
     * @return Le type de partie.
     */
    [[nodiscard]] string getTypeStr() const;

    /**
     * @brief Renvoie le type de partie.
     * @return Le type de partie
     */
    [[nodiscard]] const Type& getType() const { return type; }

    /**
     * @brief Définit le type de partie
     * @param t Type de partie
     */
    void setType(const Type& t);

    // ---- manipulation du statut ----
    /**
     * @brief Renvoie une chaine contenant le type de partie.
     * @return Le type de partie.
     */
    [[nodiscard]] string getStatusStr() const;

    /**
     * @brief Renvoie le type de partie.
     * @return Le type de partie
     */
    [[nodiscard]] const Status& getStatus() const { return status; }

#ifdef EVL_DEBUG
    /**
     * @brief define invalide Status for testing purpose
     */
    void invalidStatus() {
        status= Status{-1};
        type  = Type{-1};
    }
    /**
     * @brief define invalide Status for testing purpose
     */
    void restoreStatus() {
        status= Status::Ready;
        type  = Type::OneTwoQuineFullCard;
    }
#endif

    // ---- flux du jeu ----
    /**
     * @brief Débute la partie.
     */
    void startGameRound();
    /**
     * @brief Ajoute le numéro dans la liste des numéros tirés
     * @param num Numéro à ajouter
     */
    void addPickedNumber(const uint8_t& num);

    /**
     * @brief Renvoie un itérateur constant sur le début de la liste des tirages.
     * @return Itérateur constant sur le début de la liste des tirages.
     */
    drawsType::const_iterator beginDraws() const { return draws.cbegin(); }
    /**
     * @brief Renvoie un itérateur constant sur la fin de la liste des tirages.
     * @return Itérateur constant sur la fin de la liste des tirages.
     */
    drawsType::const_iterator endDraws() const { return draws.cend(); }
    /**
     * @brief Renvoie un itérateur inverse constant sur le début de la liste des tirages.
     * @return Itérateur inverse constant sur le début de la liste des tirages.
     */
    drawsType::const_reverse_iterator beginReverseDraws() const { return draws.crbegin(); }
    /**
     * @brief Renvoie un itérateur inverse constant sur la fin de la liste des tirages.
     * @return Itérateur inverse constant sur la fin de la liste des tirages.
     */
    drawsType::const_reverse_iterator endReverseDraws() const { return draws.crend(); }

    /**
     * @brief Renvoie le nombre de tirages.
     * @return Le nombre de tirages.
     */
    drawsType::size_type sizeDraws() const { return draws.size(); }

    /**
     * @brief Supprime le dernier tirage.
     */
    void removeLastPick();
    /**
     * @brief donne un gagnant
     * @param win Le numéro de la grille gagnante
     */
    void addWinner(uint32_t win);
    /**
     * @brief Close la Partie
     */
    void closeGameRound();

    // ---- Serialisation ----
    /**
     * @brief Lecture depuis un stream
     * @param bs Le stream d’entrée.
     * @param file_version La version du fichier à lire
     */
    void read(std::istream& bs, int file_version) override;

    /**
     * @brief Écriture dans un stream.
     * @param bs Le stream où écrire.
     */
    void write(std::ostream& bs) const override;

    /**
     * @brief Écriture dans un json.
     * @return Le json à remplir
     */
    json to_json() const override;

    /**
     * @brief Lecture depuis un json
     * @param j Le json à lire
     */
    void from_json(const json& j) override;

    // ---- accès aux timers ----
    /**
     * @brief Accès à la date de départ
     * @return La date de départ
     */
    const timePoint& getStarting() const { return start; }

    /**
     * @brief Accès à la date de fin
     * @return La date de fin
     */
    const timePoint& getEnding() const { return end; }

    // ---- accès aux sub rounds (lecture seule) ----
    /**
     * @brief Renvoie si la sous-partie actuelle est la dernière de la liste
     * @return True si on est à la dernière sous-partie.
     */
    bool isCurrentSubRoundLast() const;
    /**
     * @brief Accès à la sous-partie courante l’interface
     * @return Itérateur constant sur la sous-partie courante
     */
    subRoundsType::const_iterator getCurrentCSubRound() const;
    /**
     * @brief Accès à la sous-partie courante
     * @return Itérateur constant sur la sous-partie courante
     */
    subRoundsType::iterator getSubRound(uint32_t index);
    /**
     * @brief Renvoie un itérateur constant sur le début de la liste des subround.
     * @return Itérateur constant sur le début de la liste des subround.
     */
    subRoundsType::const_iterator beginSubRound() const { return subGames.cbegin(); }
    /**
     * @brief Renvoie un itérateur constant sur la fin de la liste des subround.
     * @return Itérateur constant sur la fin de la liste des subround.
     */
    subRoundsType::const_iterator endSubRound() const { return subGames.cend(); }
    /**
     * @brief Renvoie la taille de la liste des subround.
     * @return La taille de la fin de la liste des subround.
     */
    subRoundsType::size_type sizeSubRound() const { return subGames.size(); }

    /**
     * @brief Défini le numéro de partie
     * @param id Le numéro de partie
     */
    void setID(const int id) { Id= id; }

    /**
     * @brief Renvoie le numéro d'affichage de la partie
     * @return le numéro d'affichage
     */
    const int& getID() const { return Id; }

    /**
     * @brief affichage de nom spécial
     * @return Nom du round
     */
    string getName() const;

private:
    /**
     * @brief Accès à la sous-partie courante
     * @return Pointeur vers la sous-partie courante
     */
    subRoundsType::iterator getCurrentSubRound();

    /**
     * @brief Determine si la partie peut être éditée.
     * @return True si la partie est éditable
     */
    [[nodiscard]] bool isEditable() const;

    /// Le type de partie.
    Type type= Type::OneTwoQuineFullCard;

    /// Le type actuel de la partie.
    Status status= Status::Ready;

    /// La date et heure de début de partie
    timePoint start{};

    /// La date et heure de début de partie
    timePoint end{};

    /// La liste des numéros tirés.
    drawsType draws;

    /// La liste des
    subRoundsType subGames;

    /// Le numéro de la partie (à ne pas afficher si négatif)
    int Id= 0;
};

}// namespace evl::core
