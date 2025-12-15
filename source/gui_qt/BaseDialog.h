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
enum struct FileTypes : uint8_t {
	EventSave,/// La sauvegarde d’un événement
	Images,/// La liste des images supportées
	JSON,/// Un fichier JSON
	Text,/// Fichiers textes ou markdown
	ThemeFile,/// Fichier de thème
	Folder,/// Un dossier
};

/**
 * @brief Ouverture d’une fenêtre de dialogue pour l’ouverture de fichier
 * @param iType Le type de contenu à filter.
 * @param iExist Si le fichier doit déjà exister (mode ouverture)
 * @return Le chemin, vide si échec ou annulation.
 */
std::filesystem::path openFile(const FileTypes& iType = FileTypes::EventSave, bool iExist = false);

/**
 * @brief Ouvre une fenêtre de dialogue posant une question oui ou non
 * @param iTitle Le titre de la fenêtre
 * @param iQuestion Le texte de la question.
 * @param iAdd Le texte additionnel.
 * @return La réponse à la question
 */
bool question(const QString& iTitle, const QString& iQuestion, const QString& iAdd = "");

/**
 * @brief Ouvre un menu de selection de couleur
 * @param iColor La couleur initialement sélectionnée.
 * @return La couleur finalement sélectionnée.
 */
QColor colorSelection(const QColor& iColor);

}// namespace evl::gui::dialog
