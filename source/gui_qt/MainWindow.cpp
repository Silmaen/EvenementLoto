/**
 * @file MainWindow.cpp
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "About.h"
#include "BaseDialog.h"
#include "ConfigEvent.h"
#include "ConfigGameRounds.h"
#include "ConfigGeneral.h"
#include "MainWindow.h"
#include "WinnerInput.h"
#include <QFileDialog>
#include <QScreen>
#include <QScrollBar>
#include <core/Log.h>
#include <core/StringUtils.h>

// Les trucs de QT
#include <moc_MainWindow.cpp>
#include <ui/ui_MainWindow.h>

namespace evl::gui {

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::MainWindow),
	  settings{QString(getIniFile().generic_u8string().c_str()), QSettings::IniFormat}, timer(new QTimer(this)),
	  numberGrid(new WidgetNumberGrid()), theme(&settings) {
	// initialise l’ui depuis le fichier ui.
	ui->setupUi(this);
	// initialise la numberGrid
	const auto layout = new QGridLayout(ui->GroupPickManual);
	numberGrid->setParent(ui->GroupPickManual);
	numberGrid->setObjectName(QString::fromUtf8("numberGrid"));
	layout->addWidget(numberGrid, 0, 0, 1, 1);
	connect(numberGrid, &WidgetNumberGrid::buttonPushed, this, &MainWindow::actGridPushed);
	// connecte le timer à la fonction de mise à jour de l’affichage en jeu.
	connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::updateClocks));
	timer->setInterval(100);
	updateDisplay();
}

MainWindow::~MainWindow() {
	delete ui;
	delete timer;
	delete numberGrid;
	if (displayWindow != nullptr)
		delete displayWindow;
}

void MainWindow::actShowAbout() {
	evl::gui::About AboutWindow(this);
	AboutWindow.setModal(true);
	AboutWindow.exec();
}

void MainWindow::actShowHelp() { showNotImplemented("aide"); }

void MainWindow::actShowGlobalParameters() {
	ConfigGeneral cfg(this);
	cfg.preExec();
	cfg.exec();
}

void MainWindow::actShowGameRoundsParameters() {
	ConfigGameRounds cfg(this);
	cfg.setEvent(currentEvent);
	cfg.preExec();
	if (cfg.exec() == QDialog::Accepted) {
		currentEvent = cfg.getEvent();
		updateDisplay();
	}
}

void MainWindow::actShowEventParameters() {
	ConfigEvent cfg;
	cfg.setEvent(currentEvent);
	if (cfg.exec() == QDialog::Accepted) {
		currentEvent = cfg.getEvent();
		updateDisplay();
	}
}

void MainWindow::actNewFile() {
	currentEvent = core::Event();
	currentFile = std::filesystem::path{};
	updateDisplay();
}

void MainWindow::actLoadFile() {
	if (const std::filesystem::path file = dialog::openFile(dialog::FileTypes::EventSave, true); !file.empty()) {
		std::ifstream f;
		f.open(file, std::ios::in | std::ios::binary);
		currentEvent.setBasePath(file);
		currentEvent.read(f, 0);
		f.close();
		currentFile = file;
	}
	updateDisplay();
}

void MainWindow::actSaveFile() {
	if (currentEvent.getStatus() == core::Event::Status::Invalid)
		return;
	if (currentFile.empty()) {
		actSaveAsFile();
		return;
	}
	saveFile(currentFile);
}

void MainWindow::saveFile(const std::filesystem::path& where) {
	std::ofstream f;
	f.open(where, std::ios::out | std::ios::binary);
	currentEvent.setBasePath(currentFile.parent_path());
	// copy image file if not in child directory
	if (!currentEvent.getLogo().empty()) {
		if (std::filesystem::path ori = currentEvent.getLogoFull(); ori.parent_path() != currentEvent.getBasePath()) {
			std::filesystem::path dest = currentEvent.getBasePath() / ori.filename();
			if (exists(dest))
				remove(dest);
			copy_file(ori, dest);
			currentEvent.setLogo(dest);
		}
	}
	if (!currentEvent.getOrganizerLogo().empty()) {
		if (std::filesystem::path ori = currentEvent.getOrganizerLogoFull();
			ori.parent_path() != currentEvent.getBasePath()) {
			std::filesystem::path dest = currentEvent.getBasePath() / ori.filename();
			if (exists(dest))
				remove(dest);
			copy_file(ori, dest);
			currentEvent.setOrganizerLogo(dest);
		}
	}
	currentEvent.write(f);
	f.close();
}

void MainWindow::actSaveAsFile() {
	if (currentEvent.getStatus() == core::Event::Status::Invalid)
		return;
	std::filesystem::path file = dialog::openFile(dialog::FileTypes::EventSave, false);
	if (file.empty())
		return;
	if (!file.has_extension() || file.extension() != ".lev") {
		log_warn("Ajout de l'extension .lev au fichier de sauvegarde");
		file += ".lev";
	}
	currentFile = file;
	actSaveFile();
}

void MainWindow::actStartEvent() {
	if (currentEvent.getStatus() == core::Event::Status::Ready) {
		if (!dialog::question(
					"Démarrage de l’événement.", "Êtes-vous sûr de vouloir démarrer l’événement ?",
					"Une fois démarré, la configuration de l’événement et de ses parties ne pourra plus être modifié."))
			return;
		currentEvent.nextState();
	}
	updateDisplay();
	timer->start();
	displayWindow = new DisplayWindow(&currentEvent, this);
	actChangeFullScreen();
}

void MainWindow::actChangeScreen() {
	int screenId = ui->SelectScreen->currentIndex();
	if (QList<QScreen*> screens = QApplication::screens(); screens.size() > 2) {
		int id = 0;
		int mainId = 0;
		for (const QScreen* scr: screens) {
			if (scr == screen()) {
				mainId = id;
				break;
			}
			++id;
		}
		if (screenId == mainId) {
			screenId = (mainId + 1) % screens.size();
		}
		settings.setValue("display/screen_id", screenId);
		log_trace("setting screen {}", screenId);
	} else {
		settings.setValue("display/screen_id", -1);
	}
	syncSettings();
	actChangeFullScreen();
}

void MainWindow::actChangeFullScreen() {
	if (displayWindow == nullptr)
		return;

	const bool fullScreen = ui->CheckFullScreen->isChecked();
	settings.setValue("display/full_screen", fullScreen);
	int screenId = settings.value("display/screen_id", 0).toInt();
	QList<QScreen*> screens = QApplication::screens();
	if (screenId < 0 || !fullScreen) {
		displayWindow->showNormal();
	} else {
		log_trace("using screen {}", screenId);
		const QRect sizes = screens[screenId]->geometry();
		displayWindow->move(sizes.x(), sizes.y());
		displayWindow->showFullScreen();
	}
	syncSettings();
}

void MainWindow::actEndEvent() {
	if (!dialog::question("Terminer de l’événement.", "Êtes-vous sûr de vouloir terminer l’événement ?",
						  "Une fois terminé, l’événement ne pourra plus être démarré à nouveau."))
		return;
	timer->stop();
	displayWindow->hide();
	delete displayWindow;
	displayWindow = nullptr;
	updateDisplay();
}

void MainWindow::actQuit() { close(); }

void MainWindow::actStartStopRound() {
	timer->stop();
	bool goNext = true;
	if (currentEvent.getStatus() == core::Event::Status::GameRunning) {
		if (const auto round = currentEvent.getCurrentCGameRound();
			round->getType() != core::GameRound::Type::Pause &&
			round->getStatus() == core::GameRound::Status::Running) {
			if (const auto subround = round->getCurrentSubRound();
				subround->getStatus() == core::SubGameRound::Status::Running) {
				if (WinnerInput wi(this); wi.exec() == QDialog::Accepted)
					currentEvent.addWinnerToCurrentRound(wi.getWinner().toStdString());
				goNext = false;
			}
		}
	}
	if (goNext) {
		currentEvent.nextState();
	}
	updateDisplay();
}

void MainWindow::actRandomPick() {
	const auto number = rng.pick();
	numberGrid->setPushed(number);
	currentEvent.getCurrentGameRound()->addPickedNumber(number);
	updateDisplay();
}

void MainWindow::actCancelPick() {
	const auto cur = currentEvent.getCurrentGameRound();
	const uint8_t lastOne = cur->getLastCancelableDraw();
	if (lastOne == 255)
		return;
	numberGrid->resetPushed(lastOne);
	cur->removeLastPick();
	updateDisplay();
}

void MainWindow::actRadioPureRandom() {
	currentDrawMode = DrawMode::PickOnly;
	updateDisplay();
}

void MainWindow::actRadioPureManual() {
	currentDrawMode = DrawMode::ManualOnly;
	updateDisplay();
}

void MainWindow::actRadioBoth() {
	currentDrawMode = DrawMode::Both;
	updateDisplay();
}

void MainWindow::actDisplayRules() {
	if (currentEvent.getStatus() == core::Event::Status::DisplayRules)
		currentEvent.nextState();
	else
		currentEvent.displayRules();
	updateDisplay();
}

void MainWindow::syncSettings() { settings.sync(); }

void MainWindow::updateClocks() {
	//
	// Cette fonction est exécutée très régulièrement et doit être super rapide.
	//
	// Horloge principale
	ui->CurrentTime->setText(QString::fromStdString(core::formatClock(core::clock::now())));
	// update logfile
	logUpdateCounter++;
	if (ui->tabWidget->tabText(ui->tabWidget->currentIndex()) == "Log" && logUpdateCounter > 20) {
		logUpdateCounter = 0;
		std::ifstream file(getLogPath(), std::ios::in);
		QString text;
		for (std::string line; getline(file, line);) { text += QString::fromStdString(line) + "\n"; }
		ui->textLog->setText(text);
		ui->textLog->verticalScrollBar()->setValue(ui->textLog->verticalScrollBar()->maximum());
	}
	// Mise à jour des stats
	updateStats();
	// horloge partie en cours
	auto cur = currentEvent.getCurrentCGameRound();
	// check si la fenêtre d'affichage est ouverte ou fermée
	checkDisplayWindow();
	if (cur == currentEvent.endRounds())// pas de round en cours
		return;
	if (cur->getEnding() != core::g_epoch)// round déjà fini, pas de mise à jour
		return;
	if (cur->getStarting() == core::g_epoch)// round pas commencé, pas de mise à jour
		return;
	ui->RoundDuration->setText(QString::fromStdString(core::formatDuration(core::clock::now() - cur->getStarting())));

	// event chrono
	core::time_point start = currentEvent.getStarting();
	if (core::time_point end = currentEvent.getEnding(); start != core::g_epoch && end == core::g_epoch)
		ui->Duration->setText(QString::fromUtf8(core::formatDuration(core::clock::now() - start)));
}

void MainWindow::checkDisplayWindow() {
	// pas d'action à prendre
	if (currentEvent.getStatus() == core::Event::Status::Finished ||
		currentEvent.getStatus() == core::Event::Status::Invalid)
		return;
	if (displayWindow == nullptr || !displayWindow->isVisible()) {
		ui->actionStartEvent->setEnabled(true);
	}
	++autoSaveCounter;
	if (autoSaveCounter < 15)
		return;
	autoSaveCounter = 0;
	const QVariant p = settings.value("std::filesystem::path/data_path", "");
	if (p.toString() == "") {
		log_warn("Backup Save: pas de chemin vers les data");
		return;
	}
	std::filesystem::path pt(p.toString().toStdString());
	if (!exists(pt)) {
		log_warn("Backup Save: répertoire de data inexistant");
		return;
	}
	pt = pt / "rescue.lev";
	log_trace("Backup partie in {}", pt.string());
	saveFile(pt);
}

void MainWindow::updateDisplay() {
	onUpdate = true;
	// Mise à jour des menus
	updateMenus();
	// TODO: Mise à jour de la barre de statut
	// Mise à jour Zone du bas
	updateBottomFrame();
	// Mise à jour info événement
	updateInfoEvent();
	// Mise à jour info partie
	updateInfoRound();
	// Mise à jour des numéros tirés
	updateDraws();
	// Mise à jour des commandes
	updateCommands();
	// timer
	if (!timer->isActive())
		timer->start();
	onUpdate = false;
}

void MainWindow::updateMenus() const {
	// ui->menuFile			 <- toujours actif, change jamais de texte
	//   ui->actionNewEvent	 <- toujours actif, change jamais de texte
	//   ui->actionLoadEvent	<- toujours actif, change jamais de texte
	//   ui->actionSaveEvent
	if (currentEvent.getStatus() == core::Event::Status::Invalid ||
		currentEvent.getStatus() == core::Event::Status::MissingParties) {
		ui->actionSaveEvent->setEnabled(false);
		ui->actionSaveEventAs->setEnabled(false);
	} else {
		ui->actionSaveEvent->setEnabled(true);
		ui->actionSaveEventAs->setEnabled(true);
	}
	//   ui->actionStartEvent
	if (currentEvent.getStatus() == core::Event::Status::Ready)
		ui->actionStartEvent->setEnabled(true);
	else
		ui->actionStartEvent->setEnabled(false);
	//   ui->actionEndEvent
	if (currentEvent.getStatus() == core::Event::Status::Finished)
		ui->actionEndEvent->setEnabled(true);
	else
		ui->actionEndEvent->setEnabled(false);
	//   ui->actionQuit		 <- toujours actif, change jamais de texte
	// ui->menuParameters	   <- toujours actif, change jamais de texte
	//   ui->actionGlobalParam  <- toujours actif, change jamais de texte
	//   ui->actionConfigEvent  <- toujours actif, change jamais de texte
	//   ui->actionConfigCard   <- jamais actif, caché
	//   ui->actionConfigRounds
	if (currentEvent.getStatus() == core::Event::Status::Invalid)
		ui->actionConfigRounds->setEnabled(false);
	else
		ui->actionConfigRounds->setEnabled(true);
	// ui->menuHelp			 <- toujours actif, change jamais de texte
	//   ui->actionAbout		<- toujours actif, change jamais de texte
	//   ui->actionHelp		 <- toujours actif, change jamais de texte
}

void MainWindow::updateBottomFrame() {
	updateRadioButtons();
	// gestion des écrans
	if (ui->SelectScreen->count() == 0) {
		updateScreenList();
	}
	if (const bool fullScreen = settings.value("display/full_screen", true).toBool();
		fullScreen != ui->CheckFullScreen->isChecked())
		ui->CheckFullScreen->setChecked(fullScreen);
	if (const int screenId = settings.value("display/screen_id", 0).toInt();
		screenId != ui->SelectScreen->currentIndex())
		ui->SelectScreen->setCurrentIndex(screenId);
}

void MainWindow::updateScreenList() {
	ui->SelectScreen->clear();
	QList<QScreen*> screens = QApplication::screens();
	int id = 0;
	int mainId = 0;
	for (const QScreen* scr: screens) {
		if (scr == screen())
			mainId = id;
		ui->SelectScreen->addItem(QString::number(id) + " - " + scr->name());
		++id;
	}
	if (int screenId = settings.value("display/screen_id", 0).toInt(); screenId >= id || screenId == mainId) {
		if (screens.size() > 2) {
			screenId = (mainId + 1) % id;
			settings.setValue("display/screen_id", screenId);
		} else {
			settings.setValue("display/screen_id", -1);
		}
	}
}

void MainWindow::updateStats() const {
	if (currentEvent.getStatus() == core::Event::Status::Invalid ||
		currentEvent.getStatus() == core::Event::Status::MissingParties ||
		currentEvent.getStatus() == core::Event::Status::Ready)
		return;
	// pas de round en courts
	if (const auto cur = currentEvent.getCurrentCGameRound(); cur == currentEvent.endRounds())
		return;
	const auto res = currentEvent.getStats();

	ui->MostPickNb->setText(QString::number(res.mostPickNb));
	ui->MostPickList->setText(QString::fromStdString(join(res.mostPickList, " ")));
	ui->LessPickNb->setText(QString::number(res.lessPickNb));
	ui->LessPickList->setText(QString::fromStdString(join(res.lessPickList, " ")));

	ui->RoundLongest->setText(QString::fromStdString(core::formatDuration(res.roundLongest)));
	ui->RoundShortest->setText(QString::fromStdString(core::formatDuration(res.roundShortest)));
	ui->RoundAverage->setText(QString::fromStdString(core::formatDuration(res.roundAverage)));

	ui->SubRoundLongest->setText(QString::fromStdString(core::formatDuration(res.subRoundLongest)));
	ui->SubRoundShortest->setText(QString::fromStdString(core::formatDuration(res.subRoundShortest)));
	ui->SubRoundAverage->setText(QString::fromStdString(core::formatDuration(res.subRoundAverage)));

	ui->RoundMostNb->setText(QString::number(res.roundMostNb));
	ui->RoundLeastNb->setText(QString::number(res.roundLessNb));
	ui->RoundAverageNb->setText(QString::number(res.roundAverageNb));

	ui->SubRoundMostNb->setText(QString::number(res.subRoundMostNb));
	ui->SubRoundLeastNb->setText(QString::number(res.subRoundLessNb));
	ui->SubRoundAverageNb->setText(QString::number(res.subRoundAverageNb));
}

void MainWindow::updateInfoEvent() const {
	if (currentEvent.isEditable()) {
		ui->EventInfos->setEnabled(false);
		return;
	}
	ui->EventInfos->setEnabled(true);
	const core::time_point start = currentEvent.getStarting();
	const core::time_point end = currentEvent.getEnding();
	ui->CurrentDate->setText("");
	ui->StartingHour->setText("");
	ui->EndingHour->setText("");
	ui->Duration->setText("");
	ui->Progression->setRange(0, static_cast<int>(currentEvent.sizeRounds()));
	ui->Progression->setValue(currentEvent.getCurrentGameRoundIndex() + 1);
	if (start == core::g_epoch)
		return;
	ui->CurrentDate->setText(QString::fromUtf8(core::formatCalendar(start)));
	ui->StartingHour->setText(QString::fromUtf8(core::formatClockNoSecond(start)));
	if (end == core::g_epoch) {
		ui->Duration->setText(QString::fromUtf8(core::formatDuration(core::clock::now() - start)));
		return;
	}
	ui->EndingHour->setText(QString::fromUtf8(core::formatClockNoSecond(end)));
	ui->Duration->setText(QString::fromUtf8(core::formatDuration(end - start)));
}

void MainWindow::updateRadioButtons() const {
	switch (currentDrawMode) {
		case DrawMode::Both:
			ui->radioBtnPureManual->setChecked(false);
			ui->radioBtnPureRandom->setChecked(false);
			ui->radioBtnBoth->setChecked(true);
			ui->RandomPick->setEnabled(true);
			ui->actionRandomPick->setEnabled(true);
			ui->GroupPickManual->setEnabled(true);
			break;
		case DrawMode::PickOnly:
			ui->radioBtnPureManual->setChecked(false);
			ui->radioBtnPureRandom->setChecked(true);
			ui->radioBtnBoth->setChecked(false);
			ui->RandomPick->setEnabled(true);
			ui->actionRandomPick->setEnabled(true);
			ui->GroupPickManual->setEnabled(false);
			break;
		case DrawMode::ManualOnly:
			ui->radioBtnPureManual->setChecked(true);
			ui->radioBtnPureRandom->setChecked(false);
			ui->radioBtnBoth->setChecked(false);
			ui->RandomPick->setEnabled(false);
			ui->actionRandomPick->setEnabled(false);
			ui->GroupPickManual->setEnabled(true);
			break;
	}
	ui->CancelLastPick->setEnabled(true);
	ui->actionCancelLastPick->setEnabled(true);
	if (currentEvent.getStatus() != core::Event::Status::GameRunning) {
		ui->RandomPick->setEnabled(false);
		ui->actionRandomPick->setEnabled(false);
		ui->GroupPickManual->setEnabled(false);
		ui->CancelLastPick->setEnabled(false);
		ui->actionCancelLastPick->setEnabled(false);
	}
}

void MainWindow::updateInfoRound() const {
	// reset all infos
	ui->RoundStartTime->setText("");
	ui->RoundDraws->setText("");
	ui->RoundName->setText("");
	ui->RoundPhase->setText("");
	ui->RoundPhaseValue->setText("");
	ui->RoundPhasePrise->setText("");
	const auto cur = currentEvent.getCurrentCGameRound();
	if (cur == currentEvent.endRounds())
		return;
	if (cur->getStarting() == core::g_epoch)
		return;
	ui->RoundStartTime->setText(QString::fromUtf8(core::formatClock(cur->getStarting())));
	if (cur->getStarting() == core::g_epoch) {
		ui->RoundDraws->setText("0");
	} else {
		ui->RoundDraws->setText(QString::number(cur->drawsCount()));
	}
	ui->RoundName->setText(QString::number(currentEvent.getCurrentGameRoundIndex() + 1) + " - " +
						   QString::fromStdString(cur->getTypeStr()));

	if (cur->getType() == core::GameRound::Type::Pause) {
		ui->RoundPhase->setText("Pause");
	} else {
		QString phase = QString::fromStdString(cur->getStatusStr());
		if (cur->getStatus() == core::GameRound::Status::Running) {
			const auto sub = cur->getCurrentSubRound();
			phase += " - " + QString::fromUtf8(sub->getTypeStr()) + " - " + QString::fromUtf8(sub->getStatusStr());
			ui->RoundPhaseValue->setText(QString::number(sub->getValue()) + "€");
			ui->RoundPhasePrise->setText(QString::fromStdString(sub->getPrices()));
		}
		ui->RoundPhase->setText(phase);
	}
}

void MainWindow::updateDraws() const {
	if (currentEvent.getStatus() != core::Event::Status::GameRunning) {
		ui->lastNumbersDisplay->setEnabled(false);
		ui->GroupPickManual->setEnabled(false);
		ui->RandomPick->setEnabled(false);
		ui->actionRandomPick->setEnabled(false);
		return;
	}
	const auto cur = currentEvent.getCurrentCGameRound();
	const auto scur = cur->getCurrentSubRound();
	if (cur->getType() == core::GameRound::Type::Pause || cur->getStatus() != core::GameRound::Status::Running ||
		scur->getStatus() != core::SubGameRound::Status::Running) {
		ui->lastNumbersDisplay->setEnabled(false);
		ui->GroupPickManual->setEnabled(false);
		ui->RandomPick->setEnabled(false);
		ui->actionRandomPick->setEnabled(false);
		return;
	}
	ui->lastNumbersDisplay->setEnabled(true);
	ui->CurrentDraw->display(0);
	ui->LastNumber1->display(0);
	ui->LastNumber2->display(0);
	ui->LastNumber3->display(0);
	auto draws = cur->getAllDraws();
	if (!draws.empty()) {
		auto it = draws.rbegin();
		ui->CurrentDraw->display(*it);
		++it;
		if (it != draws.rend()) {
			ui->LastNumber1->display(*it);
			++it;
			if (it != draws.rend()) {
				ui->LastNumber2->display(*it);
				++it;
				if (it != draws.rend()) {
					ui->LastNumber3->display(*it);
				} else {
					ui->LastNumber3->display(0);
				}
			} else {
				ui->LastNumber2->display(0);
			}
		} else {
			ui->LastNumber1->display(0);
		}
	}
	numberGrid->resetPushed();
	for (const auto draw: draws) { numberGrid->setPushed(draw); }
	ui->textLastDraw->setText(QString::fromStdString(cur->getDrawStr()));
}

void MainWindow::updateCommands() {
	updateStartStopButton();
	ui->DisplayRules->setEnabled(false);
	ui->CancelLastPick->setEnabled(false);
	ui->actionDisplayRules->setEnabled(false);
	ui->actionCancelLastPick->setEnabled(false);
	if (currentEvent.isEditable())
		return;
	ui->DisplayRules->setEnabled(true);
	ui->actionDisplayRules->setEnabled(true);
	if (currentEvent.getStatus() == core::Event::Status::DisplayRules) {
		ui->DisplayRules->setText("Retour");
		ui->actionDisplayRules->setText("Retour");
		ui->StartEndGameRound->setEnabled(false);
		ui->actionStartEndGameRound->setEnabled(false);
	} else {
		ui->DisplayRules->setText("Affichage règlement");
		ui->actionDisplayRules->setText("Affichage règlement");
	}
	// update cancel last pick
	if (const auto cur = currentEvent.getCurrentCGameRound();
		cur != currentEvent.endRounds() && cur->getLastCancelableDraw() != 255) {
		ui->CancelLastPick->setEnabled(true);
		ui->actionCancelLastPick->setEnabled(true);
	}
}

void MainWindow::updateStartStopButton() {

	bool state = false;
	QString text = "(pas d'action)";
	switch (currentEvent.getStatus()) {
		case core::Event::Status::Invalid:
		case core::Event::Status::MissingParties:
		case core::Event::Status::Ready:
		case core::Event::Status::Finished:
		case core::Event::Status::DisplayRules:
			break;
		case core::Event::Status::EventStarting:
			state = true;
			text = "Démarrer première partie";
			break;
		case core::Event::Status::GameRunning:
			{
				state = true;
				if (const auto round = currentEvent.getCurrentCGameRound();
					round->getType() == core::GameRound::Type::Pause) {
					text = "Reprise";
				} else {
					if (round->getStatus() == core::GameRound::Status::PostScreen) {
						if (const auto nextRound = round + 1; nextRound == currentEvent.endRounds()) {
							text = "Fin de l'événement";
						} else {
							if (nextRound->getType() == core::GameRound::Type::Pause) {
								text = "Aller en pause";
							} else {
								text = "Aller à la partie suivante";
							}
						}
					} else {
						const auto subround = round->getCurrentSubRound();
						if (subround->getStatus() == core::SubGameRound::Status::PreScreen) {
							text = "Jouer la sous-partie";
						}
						if (subround->getStatus() == core::SubGameRound::Status::Running) {
							text = "Renseigner un gagnant";
						}
					}
				}
			}
			break;
		case core::Event::Status::EventEnding:
			state = true;
			text = "Clore l'événement";
			break;
	}
	ui->StartEndGameRound->setEnabled(state);
	ui->actionStartEndGameRound->setEnabled(state);
	ui->StartEndGameRound->setText(text);
	ui->actionStartEndGameRound->setText(text);
}

void MainWindow::actGridPushed(const int value) {
	if (onUpdate)
		return;
	currentEvent.getCurrentGameRound()->addPickedNumber(static_cast<uint8_t>(value));
	rng.addPick(static_cast<uint8_t>(value));
	updateDisplay();
}

}// namespace evl::gui
