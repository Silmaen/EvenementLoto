/**
 * @file CsvCatalogue.h
 * @author Silmaen
 * @date 28/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "Event.h"
#include "Prize.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace evl::core {

/// La place qu'une ligne donne à son article, en numéros d'écran (à partir de 1).
struct CsvPlacement {
	std::size_t prize = 0;///< Position de l'article dans `CsvImport::prizes`.
	uint32_t round = 1;///< Numéro défini de la partie (son champ « N° »), non son rang.
	uint32_t phase = 1;///< Numéro de la phase dans la partie.
};

/// Ce qu'une lecture de tableur a donné.
struct CsvImport {
	/// Les articles lus.
	prizes_type prizes;
	/// Où le fichier place ceux qu'il place, dans l'ordre des lignes : celui de l'affichage.
	std::vector<CsvPlacement> placements;
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
 * - un fichier **déjà réparti** porte en plus les colonnes « Partie » et « Phase » (numéros) :
 *   chaque article y dit où il joue, et l'ordre des lignes d'une phase est celui de
 *   l'affichage. Sans « Phase », la première est supposée ;
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

/// Ce qu'a donné la mise en place des articles importés.
struct CsvPlacementResult {
	std::size_t placed = 0;///< Articles mis en jeu où le fichier le demandait.
	std::size_t refused = 0;///< Articles restés au catalogue : partie inconnue ou phase entamée.
};

/**
 * @brief Met en jeu les articles qu'un tableur place, dans l'ordre de ses lignes.
 *
 * Les articles doivent déjà être entrés au catalogue de l'événement, à la suite de ce
 * qui s'y trouvait : `iFirstNew` est leur première position. Un article dont la place
 * n'existe pas, ou ne se laisse pas modifier, reste au catalogue sans emploi.
 *
 * @param ioEvent L'événement.
 * @param iFirstNew Position, au catalogue, du premier article importé.
 * @param iImport Le tableur lu.
 * @return Ce qui a été placé et ce qui ne l'a pas été.
 */
auto placeImported(Event& ioEvent, std::size_t iFirstNew, const CsvImport& iImport) -> CsvPlacementResult;

/**
 * @brief Lit un catalogue de lots depuis un fichier de tableur.
 * @param iPath Le fichier à lire.
 * @return Les articles lus et ce qu'il faut en dire, `read` à faux si le fichier n'a
 *         pas pu être ouvert.
 */
[[nodiscard]] auto importCatalogueCsv(const std::filesystem::path& iPath) -> CsvImport;

}// namespace evl::core
