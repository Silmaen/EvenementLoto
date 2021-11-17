/**
 * @file VisualTheme.h
 * @author Silmaen
 * @date 15/11/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "baseDefine.h"
#include <QSettings>
#include <unordered_map>

namespace evl::gui {

/// Le type de liste de paramètre
using params= std::unordered_map<QString, QVariant>;
/// Liste des valeurs par défaut des paramètres
static const params themeDefaults{
        {"name", "default"},
        {"baseRatio", 0.02},
        {"titleRatio", 2.0},
        {"gridTextRatio", 0.85},
        {"shortTextRatio", 1.4},
        {"longTextRatio", 0.6},
        {"backgroundColor", "#F0F0F0"},
        {"gridBackgroundColor", "#F0F0F0"},
        {"textColor", "#000000"},
        {"selectedNumberColor", "#ff7000"},
        {"fadeNumbers", true},
        {"fadeNumbersAmount", 3},
        {"fadeNumbersStrength", 10},
};

/**
 * @brief Classe permettant la manipulation des éléments de Thème
 */
class VisualTheme {
public:
    /**
     * @brief Constructeur
     * @param set Lien vers les settings
     */
    explicit VisualTheme(QSettings* set);

    /**
     * @brief Remet tous les paramètres aux valeurs d'usine
     */
    void resetFactory();

    /**
     * @brief Exporte les paramètres dans un fichier JSON
     * @param file le Fichier JSON
     */
    void exportJSON(const path& file) const;

    /**
     * @brief Importe les paramètres depuis un fichier JSON
     * @param file le fichier JSON
     */
    void importJSON(const path& file);

    /**
     * @brief Récupération des valeurs depuis les paramètres
     */
    void setFromSettings();
    /**
     * @brief Écrit dans les paramètres
     */
    void writeInSettings();

    /**
     * @brief Accède à un paramètre.
     * @param key Le nom du paramètre.
     * @return Sa valeur si celui-ci existe (un QVariant vide sinon).
     */
    const QVariant& getParam(const QString& key);

    /**
     * @brief Définition d’un paramètre
     * Ne défini un paramètre que si celui-ci existe dans la liste des défauts et si le type de valeur correspond bien
     * @param key Le nom du paramètre.
     * @param value La valeur du paramètre.
     */
    void setParam(const QString& key, const QVariant& value);

    /**
     * @brief Si le thème a été modifié mais pas encore lu
     * @return Vrai si le theme n'a encore jamais été lu et a été modifié.
     */
    const bool& isModified() { return toUpdate; }

private:
    /// Les settings généraux.
    QSettings* settings;
    /// Les paramètres
    params parameters= themeDefaults;
    /// Si le thème a été modifié
    bool toUpdate= true;
};

}// namespace evl::gui
