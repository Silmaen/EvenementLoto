/**
* @file BaseDialog.h
* @author Silmaen
* @date 14/11/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/

#pragma once
#include "baseDefinitions.h"

/**
 * @brief Namespace for base dialogs
 */
namespace evl::gui::dialog {

/**
 * @brief Les type de fichiers
 */
enum struct FileTypes {
    EventSave,/// La sauvegarde d’un événement
    Images,   /// La liste des images supportées
    JSON,     /// Un fichier JSON
    Text,     /// Fichiers textes ou markdown
    ThemeFile,/// Fichier de thème
    Folder,   /// Un dossier
};

/**
 * @brief Ouverture d’une fenêtre de dialogue pour l’ouverture de fichier
 * @param type Le type de contenu à filter.
 * @param exist Si le fichier doit déjà exister (mode ouverture)
 * @return Le chemin, vide si échec ou annulation.
 */
path openFile(const FileTypes& type= FileTypes::EventSave, bool exist= false);

/**
 * @brief Ouvre une fenêtre de dialogue posant une question oui ou non
 * @param title Le titre de la fenêtre
 * @param question Le texte de la question.
 * @param add Le texte additionnel.
 * @return La réponse à la question
 */
bool question(const QString& title, const QString& question, const QString& add= "");

/**
 * @brief Ouvre un menu de selection de couleur
 * @param color La couleur initialement sélectionnée.
 * @return La couleur finalement sélectionnée.
 */
QColor colorSelection(const QColor& color);

}// namespace evl::gui::dialog
