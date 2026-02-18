/**
 * @file HelpView.cpp
 * @author Silmaen
 * @date 18/02/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "HelpView.h"

#include "core/Log.h"
#include "core/utilities.h"
#include "gui/Application.h"

#include <imgui.h>
#include <stb_image.h>

namespace evl::gui::views {

HelpView::HelpView() { hide(); }
HelpView::~HelpView() = default;

void HelpView::loadIfNeeded() {
	if (m_loaded)
		return;
	m_loaded = true;
	m_basePath = core::getExecPath() / "resources" / "documentation";
	const auto mdPath = m_basePath / "Utilisation.md";
	if (!exists(mdPath)) {
		log_warn("Documentation file not found: {}", mdPath.string());
		return;
	}
	m_elements = utils::parseMarkdownFile(mdPath);
	log_info("Loaded help documentation with {} elements.", m_elements.size());
}

void HelpView::renderSpans(const std::vector<utils::TextSpan>& iSpans) {
	for (size_t i = 0; i < iSpans.size(); ++i) {
		if (i > 0)
			ImGui::SameLine(0.0f, 0.0f);
		const auto& span = iSpans[i];
		if (span.bold) {
			// Faux-bold: draw text twice with 1px horizontal offset.
			const auto pos = ImGui::GetCursorScreenPos();
			const auto color = ImGui::GetColorU32(ImGuiCol_Text);
			ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + 1.0f, pos.y), color, span.text.c_str());
			ImGui::TextUnformatted(span.text.c_str());
		} else {
			ImGui::TextUnformatted(span.text.c_str());
		}
	}
}

auto HelpView::getImageInfo(const std::string& iImagePath) -> const ImageInfo& {
	if (const auto it = m_imageCache.find(iImagePath); it != m_imageCache.end())
		return it->second;

	const auto fullPath = m_basePath / iImagePath;
	auto& texLib = Application::get().getTextureLibrary();
	const auto texName = "doc_" + iImagePath;
	const auto texId = texLib.getOrLoadTextureId(texName, fullPath);

	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_info(fullPath.string().c_str(), &width, &height, &channels);

	auto& info = m_imageCache[iImagePath];
	info.textureId = texId;
	info.width = static_cast<float>(width);
	info.height = static_cast<float>(height);
	return info;
}

void HelpView::renderElement(const utils::MarkdownElement& iElement, const size_t iIndex) {
	switch (iElement.type) {
		case utils::MarkdownElementType::Heading1: {
			ImGui::Spacing();
			ImGui::SetWindowFontScale(1.8f);
			ImGui::TextWrapped("%s", iElement.rawText.c_str());
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Separator();
			ImGui::Spacing();
			break;
		}
		case utils::MarkdownElementType::Heading2: {
			ImGui::Spacing();
			ImGui::SetWindowFontScale(1.5f);
			ImGui::TextWrapped("%s", iElement.rawText.c_str());
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Separator();
			ImGui::Spacing();
			break;
		}
		case utils::MarkdownElementType::Heading3: {
			ImGui::Spacing();
			ImGui::SetWindowFontScale(1.3f);
			ImGui::TextWrapped("%s", iElement.rawText.c_str());
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Spacing();
			break;
		}
		case utils::MarkdownElementType::Heading4: {
			ImGui::Spacing();
			ImGui::SetWindowFontScale(1.15f);
			ImGui::TextWrapped("%s", iElement.rawText.c_str());
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Spacing();
			break;
		}
		case utils::MarkdownElementType::Paragraph: {
			renderSpans(iElement.spans);
			ImGui::Spacing();
			break;
		}
		case utils::MarkdownElementType::BulletItem: {
			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::TextWrapped("%s", iElement.rawText.c_str());
			break;
		}
		case utils::MarkdownElementType::Image: {
			const auto& imgInfo = getImageInfo(iElement.imagePath);
			if (imgInfo.textureId != 0 && imgInfo.width > 0.0f && imgInfo.height > 0.0f) {
				const float availWidth = ImGui::GetContentRegionAvail().x;
				float displayWidth = imgInfo.width;
				float displayHeight = imgInfo.height;
				if (displayWidth > availWidth) {
					const float scale = availWidth / displayWidth;
					displayWidth = availWidth;
					displayHeight *= scale;
				}
				// Center the image.
				const float offset = (availWidth - displayWidth) * 0.5f;
				if (offset > 0.0f)
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
				ImGui::Image(static_cast<ImTextureID>(imgInfo.textureId), ImVec2(displayWidth, displayHeight));
			} else {
				ImGui::TextDisabled("[Image: %s]", iElement.imageAlt.c_str());
			}
			ImGui::Spacing();
			break;
		}
		case utils::MarkdownElementType::TableHeader:
		case utils::MarkdownElementType::TableRow: {
			// Find consecutive table elements starting from this one and render as a single table.
			// Only render the table when we encounter the first table element of a group.
			// Check if previous element was also a table element - if so, skip (already rendered).
			if (iIndex > 0) {
				const auto& prev = m_elements[iIndex - 1];
				if (prev.type == utils::MarkdownElementType::TableHeader ||
					prev.type == utils::MarkdownElementType::TableRow)
					break;
			}

			// Count columns from the first table element.
			const auto cols = static_cast<int>(iElement.tableCells.size());
			if (cols == 0)
				break;

			const auto tableId = std::format("##help_table_{}", iIndex);
			if (ImGui::BeginTable(tableId.c_str(), cols,
								  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
				// Render all consecutive table elements.
				for (size_t j = iIndex; j < m_elements.size(); ++j) {
					const auto& tableElem = m_elements[j];
					if (tableElem.type != utils::MarkdownElementType::TableHeader &&
						tableElem.type != utils::MarkdownElementType::TableRow)
						break;
					ImGui::TableNextRow();
					for (size_t c = 0; c < tableElem.tableCells.size() && c < static_cast<size_t>(cols); ++c) {
						ImGui::TableSetColumnIndex(static_cast<int>(c));
						// Render cell spans.
						for (size_t s = 0; s < tableElem.tableCells[c].size(); ++s) {
							if (s > 0)
								ImGui::SameLine(0.0f, 0.0f);
							const auto& span = tableElem.tableCells[c][s];
							if (span.bold) {
								const auto pos = ImGui::GetCursorScreenPos();
								const auto color = ImGui::GetColorU32(ImGuiCol_Text);
								ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + 1.0f, pos.y), color,
																	span.text.c_str());
								ImGui::TextUnformatted(span.text.c_str());
							} else {
								ImGui::TextUnformatted(span.text.c_str());
							}
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::Spacing();
			break;
		}
		case utils::MarkdownElementType::Separator: break;
	}
}

void HelpView::onUpdate() {
	loadIfNeeded();

	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Aide", &visibility())) {
		ImGui::End();
		return;
	}

	for (size_t i = 0; i < m_elements.size(); ++i) { renderElement(m_elements[i], i); }

	ImGui::End();
}

}// namespace evl::gui::views
