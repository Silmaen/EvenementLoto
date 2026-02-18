/**
 * @file HelpView.h
 * @author Silmaen
 * @date 18/02/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"
#include "gui/utils/MarkdownParser.h"

#include <unordered_map>

namespace evl::gui::views {

/**
 * @brief Non-modal help view that renders the user documentation with markdown formatting.
 */
class HelpView final : public View {
public:
	/// Default constructor.
	HelpView();
	/// Default destructor.
	~HelpView() override;

	HelpView(const HelpView&) = delete;
	HelpView(HelpView&&) = delete;
	auto operator=(const HelpView&) -> HelpView& = delete;
	auto operator=(HelpView&&) -> HelpView& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "help_view"; }

private:
	/// Parsed markdown elements.
	std::vector<utils::MarkdownElement> m_elements;
	/// Whether the markdown has been loaded.
	bool m_loaded = false;
	/// Base path for documentation resources.
	std::filesystem::path m_basePath;

	/// Cached image texture info: texture ID and original size.
	struct ImageInfo {
		/// The texture ID.
		uint64_t textureId = 0;
		/// The image width.
		float width = 0.0f;
		/// The image height.
		float height = 0.0f;
	};
	/// Cache for loaded image textures.
	std::unordered_map<std::string, ImageInfo> m_imageCache;

	/**
	 * @brief Load the markdown documentation if not yet loaded.
	 */
	void loadIfNeeded();

	/**
	 * @brief Render text spans with bold support.
	 * @param iSpans The text spans to render.
	 */
	static void renderSpans(const std::vector<utils::TextSpan>& iSpans);

	/**
	 * @brief Render a single markdown element.
	 * @param iElement The element to render.
	 * @param iIndex The element index (for unique table IDs).
	 */
	void renderElement(const utils::MarkdownElement& iElement, size_t iIndex);

	/**
	 * @brief Get or load an image and return its display info.
	 * @param iImagePath The relative image path.
	 * @return The image info.
	 */
	auto getImageInfo(const std::string& iImagePath) -> const ImageInfo&;
};

}// namespace evl::gui::views
