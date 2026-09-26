/**
 * @file GameActions.cpp
 * @author Silmaen
 * @date 26/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "GameActions.h"

#include "core/Log.h"
#include "gui/Application.h"


namespace evl::gui::actions {


StartGameAction::StartGameAction() { setIconName("toggle-on"); }
StartGameAction::~StartGameAction() = default;
void StartGameAction::onExecute() {
	auto& app = Application::get();
	if (app.getCurrentEvent().getStatus() != core::Event::Status::Ready) {
		return;
	}
	app.getCurrentEvent().nextState();
	log_trace("Start game action executed.");
}


StopGameAction::StopGameAction() { setIconName("toggle-off"); }
StopGameAction::~StopGameAction() = default;
void StopGameAction::onExecute() { log_trace("Stop game action executed."); }

GameNextActions::GameNextActions() { setIconName("submit-for-approval"); }
GameNextActions::~GameNextActions() = default;
void GameNextActions::onExecute() {
	auto& currentEvent = Application::get().getCurrentEvent();

	if (currentEvent.getStatus() == core::Event::Status::DisplayRules) {
		currentEvent.displayRules();// The call in this state will restore previous state.
		return;// No further action needed.
	}
	if (currentEvent.getStatus() == core::Event::Status::GameRunning) {
		if (const auto round = currentEvent.getCurrentCGameRound();
			round->getType() != core::GameRound::Type::Pause &&
			round->getStatus() == core::GameRound::Status::Running) {
			if (round->getCurrentSubRound()->getStatus() == core::SubGameRound::Status::Running) {
				// Ending a sub-round means awarding its prize: who won is asked for, and
				// the popup is what moves the game on once it knows.
				if (const auto popup = Application::get().getPopup("popup_winner")) {
					popup->open();
					return;
				}
				log_warn("Popup 'popup_winner' not found, manche validée sans gagnant.");
				currentEvent.addWinnerToCurrentRound({});
			}
		}
	}
	currentEvent.nextState();
	if (const auto round = currentEvent.getCurrentCGameRound(); round != currentEvent.endRounds() &&
																round->getType() != core::GameRound::Type::Pause &&
																round->drawsCount() == 0) {
		Application::get().getRng().resetPick();
	}
	// A new phase starts with its controls armed, whatever the tempo of the last draw.
	Application::get().clearDrawDelay();
	Application::get().saveProgress();
}

RandomPickAction::RandomPickAction() { setIconName("dice"); }
RandomPickAction::~RandomPickAction() = default;
void RandomPickAction::onExecute() {
	auto& app = Application::get();
	auto& event = app.getCurrentEvent();
	if (!event.canDraw() || app.isDrawHeld())
		return;
	event.getCurrentGameRound()->addPickedNumber(app.getRng().pick());
	app.notifyDraw();
	app.saveProgress();
	log_trace("Random pick action executed.");
}

CancelPickAction::CancelPickAction() { setIconName("clear-symbol"); }
CancelPickAction::~CancelPickAction() = default;
void CancelPickAction::onExecute() {
	auto& event = Application::get().getCurrentEvent();
	if (!event.canDraw())
		return;
	event.getCurrentGameRound()->removeLastPick();
	Application::get().getRng().popNum();
	Application::get().clearDrawDelay();
	Application::get().saveProgress();
	log_trace("Cancel pick action executed.");
}

DisplayRulesAction::DisplayRulesAction() { setIconName("terms-and-conditions"); }
DisplayRulesAction::~DisplayRulesAction() = default;
void DisplayRulesAction::onExecute() {
	auto& event = Application::get().getCurrentEvent();
	event.displayRules();
}

WinnersAction::WinnersAction() { setIconName("euro-money"); }
WinnersAction::~WinnersAction() = default;
void WinnersAction::onExecute() {
	if (const auto popup = Application::get().getPopup("popup_winners")) {
		popup->open();
	} else {
		log_warn("Popup 'popup_winners' not found.");
	}
}

QuickGameAction::QuickGameAction() { setIconName("new-file"); }
QuickGameAction::~QuickGameAction() = default;
void QuickGameAction::onExecute() {
	if (const auto popup = Application::get().getPopup("popup_quick_game")) {
		popup->open();
	} else {
		log_warn("Popup 'popup_quick_game' not found.");
	}
}

}// namespace evl::gui::actions
