/**
 * @file CsvCatalogue.h
 * @author Silmaen
 * @date 28/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "Prize.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace evl::core {

/// Ce qu'une lecture de tableur a donné.
struct CsvImport {
	/// Les articles lus.
	prizes_type prizes;
	/// Les lignes de données ignorées, faute de désignation comme de valeur.
	std::size_t skipped = 0;
	/// True si le fichier a pu être lu, même sans y trouver d'article.
	bool read = false;
	/// Ce qu'il faut dire à l'organisateur, en une phrase.
	std::string summary;
};

/**
 * @brief Lit un catalogue de lots dans du texte séparé par des délimiteurs.
 *
 * Écrit pour accepter ce qu'un tableur produit vraiment, et non un format idéal :
 *
 * - le **séparateur** est deviné sur la première ligne, point-virgule, virgule ou
 *   tabulation — un tableur français exporte des points-virgules ;
 * - les **guillemets** protègent un séparateur dans un champ, `""` valant un guillemet ;
 * - la **virgule décimale** est acceptée autant que le point, et les espaces comme le
 *   symbole euro sont ignorés dans un prix ;
 * - l'**en-tête** est reconnu sur les noms de colonnes usuels, en français comme en
 *   anglais, accentués ou non ; sans en-tête reconnu, l'ordre désignation, donateur,
 *   valeur, attrait, enfant est supposé ;
 * - les colonnes peuvent venir dans n'importe quel ordre et **manquer** : seule la
 *   désignation compte vraiment.
 *
 * Une ligne sans désignation ni valeur est comptée comme ignorée plutôt que rejetée : un
 * export de tableur se termine souvent par des lignes vides, ce n'est pas une erreur.
 *
 * @param iContent Le contenu du fichier.
 * @return Les articles lus et ce qu'il faut en dire.
 */
[[nodiscard]] auto parseCatalogueCsv(std::string_view iContent) -> CsvImport;

/**
 * @brief Lit un catalogue de lots depuis un fichier de tableur.
 * @param iPath Le fichier à lire.
 * @return Les articles lus et ce qu'il faut en dire, `read` à faux si le fichier n'a
 *         pas pu être ouvert.
 */
[[nodiscard]] auto importCatalogueCsv(const std::filesystem::path& iPath) -> CsvImport;

}// namespace evl::core
