/**
 * @file Rendering.cpp
 * @author Silmaen
 * @date 26/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Rendering.h"

#include "gui_imgui/Application.h"
#include <imgui.h>

namespace evl::gui_imgui::utils {

void defineActionButtonItem(const std::string& iLabel, const std::string& iActionName,
							const ActionButtonOptions& iOptions) {
	if (iOptions.sameLine) {
		ImGui::SameLine();
	}
	if (const auto action = Application::get().getAction(iActionName); action == nullptr) {
		log_warn("Action '{}' not found ({}).", iLabel, iActionName);
		ImGui::Button(iLabel.c_str());
	} else {
		if (iOptions.setDisabled) {
			if (iOptions.disabled) {
				action->disable();
			} else {
				action->enable();
			}
		}
		if (iOptions.disabled || !action->isEnabled()) {
			ImGui::BeginDisabled();
		}
		uint64_t texId = 0;
		if (action->hasIcon()) {
			const auto& texLib = Application::get().getTextureLibrary();
			texId = texLib.getTextureId(action->getIconName());
		}
		ImGui::PushID(iActionName.c_str());
		if (texId != 0) {
			if (iOptions.showLabel) {
				// Calculate total width needed for icon + text
				constexpr float iconSize = 24.0f;
				const ImVec2 spacing = ImGui::GetStyle().ItemInnerSpacing;
				const ImVec2 textSize = ImGui::CalcTextSize(iLabel.c_str());
				const ImVec2 padding = ImGui::GetStyle().FramePadding;
				const ImVec2 totalSize{padding.x * 2 + iconSize + spacing.x + textSize.x,
									   padding.y * 2 + std::max(iconSize, textSize.y)};
				const float rounding = ImGui::GetStyle().FrameRounding;

				// Create invisible button covering both icon and text
				if (ImGui::InvisibleButton(std::format("{}##btn", iLabel).c_str(), totalSize)) {
					action->execute();
				}
				// Get button state
				const bool isHovered = ImGui::IsItemHovered();
				const bool isActive = ImGui::IsItemActive();
				const bool isDisabled = iOptions.disabled || !action->isEnabled();

				// Draw button background with proper styling
				const ImVec2 buttonMin = ImGui::GetItemRectMin();
				const ImVec2 buttonMax = ImGui::GetItemRectMax();
				const ImVec2 contentMin{buttonMin.x + padding.x, buttonMin.y + padding.y};
				ImU32 bgColor = {};
				if (isActive) {
					bgColor = ImGui::GetColorU32(ImGuiCol_ButtonActive);
				} else if (isHovered) {
					bgColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
				} else {
					bgColor = ImGui::GetColorU32(ImGuiCol_Button);
				}
				ImGui::GetWindowDrawList()->AddRectFilled(buttonMin, buttonMax, bgColor, rounding);
				// Draw border
				ImGui::GetWindowDrawList()->AddRect(buttonMin, buttonMax, ImGui::GetColorU32(ImGuiCol_Border),
													rounding);
				// Draw icon with disabled tint if needed
				ImU32 iconTint = ImGui::GetColorU32(ImGuiCol_Text);
				if (isDisabled) {
					iconTint = ImGui::GetColorU32(ImGuiCol_TextDisabled);
				}
				// Draw icon and text on top
				ImGui::GetWindowDrawList()->AddImage(texId, contentMin,
													 ImVec2(contentMin.x + iconSize, contentMin.y + iconSize), {0, 0},
													 {1, 1}, iconTint);
				ImGui::GetWindowDrawList()->AddText(
						ImVec2(contentMin.x + iconSize + spacing.x, contentMin.y + (iconSize - textSize.y) * 0.5f),
						ImGui::GetColorU32(ImGuiCol_Text), iLabel.c_str());
			} else {
				if (ImGui::ImageButton(std::format("##{}", iLabel).c_str(), texId, {24.0f, 24.0f})) {
					action->execute();
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("%s", iLabel.c_str());
				}
			}
		} else {
			if (ImGui::Button(iLabel.c_str())) {
				action->execute();
			}
		}
		ImGui::PopID();
		if (iOptions.disabled || !action->isEnabled()) {
			ImGui::EndDisabled();
		}
	}
}

auto getNextStepStr(const core::Event& iEvent) -> std::string {
	switch (iEvent.getStatus()) {
		case core::Event::Status::MissingParties:
		case core::Event::Status::Invalid:
			return "Configuration requise";
		case core::Event::Status::Ready:
			return "Démarrer l'événement";
		case core::Event::Status::EventStarting:
			return "Demarrer la première partie";
		case core::Event::Status::DisplayRules:
			return "Terminer l'affichage des règles";
		case core::Event::Status::GameRunning:
			{
				const auto currentRound = iEvent.getCurrentCGameRound();
				if (currentRound->getType() == core::GameRound::Type::Pause) {
					return "Fin de la pause";
				}
				if (currentRound->getStatus() == core::GameRound::Status::Running) {
					if (currentRound->isCurrentSubRoundLast()) {
						if (iEvent.isCurrentGameRoundLast()) {
							return "Terminer l'événement";
						}
						if (iEvent.getNextCGameRound()->getType() == core::GameRound::Type::Pause) {
							return "Aller en pause";
						}
						return "Passer à la partie suivante";
					}
					return "Passer à la phase suivante";
				}
				return "Lancer la partie";
			}
		case core::Event::Status::EventEnding:
			return "Terminer l'événement";
		case core::Event::Status::Finished:
			return "Événement terminé";
	}
	return "Inconnu";
}

}// namespace evl::gui_imgui::utils
