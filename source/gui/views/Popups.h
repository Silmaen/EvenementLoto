/**
 * @file Popups.h
 * @author Silmaen
 * @date 17/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"
#include "core/maths/vectors.h"

namespace evl::gui::views {

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

	/**
	 * @brief How a popup wants to be laid out, in font heights.
	 *
	 * Sized in font heights rather than pixels so it follows the font size, and always
	 * kept within the control window: a dialog wider than its window got a platform
	 * window of its own, on whichever screen.
	 */
	struct Layout {
		/// Width and height, in font heights.
		math::vec2 size{40.0f, 30.0f};
		/// The height follows the content, for the short dialogs.
		bool fitHeight = false;
	};

	/**
	 * @brief The layout this popup asks for.
	 * @return Its layout.
	 */
	[[nodiscard]] virtual auto layout() const -> Layout { return {}; }

protected:
	/// Function called when the popup is opened.
	virtual void onOpen() {}

private:
	/// Flag to indicate if the popup should be opened.
	bool m_shouldOpen{false};
};

}// namespace evl::gui::views
