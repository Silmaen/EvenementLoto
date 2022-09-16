/**
* @file SubGameRound.h
* @author Silmaen
* @date 26/10/202
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#pragma once
#include "Serializable.h"

namespace evl::core {

/**
 * @brief Classe définissant une sous partie.
 */
class SubGameRound: public Serializable {
public:
    /// Le type utilisé pour représenter la liste des tirages.
    using drawsType= std::vector<uint8_t>;
    /**
     * @brief Liste des types de sous-parties connus.
     */
    enum struct Type {
        OneQuine, ///< Le joueur gagne dès qu’il complète une ligne.
        TwoQuines,///< Le joueur gagne dès qu’il complète deux lignes.
        FullCard, ///< Le joueur gagne dès qu’il complète son carton.
        Inverse   ///< Le joueur est éliminé dès qu’un se ses numéros est tiré.
    };
    static const std::unordered_map<Type, string> TypeConvert;

    /**
     * @brief Constructeur Avec données
     * @param t Le type de sous partie
     * @param _prices Le(s) lot(s) pour cette partie
     * @param value La valeur du lot
     */
    explicit SubGameRound(const Type& t= Type::OneQuine, const string& _prices= "", const double value= 0) {
        define(t, _prices, value);
    }

    /**
     * @brief Définition de la sous-partie
     * @param t Le type de sous partie
     * @param _prices Le(s) lot(s) pour cette partie
     * @param value La valeur des lots
     */
    void define(const Type& t, const string& _prices= "", const double value= 0) {
        type       = t;
        prices     = _prices;
        pricesValue= value;
        winner     = "";
    }

    /**
     * @brief Renvoie les lots de cette partie.
     * @return Les lots.
     */
    const string& getPrices() const { return prices; }

    /**
     * @brief Renvoie la valeur du lot à gagner.
     * @return La valeur du lot.
     */
    const double& getValue() const { return pricesValue; }

    /**
     * @brief Renvoie le type de sous-partie.
     * @return Le type de sous-partie.
     */
    const Type& getType() const { return type; }

    /**
     * @brief Renvoie le nom du type de sous-partie
     * @return Le nom du type de sous-partie
     */
    const string getTypeStr() const;

    /**
     * @brief Ajoute le numéro dans la liste des numéros tirés
     * @param num Numéro à ajouter
     */
    void addPickedNumber(const uint8_t& num) {
        draws.push_back(num);
    }

    /**
     * @brief Supprime le dernier tirage.
     */
    void removeLastPick() {
        if(!draws.empty()) draws.pop_back();
    }

    /**
     * @brief Accès à la lioste des tirage
     * @return La liste des tirages
     */
    const drawsType& getDraws() const { return draws; }

    /**
     * @brief Vérifie si la sous-partie est finie.
     * @return True si la partie est finie.
     */
    bool isFinished() const { return !winner.empty(); }

    /**
     * @brief Renvoie le numéro de la grille gagnante.
     * @return Le numéro de la grille gagnante.
     */
    const string& getWinner() const { return winner; }

    /**
     * @brief Défini le numéro du gagnant
     * @param winner_ Le numéro de la grille gagnante
     */
    void setWinner(const string& winner_) {
        winner= winner_;
    }

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

private:
    /// La liste des prix pour cette sous-partie
    string prices;
    /// La valeur du lot
    double pricesValue= 0;
    /// Le type de la sous-partie
    Type type;
    /// Le nom du gagnant
    string winner;
    /// La liste des numéros tirés.
    drawsType draws;
};

}// namespace evl::core
