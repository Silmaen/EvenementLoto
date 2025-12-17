/**
 * @file Popups.h
 * @author Silmaen
 * @date 17/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"

namespace evl::gui_imgui::views {

/**
 * @brief Class Popup.
 */
class Popup : public View {
public:
	/// Default constructor.
	Popup();
	/// Default destructor.
	~Popup() override;

	Popup(const Popup&) = delete;
	Popup(Popup&&) = delete;
	auto operator=(const Popup&) -> Popup& = delete;
	auto operator=(Popup&&) -> Popup& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onUpdate() final;

	/**
	 * @brief Function called at Update Time for the popup content.
	 */
	virtual void onPopupUpdate() = 0;

	/**
	 * @brief Open the popup.
	 */
	void open() { m_shouldOpen = true; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] virtual auto getPopupTitle() const -> std::string { return getName(); }

private:
	bool m_shouldOpen{false};
};

/**
 * @brief Class PopupAide.
 */
class PopupAide final : public Popup {
public:
	/// Default constructor.
	PopupAide();
	/// Default destructor.
	~PopupAide() override;

	PopupAide(const PopupAide&) = delete;
	PopupAide(PopupAide&&) = delete;
	auto operator=(const PopupAide&) -> PopupAide& = delete;
	auto operator=(PopupAide&&) -> PopupAide& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_aide"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Aide"; }
};

/**
 * @brief Class PopupAide.
 */
class PopupAbout final : public Popup {
public:
	/// Default constructor.
	PopupAbout();
	/// Default destructor.
	~PopupAbout() override;

	PopupAbout(const PopupAbout&) = delete;
	PopupAbout(PopupAbout&&) = delete;
	auto operator=(const PopupAbout&) -> PopupAbout& = delete;
	auto operator=(PopupAbout&&) -> PopupAbout& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_about"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "À propos"; }
};


}// namespace evl::gui_imgui::views
