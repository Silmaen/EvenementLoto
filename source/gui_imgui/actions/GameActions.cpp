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
#include "gui_imgui/Application.h"


namespace evl::gui_imgui::actions {


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
		currentEvent.displayRules();// The call in this state willrestore previous state.
		return;// No further action needed.
	}
	bool goNext = true;
	if (currentEvent.getStatus() == core::Event::Status::GameRunning) {
		if (const auto round = currentEvent.getCurrentCGameRound();
			round->getType() != core::GameRound::Type::Pause &&
			round->getStatus() == core::GameRound::Status::Running) {
			if (round->getCurrentSubRound()->getStatus() == core::SubGameRound::Status::Running) {
				currentEvent.addWinnerToCurrentRound("john_doe");// Placeholder for winner input
				goNext = false;
			}
		}
	}
	if (goNext) {
		currentEvent.nextState();
	}
	if (const auto round = currentEvent.getCurrentCGameRound();
		round->getType() != core::GameRound::Type::Pause && round->drawsCount() == 0) {
		Application::get().getRng().resetPick();
	}
}

RandomPickAction::RandomPickAction() { setIconName("dice"); }
RandomPickAction::~RandomPickAction() = default;
void RandomPickAction::onExecute() {
	auto& event = Application::get().getCurrentEvent();
	if (!event.canDraw())
		return;
	event.getCurrentGameRound()->addPickedNumber(Application::get().getRng().pick());
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
	log_trace("Cancel pick action executed.");
}

DisplayRulesAction::DisplayRulesAction() { setIconName("terms-and-conditions"); }
DisplayRulesAction::~DisplayRulesAction() = default;
void DisplayRulesAction::onExecute() {
	auto& event = Application::get().getCurrentEvent();
	event.displayRules();
}

}// namespace evl::gui_imgui::actions
