/**
 * @file CataloguePopup.h
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Popups.h"
#include "core/Distribution.h"

namespace evl::gui::views {

/**
 * @brief The prize catalogue of the event, and the tool that spreads it over the rounds.
 *
 * Everything the organizer has, entered in one go and independently of the programme:
 * that is the raw material. The distribution then places it, with the two climbs the
 * afternoon needs — one inside each round, one across the whole event — and whatever it
 * decides stays adjustable by hand afterwards, in the round configuration.
 */
class PopupCatalogue final : public Popup {
public:
	/// Default constructor.
	PopupCatalogue();
	/// Default destructor.
	~PopupCatalogue() override;

	PopupCatalogue(const PopupCatalogue&) = delete;
	PopupCatalogue(PopupCatalogue&&) = delete;
	auto operator=(const PopupCatalogue&) -> PopupCatalogue& = delete;
	auto operator=(PopupCatalogue&&) -> PopupCatalogue& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_catalogue"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Catalogue des lots"; }

	/**
	 * @brief Demande l'affichage de la page « Répartition » à la prochaine image.
	 *
	 * Ce que fait le bouton « Répartir » : on veut voir le résultat, pas aller le
	 * chercher.
	 */
	void showDistribution() { m_showDistribution = true; }

	/**
	 * @brief Relit le catalogue depuis l'événement.
	 *
	 * La page « Répartition » lit le catalogue que le popup détient : après une
	 * modification venue d'ailleurs, il doit être remis à jour.
	 */
	void refreshCatalogue();

protected:
	/**
	 * @brief Function called when the popup is opened.
	 */
	void onOpen() override;

private:
	/// Page « Catalogue » : la liste des articles et la répartition automatique.
	void renderCatalogueTab();
	/// Page « Répartition » : les courbes, le détail par manche et l'ajustement à la main.
	void renderDistributionTab();
	/// Les deux histogrammes : par manche, puis par partie.
	static void renderCharts(const core::DistributionOverview& iOverview);
	/// Le tableau des articles et de leur affectation.
	void renderAssignmentTable(const core::DistributionOverview& iOverview);

	/// The catalogue being edited, written back to the event on every change.
	core::prizes_type m_catalogue;
	/// How the distribution should weigh things.
	core::DistributionSettings m_settings;
	/// What the last distribution did, to be told once rather than guessed.
	std::string m_lastResult;
	/// Vrai quand la page « Répartition » doit passer devant.
	bool m_showDistribution = false;
};

}// namespace evl::gui::views
