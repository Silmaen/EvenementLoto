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
     */
    explicit SubGameRound(const Type& t= Type::OneQuine, const string& _prices= "") {
        define(t, _prices);
    }

    /**
     * @brief Définition de la sous-partie
     * @param t Le type de sous partie
     * @param _prices Le(s) lot(s) pour cette partie
     */
    void define(const Type& t, const string& _prices= "") {
        type  = t;
        prices= _prices;
        winner= 0;
    }

    /**
     * @brief Renvoie les lots de cette partie.
     * @return Les lots.
     */
    const string& getPrices() const { return prices; }

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
     * @brief Renvoie le numéro de la grille gagnante.
     * @return Le numéro de la grille gagnante.
     */
    uint32_t getWinner() const { return winner; }

    /**
     * @brief Défini le numéro du gagnant
     * @param w Le numéro de la grille gagnante
     */
    void setWinner(const uint32_t w) {
        if(w > 0)
            winner= w;
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
    /// La liste des prix pour ce
    string prices;
    /// Le type de la sous-partie
    Type type;
    /// Le numéro du winner
    uint32_t winner= 0;
};

}// namespace evl::core
