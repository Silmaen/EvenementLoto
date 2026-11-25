/**
 * @file Rendering.cpp
 * @author Silmaen
 * @date 26/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Rendering.h"

#include "Convert.h"
#include "Layout.h"
#include "gui/Application.h"
#include <imgui.h>
#include <imgui_stdlib.h>

namespace evl::gui::utils {

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
				constexpr float iconSize = g_actionIconSize;
				const ImVec2 spacing = ImGui::GetStyle().ItemInnerSpacing;
				const ImVec2 textSize = ImGui::CalcTextSize(iLabel.c_str());
				constexpr ImVec2 padding{g_actionIconPadding, g_actionIconPadding};
				const ImVec2 totalSize{padding.x * 2 + iconSize + spacing.x + textSize.x,
									   padding.y * 2 + std::max(iconSize, textSize.y)};
				const float rounding = ImGui::GetStyle().FrameRounding;

				// Create invisible button covering both icon and text
				if (ImGui::InvisibleButton("##btn", totalSize)) {
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
				// La marge est imposée, pas héritée de l'habillage, pour que les
				// boutons de la barre d'outils fassent la même taille partout.
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {g_actionIconPadding, g_actionIconPadding});
				const bool clicked = ImGui::ImageButton("##icon", texId, {g_actionIconSize, g_actionIconSize});
				ImGui::PopStyleVar();
				if (clicked) {
					action->execute();
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("%s", iLabel.c_str());
				}
			}
		} else {
			if (ImGui::Button(std::format("{}##plain", iLabel).c_str())) {
				action->execute();
			}
		}
		ImGui::PopID();
		if (iOptions.disabled || !action->isEnabled()) {
			ImGui::EndDisabled();
		}
	}
}

auto confirmDialog(const char* iTitle, const std::string& iMessage, const char* iConfirmLabel) -> bool {
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
	bool confirmed = false;
	if (ImGui::BeginPopupModal(iTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
		ImGui::PushTextWrapPos(em(28.0f));
		ImGui::TextWrapped("%s", iMessage.c_str());
		ImGui::PopTextWrapPos();
		ImGui::Spacing();
		if (ImGui::Button(iConfirmLabel)) {
			confirmed = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Annuler"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	return confirmed;
}

auto renderPrizeList(core::prizes_type& ioPrizes, const PrizeListOptions& iOptions) -> bool {
	const bool iEditable = iOptions.editable;
	bool changed = false;
	// Au-dessus de la liste : sous une liste qui remplissait sa zone, le bouton se
	// retrouvait hors champ et rien ne semblait permettre d'ajouter un lot.
	if (iOptions.withAddButton) {
		if (!iEditable)
			ImGui::BeginDisabled();
		if (ImGui::Button("Ajouter un lot")) {
			ioPrizes.emplace_back();
			changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Vider la liste") && !ioPrizes.empty())
			ImGui::OpenPopup("Vider la liste ?");
		if (confirmDialog("Vider la liste ?", std::format("Retirer les {} lot(s) de cette liste ?", ioPrizes.size()),
						  "Vider")) {
			ioPrizes.clear();
			changed = true;
		}
		if (!iEditable)
			ImGui::EndDisabled();
	}
	if (ImGui::BeginChild("PrizeList", vec2ToImVec2(iOptions.size), ImGuiChildFlags_Borders)) {
		if (ioPrizes.empty())
			ImGui::TextDisabled("Aucun lot pour cette phase.");
		// Everything after the donor has a fixed width; the two names share what is left.
		// Below a certain width they cannot both stay readable, and the designation then
		// gets a line of its own rather than being cut to a few letters.
		const auto& style = ImGui::GetStyle();
		const float checkboxWidth =
				ImGui::GetFrameHeight() + style.ItemInnerSpacing.x + ImGui::CalcTextSize("enfant").x;
		const float removeWidth = ImGui::CalcTextSize("Retirer").x + style.FramePadding.x * 2.0f;
		const float avail = ImGui::GetContentRegionAvail().x;
		// Two arrow buttons move the article up or down: that is the order it is shown in.
		const float arrowsWidth = ImGui::GetFrameHeight() * 2.0f + style.ItemSpacing.x * 2.0f;
		const float setWidth = iOptions.withSets ? em(6.5f) + style.ItemSpacing.x : 0.0f;
		const float wideTail =
				em(11.5f) + checkboxWidth + removeWidth + arrowsWidth + setWidth + style.ItemSpacing.x * 4.0f;
		const bool compact = avail - wideTail < em(16.0f);
		// Compact, the names share the first line and the rest gets the whole second one.
		const float fieldsRoom = compact ? avail - checkboxWidth - removeWidth - arrowsWidth - setWidth - style.ItemSpacing.x * 3.0f : em(11.5f);
		const float valueWidth = std::max(fieldsRoom * 0.48f, em(3.0f));
		const float appealWidth = std::max(fieldsRoom * 0.52f, em(3.0f));
		const float tail = compact ? 0.0f : wideTail;
		for (size_t i = 0; i < ioPrizes.size(); ++i) {
			auto& prize = ioPrizes[i];
			ImGui::PushID(static_cast<int>(i));
			if (compact && i > 0)
				ImGui::Separator();
			const float names = avail - tail - style.ItemSpacing.x;
			ImGui::SetNextItemWidth(names * 0.6f);
			std::string designation = prize.getDesignation();
			if (ImGui::InputTextWithHint("##designation", "désignation", &designation) && iEditable) {
				prize.setDesignation(designation);
				changed = true;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(names * 0.4f);
			std::string donor = prize.getDonor();
			if (ImGui::InputTextWithHint("##donor", "donateur", &donor) && iEditable) {
				prize.setDonor(donor);
				changed = true;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Donateur, facultatif");
			if (!compact)
				ImGui::SameLine();
			ImGui::SetNextItemWidth(valueWidth);
			auto value = static_cast<float>(prize.getValue());
			if (ImGui::InputFloat("##value", &value, 0.0f, 0.0f, "%.2f €") && iEditable) {
				prize.setValue(static_cast<double>(value));
				changed = true;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(appealWidth);
			auto attractiveness = static_cast<int>(prize.getAttractiveness());
			if (ImGui::SliderInt("##attractiveness", &attractiveness, 0,
								 static_cast<int>(core::Prize::g_maxAttractiveness), "attrait %d") &&
				iEditable) {
				prize.setAttractiveness(static_cast<uint8_t>(attractiveness));
				changed = true;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Envie qu'il suscite, indépendamment de son prix. 0 : non noté.");
			ImGui::SameLine();
			bool childFriendly = prize.isChildFriendly();
			if (ImGui::Checkbox("enfant", &childFriendly) && iEditable) {
				prize.setChildFriendly(childFriendly);
				changed = true;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("L'article peut être mis en jeu dans une partie enfant.");
			if (iOptions.withSets) {
				ImGui::SameLine();
				ImGui::SetNextItemWidth(em(6.5f));
				auto set = static_cast<int>(prize.getSet());
				if (ImGui::SliderInt("##set", &set, 0, static_cast<int>(core::Prize::g_maxSet), "gagnant %d") &&
					iEditable) {
					prize.setSet(static_cast<uint8_t>(set));
					changed = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Le gagnant qui emporte cet article, avec les autres du même numéro.\n"
									  "0 : l'article suit celui qui le précède dans la liste.");
			}
			ImGui::SameLine();
			// A move changes the list under the loop: the frame ends there, like a removal.
			const bool moveUp = ImGui::ArrowButton("##up", ImGuiDir_Up) && iEditable && i > 0;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Monter : le lot s'affiche plus tôt.");
			ImGui::SameLine();
			const bool moveDown = ImGui::ArrowButton("##down", ImGuiDir_Down) && iEditable && i + 1 < ioPrizes.size();
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Descendre : le lot s'affiche plus tard.");
			if (moveUp || moveDown) {
				std::swap(ioPrizes[i], ioPrizes[moveUp ? i - 1 : i + 1]);
				ImGui::PopID();
				ImGui::EndChild();
				return true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Retirer") && iEditable) {
				ioPrizes.erase(ioPrizes.begin() + static_cast<ptrdiff_t>(i));
				ImGui::PopID();
				ImGui::EndChild();
				return true;
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	return changed;
}

auto getNextStepStr(const core::Event& iEvent) -> std::string {
	switch (iEvent.getStatus()) {
		case core::Event::Status::MissingParties:
		case core::Event::Status::Invalid:
			return "Configuration requise";
		case core::Event::Status::Ready:
			return "Démarrer l'événement";
		case core::Event::Status::EventStarting:
			return "Démarrer la première partie";
		case core::Event::Status::DisplayRules:
			return "Terminer l'affichage des règles";
		case core::Event::Status::GameRunning:
			{
				const auto currentRound = iEvent.getCurrentCGameRound();
				if (currentRound == iEvent.endRounds())
					return "Terminer l'événement";
				if (currentRound->getType() == core::GameRound::Type::Pause) {
					return "Fin de la pause";
				}
				const auto sub = currentRound->getCurrentSubRound();
				const bool running = currentRound->getStatus() == core::GameRound::Status::Running &&
									 sub != currentRound->endSubRound();
				if (running) {
					// What the button does depends on the phase, not on how many are left: the lots
					// are shown first, then the draws run until somebody wins.
					if (sub->getStatus() == core::SubGameRound::Status::PreScreen)
						return "Lancer les tirages";
					if (currentRound->getType() == core::GameRound::Type::Enfant)
						return "Désigner les gagnants";
					return "Désigner le gagnant";
				}
				if (currentRound->getStatus() == core::GameRound::Status::Ready)
					return "Lancer la partie";
				// The round is over and its end screen is up: the button leaves it.
				if (iEvent.isCurrentGameRoundLast())
					return "Terminer l'événement";
				if (iEvent.getNextCGameRound()->getType() == core::GameRound::Type::Pause)
					return "Aller en pause";
				return "Passer à la partie suivante";
			}
		case core::Event::Status::EventEnding:
			return "Terminer l'événement";
		case core::Event::Status::Finished:
			return "Événement terminé";
	}
	return "Inconnu";
}

void adaptTextToRegion(const std::string& iText, const TextAdaptOptions& iOptions) {
	ImVec2 numberSize;
	if (iOptions.autoRegion) {
		numberSize = ImGui::GetContentRegionAvail();
	} else {
		numberSize = vec2ToImVec2(iOptions.contentSize);
	}
	auto numberTextSize = ImGui::CalcTextSize(iText.c_str());
	if (!iOptions.textAdapt.empty())
		numberTextSize = ImGui::CalcTextSize(iOptions.textAdapt.c_str());
	const float scaleX = numberSize.x / numberTextSize.x;
	const float scaleY = numberSize.y / numberTextSize.y;
	const float scale = std::min(scaleX, scaleY) * 0.9f;// 90% of the available space
	if (scale <= 0.0f) {
		// Nothing can be written in a degenerate region. An item still has to follow the
		// caller's cursor move, otherwise ImGui cannot grow the parent.
		if (iOptions.drawText)
			ImGui::Dummy({0.0f, 0.0f});
		return;
	}
	ImGui::SetWindowFontScale(scale);
	if (iOptions.hCenter) {
		const float centerX = (numberSize.x - numberTextSize.x * scale) * 0.5f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerX);
	}
	if (iOptions.vCenter) {
		const float centerY = (numberSize.y - numberTextSize.y * scale) * 0.5f;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + centerY);
	}
	if (iOptions.drawText) {
		ImGui::Text("%s", iText.c_str());
		ImGui::SetWindowFontScale(1.0f);
	}
}

}// namespace evl::gui::utils
