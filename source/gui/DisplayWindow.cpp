/**
* @file DisplayWindow.cpp
* @author Silmaen
* @date 18/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "DisplayWindow.h"
#include "MainWindow.h"
#include "baseDefinitions.h"
#include <spdlog/spdlog.h>

// Les trucs de QT
#include <moc_DisplayWindow.cpp>
#include <ui/ui_DisplayWindow.h>

namespace evl::gui {

const std::unordered_map<DisplayWindow::Page, int> DisplayWindow::PageIndex= {
        {Page::MainTitle, 0},
        {Page::PricesDisplay, 1},
        {Page::NumberDisplay, 2},
        {Page::EventStarting, 7},
        {Page::PauseDisplay, 4},
        {Page::RulesDisplay, 6},
        {Page::EventEnding, 5},
        {Page::RoundEnding, 3},
};

DisplayWindow::DisplayWindow(core::Event* evt, MainWindow* parent):
    QMainWindow(parent),
    ui(new Ui::DisplayWindow), timer(new QTimer(this)), event(evt), mwd(parent) {
    ui->setupUi(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&DisplayWindow::updateDisplay));
    if(event != nullptr) {
        timer->setInterval(200);
        timer->start();
        initializeNumberGrid();
        resize();
    }
}

DisplayWindow::~DisplayWindow() {
    delete ui;
    delete timer;
}

void setPixMap(QLabel* where, const QString& name, const QImage& img) {
    QString realName= name + "_" + QString::number(where->width()) + "_" + QString::number(where->height());
    if(where->text() != realName) {// charge l’image seulement une fois
        where->setText(realName);
        where->setPixmap(QPixmap::fromImage(img.scaled(where->size(), Qt::KeepAspectRatio)));
    }
}

void DisplayWindow::initializeDisplay() {
    //  les images
    if(event->getOrganizerLogoFull().empty()) {
        ui->ET_OrganizerLogo->setVisible(false);
        ui->RR_Logo->setVisible(false);
        ui->RT_Logo->setVisible(false);
        ui->RE_Logo->setVisible(false);
        ui->EP_LogoA->setVisible(false);
        ui->EP_LogoB->setVisible(false);
        ui->EE_LogoA->setVisible(false);
        ui->EE_LogoB->setVisible(false);
        ui->ER_LogoA->setVisible(false);
        ui->ER_LogoB->setVisible(false);
        ui->SR_LogoA->setVisible(false);
        ui->SR_LogoB->setVisible(false);
    } else {
        if(!exists(event->getOrganizerLogoFull())) {
            ui->ET_OrganizerLogo->setVisible(false);
            ui->RR_Logo->setVisible(false);
            ui->RT_Logo->setVisible(false);
            ui->RE_Logo->setVisible(false);
            ui->EP_LogoA->setVisible(false);
            ui->EP_LogoB->setVisible(false);
            ui->EE_LogoA->setVisible(false);
            ui->EE_LogoB->setVisible(false);
            ui->ER_LogoA->setVisible(false);
            ui->ER_LogoB->setVisible(false);
            ui->SR_LogoA->setVisible(false);
            ui->SR_LogoB->setVisible(false);
        } else {
            QImage img  = loadImage(event->getOrganizerLogoFull());
            QString name= QString::fromUtf8(event->getOrganizerLogoFull().string());
            setPixMap(ui->ET_OrganizerLogo, name, img);
            setPixMap(ui->RR_Logo, name, img);
            setPixMap(ui->RT_Logo, name, img);
            setPixMap(ui->RE_Logo, name, img);
            setPixMap(ui->EP_LogoA, name, img);
            setPixMap(ui->EP_LogoB, name, img);
            setPixMap(ui->EE_LogoA, name, img);
            setPixMap(ui->EE_LogoB, name, img);
            setPixMap(ui->ER_LogoA, name, img);
            setPixMap(ui->ER_LogoB, name, img);
            setPixMap(ui->SR_LogoA, name, img);
            setPixMap(ui->SR_LogoB, name, img);
        }
    }
    if(event->getLogoFull().empty()) {
        ui->ET_EventLogo->setVisible(false);
    } else {
        if(!exists(event->getLogoFull())) {
            ui->ET_EventLogo->setVisible(false);
        } else {
            QImage img= loadImage(event->getLogoFull());
            setPixMap(ui->ET_EventLogo, QString::fromUtf8(event->getLogoFull().string()), img);
        }
    }
}

void DisplayWindow::setPage(const Page& newPage) {
    if(PageIndex.contains(newPage)) {
        if(ui->PageManager->currentIndex() != PageIndex.at(newPage)) {
            ui->PageManager->setCurrentIndex(PageIndex.at(newPage));
            resize();
        }
    }
}

void DisplayWindow::updateDisplay() {
    if(event == nullptr)
        return;
    if(mode == Mode::Preview) {
        auto cur= event->getGameRound(roundIndex);
        if(cur->getType() == core::GameRound::Type::Pause) {
            setPage(Page::PauseDisplay);
            updatePauseScreen();
        } else {
            setPage(Page::PricesDisplay);
            updateRoundTitlePage();
        }
        // action de redimensionnement
        updateColors();
        resize();
        return;
    }
    switch(event->getStatus()) {
    case core::Event::Status::Invalid:
    case core::Event::Status::MissingParties:
    case core::Event::Status::Ready:
    case core::Event::Status::EventStarting:
        setPage(Page::MainTitle);
        updateEventTitlePage();
        break;
    case core::Event::Status::GameRunning: {
        auto cur= event->getCurrentCGameRound();
        if(cur->getType() == core::GameRound::Type::Pause) {
            // round special type
            setPage(Page::PauseDisplay);
            updatePauseScreen();
            break;
        }
        if(cur->getStatus() == core::GameRound::Status::PostScreen) {
            // round special type
            setPage(Page::RoundEnding);
            updateRoundEndingPage();
            break;
        }
        if(cur->getStatus() == core::GameRound::Status::Running) {
            auto subcur= cur->getCurrentSubRound();
            auto status= subcur->getStatus();
            if(status == core::SubGameRound::Status::PreScreen) {
                setPage(Page::PricesDisplay);
                updateRoundTitlePage();
                break;
            }
            if(status == core::SubGameRound::Status::Running) {
                setPage(Page::NumberDisplay);
                updateRoundRunning();
                break;
            }
        }
        break;
    }
    case core::Event::Status::DisplayRules:
        setPage(Page::RulesDisplay);
        updateDisplayRules();
        break;
    case core::Event::Status::EventEnding:
    case core::Event::Status::Finished:
        setPage(Page::EventEnding);
        break;
    }
    // action de redimensionnement
    updateColors();
    if(currentSize != size() || event->checkStateChanged() || mwd->getTheme().isModified())
        resize();
}

void DisplayWindow::updateEventTitlePage() {
    ui->ET_EventDate->setText(QString::fromUtf8(core::formatCalendar(event->getStarting())));
    ui->ET_EventTitle->setText(QString::fromUtf8(event->getName()));
    ui->ET_OrganizerName->setText(QString::fromUtf8(event->getOrganizerName()));
    if(event->getLocation().empty()) {
        ui->ET_EventLocation->setVisible(false);
    } else {
        ui->ET_EventLocation->setVisible(true);
        ui->ET_EventLocation->setText(QString::fromUtf8(event->getLocation()));
    }
}

void DisplayWindow::updateRoundTitlePage() {
    core::Event::roundsType::const_iterator round;
    core::GameRound::subRoundsType::const_iterator subRound;
    if(mode == Mode::Preview) {
        round   = event->getGameRound(roundIndex);
        subRound= round->getSubRound(subRoundIndex);
    } else {
        round   = event->getCurrentCGameRound();
        subRound= round->getCurrentSubRound();
    }
    ui->RT_RoundTitle->setText(QString::fromUtf8(round->getName()) + " - " + QString::fromUtf8(subRound->getTypeStr()));
    ui->RT_SubRound->setVisible(false);
    ui->RT_Valeur->setVisible(false);
    ui->RT_ValeurLbl->setVisible(false);
    if(subRound->getPrices().empty())
        return;
    ui->RT_SubRound->setVisible(true);
    ui->RT_SubRound->setText(QString::fromUtf8(subRound->getPrices()));
    if(subRound->getValue() > 0) {
        ui->RT_Valeur->setVisible(true);
        ui->RT_ValeurLbl->setVisible(true);
        ui->RT_Valeur->setText(QString::number(subRound->getValue()) + " €");
    }
}

void DisplayWindow::updateRoundRunning() {
    auto round   = event->getCurrentCGameRound();
    auto subRound= round->getCurrentSubRound();
    ui->RR_Title->setText(QString::fromUtf8(round->getName()) + " - " + QString::fromStdString(subRound->getTypeStr()));
    // mise à jour de l’heure et durée
    auto now= core::clock::now();
    ui->RR_Clock->setText(QString::fromStdString(core::formatClockNoSecond(now)));
    auto duration= now - round->getStarting();
    ui->RR_Duration->setText(QString::fromStdString(core::formatDuration(duration)));
    // mise à jour de l’affichage de la grille
    resetGrid();
    QBrush br;
    br.setStyle(Qt::BrushStyle::SolidPattern);
    int i       = 0;
    QColor color= QColor(mwd->getTheme().getParam("selectedNumberColor").toString());
    auto text   = QColor(mwd->getTheme().getParam("textColor").toString());
    text.setRedF(1.0f - text.redF());
    text.setGreenF(1.0f - text.greenF());
    text.setBlueF(1.0f - text.blueF());
    QBrush fr{text};
    const auto draws= round->getAllDraws();
    for(auto draw= draws.rbegin(); draw != draws.rend(); ++draw) {
        br.setColor(color);
        if(mwd->getTheme().getParam("fadeNumbers").toBool() &&
           i < mwd->getTheme().getParam("fadeNumbersAmount").toInt()) {
            int strength= mwd->getTheme().getParam("fadeNumbersStrength").toInt();
            if(strength < 0) {
                color= color.darker(100 - strength);
            } else {
                color= color.lighter(100 + strength);
            }
        }
        ++i;
        int row= (*draw - 1) / 10;
        int col= (*draw - 1) % 10;
        ui->RR_NumberGrid->item(row, col)->setBackground(br);
        ui->RR_NumberGrid->item(row, col)->setForeground(fr);
    }
    // mise à jour de l’affichage des derniers numéros
    ui->RR_CurrentDraw->display(0);
    auto it= draws.rbegin();
    if(it != draws.rend()) {
        ui->RR_CurrentDraw->display(*it);
    }
    // mise à jour de l'affichage des lots et type de partie

    if(subRound->getValue() < 0.01) {
        ui->RR_SubRoundValue->setVisible(false);
    } else {
        QString value= "Valeur du lot: \n" + QString::number(subRound->getValue()) + "€";
        ui->RR_SubRoundValue->setVisible(true);
        ui->RR_SubRoundValue->setText(value);
    }
    QString gain= QString::fromUtf8(subRound->getPrices());
    if(gain.isEmpty()) {
        ui->RR_SubRoundPrice->setVisible(false);
    } else {
        ui->RR_SubRoundPrice->setVisible(true);
        if(mwd->getTheme().getParam("truncatePrice").toBool()) {
            auto gains = gain.split("\n");
            int maxLine= mwd->getTheme().getParam("truncatePriceLines").toInt();
            if(gains.size() <= maxLine) {
                ui->RR_SubRoundPrice->setText(gain);
            } else {
                QString s= gains[0];
                for(int iLine= 1; iLine < maxLine; ++iLine) s+= "\n" + gains[iLine];
                ui->RR_SubRoundPrice->setText(s);
            }
        } else {
            ui->RR_SubRoundPrice->setText(gain);
        }
    }
}

void DisplayWindow::updateDisplayRules() {
    if(event->getRules().empty())
        return;
    ui->ER_Rules->setText(QString::fromUtf8(event->getRules()));
}

void DisplayWindow::updatePauseScreen() {
    core::Event::roundsType::const_iterator round;
    if(mode == Mode::Preview)
        round= event->getGameRound(roundIndex);
    else
        round= event->getCurrentCGameRound();
    auto [dPath, dDelay]= round->getDiapo();
    if(dPath.empty() || dDelay <= 0 || !exists(dPath)) {
        QImage img  = loadImage(event->getOrganizerLogoFull());
        QString name= QString::fromUtf8(event->getOrganizerLogoFull().string());
        setPixMap(ui->EP_Text, name, img);
        return;
    }
    std::vector<path> slides;
    const std::vector<path> extension{".png", ".jpg", ".jpeg", ".svg", ".PNG", ".JPG", ".JPEG", ".SVG"};
    for(const auto& ipath: std::filesystem::directory_iterator(dPath)) {
        if(std::find(extension.begin(), extension.end(), ipath.path().extension()) == extension.end())
            continue;
        slides.push_back(ipath);
    }
    if(slides.empty()) {
        QImage img  = loadImage(event->getOrganizerLogoFull());
        QString name= QString::fromUtf8(event->getOrganizerLogoFull().string());
        setPixMap(ui->EP_Text, name, img);
        return;
    }
    static size_t curSlide   = 0;
    static core::timePoint tp= core::epoch;
    if(static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(core::clock::now() - tp).count()) / 1000.0 < dDelay)
        return;
    tp        = core::clock::now();
    curSlide  = (curSlide + 1) % slides.size();
    auto slide= loadImage(slides[curSlide]);
    setPixMap(ui->EP_Text, QString::fromUtf8(slides[curSlide].string()), slide);
}

void DisplayWindow::updateRoundEndingPage() {
}

void DisplayWindow::updateColors() {
    if(mwd == nullptr)
        return;
    auto curent= palette();
    auto back  = QColor(mwd->getTheme().getParam("backgroundColor").toString());
    curent.setColor(QPalette::ColorRole::Window, back);
    auto text= QColor(mwd->getTheme().getParam("textColor").toString());
    curent.setColor(QPalette::ColorRole::Text, text);
    curent.setColor(QPalette::ColorRole::WindowText, text);
    curent.setColor(QPalette::ColorRole::PlaceholderText, text);
    auto gridBack= QColor(mwd->getTheme().getParam("gridBackgroundColor").toString());
    curent.setColor(QPalette::ColorRole::Base, gridBack);
    setPalette(curent);
}

void DisplayWindow::initializeNumberGrid() {
    ui->RR_NumberGrid->setColumnCount(10);
    ui->RR_NumberGrid->setRowCount(9);
    int px   = std::min(ui->RR_NumberGrid->width() / 10, ui->RR_NumberGrid->height() / 9) / 3;
    auto font= QFont("Segoe UI", px);
    font.setBold(true);
    for(int row= 0; row < 9; ++row) {
        for(int col= 0; col < 10; ++col) {
            QTableWidgetItem* pCell= ui->RR_NumberGrid->item(row, col);
            if(!pCell) {
                pCell= new QTableWidgetItem;
                ui->RR_NumberGrid->setItem(row, col, pCell);
            }
            pCell->setFlags(Qt::ItemFlag::ItemIsEnabled);
            pCell->setText(QString::number(10 * row + col + 1));
            pCell->setTextAlignment(Qt::AlignCenter);
            pCell->setFont(font);
        }
    }
}

void DisplayWindow::resetGrid() {
    auto theme= mwd->getTheme();
    QBrush br;
    br.setColor(QColor(0, 0, 0, 0));
    br.setStyle(Qt::BrushStyle::SolidPattern);
    auto text= QColor(theme.getParam("textColor").toString());
    QBrush fr{text};
    for(int row= 0; row < 9; ++row) {
        for(int col= 0; col < 10; ++col) {
            ui->RR_NumberGrid->item(row, col)->setBackground(br);
            ui->RR_NumberGrid->item(row, col)->setForeground(fr);
        }
    }
    // taille des lignes et colonnes de la grille
    ui->RR_NumberGrid->horizontalHeader()->setDefaultSectionSize(ui->RR_NumberGrid->width() / 10);
    ui->RR_NumberGrid->verticalHeader()->setDefaultSectionSize(ui->RR_NumberGrid->height() / 9);
    // taille de police dans la grille
    double setting_ratio= theme.getParam("gridTextRatio").toDouble();
    auto baseFont       = font();
    double baseRatio    = std::min(width() * 1.0, height() * 1.4) * theme.getParam("baseRatio").toDouble();
    double gridRatio    = baseRatio * setting_ratio;
    baseFont.setPointSizeF(gridRatio);
    baseFont.setBold(true);
    for(int row= 0; row < 9; ++row) {
        for(int col= 0; col < 10; ++col) {
            QTableWidgetItem* pCell= ui->RR_NumberGrid->item(row, col);
            if(pCell) {
                pCell->setFont(baseFont);
            }
        }
    }
}

void DisplayWindow::resize() {
    initializeDisplay();
    // taille de la font par défaut
    if(mwd == nullptr) {
        spdlog::warn("No mainwindows link for Resize!!");
        return;
    }

    ui->ET_OrganizerLogo->setText("");
    ui->RR_Logo->setText("");
    ui->RT_Logo->setText("");
    ui->RE_Logo->setText("");
    ui->EP_LogoA->setText("");
    ui->EP_LogoB->setText("");

    ui->EE_LogoA->setText("");
    ui->EE_LogoB->setText("");

    ui->ER_LogoA->setText("");
    ui->ER_LogoB->setText("");

    ui->SR_LogoA->setText("");
    ui->SR_LogoB->setText("");

    auto theme       = mwd->getTheme();
    auto baseFont    = font();
    auto baseFontBold= font();
    baseFontBold.setBold(true);
    double setting_ratio= theme.getParam("baseRatio").toDouble();
    double baseRatio    = std::min(width() * 1.0, height() * 1.4) * setting_ratio;
    baseFont.setPointSizeF(baseRatio);
    baseFontBold.setPointSizeF(baseRatio);
    setFont(baseFont);

    ui->RR_SubRoundPrice->setFont(baseFont);
    ui->RT_SubRound->setFont(baseFont);
    // reduce size if too many lines
    QString text= ui->RR_SubRoundPrice->text();
    auto nbLine = text.split("\n").size();
    if(nbLine > 4) {
        double pricesRatio= 4.0 / static_cast<double>(nbLine) * baseRatio;
        baseFont.setPointSizeF(pricesRatio);
        ui->RR_SubRoundPrice->setFont(baseFont);
    }

    // taille font des titres
    setting_ratio    = theme.getParam("titleRatio").toDouble();
    double titleRatio= baseRatio * setting_ratio;
    baseFont.setPointSizeF(titleRatio);
    ui->ET_EventTitle->setFont(baseFont);
    ui->RT_RoundTitle->setFont(baseFont);
    ui->RR_Title->setFont(baseFont);
    ui->RE_RoundEndTitle->setFont(baseFont);
    ui->EP_Title->setFont(baseFont);
    ui->EE_Title->setFont(baseFont);
    ui->ER_Title->setFont(baseFont);
    // taille de textes longs
    setting_ratio       = theme.getParam("longTextRatio").toDouble();
    double longTextRatio= baseRatio * setting_ratio;
    baseFont.setPointSizeF(longTextRatio);
    ui->ER_Rules->setFont(baseFont);
    // Taille textes courts
    setting_ratio        = theme.getParam("shortTextRatio").toDouble();
    double shortTextRatio= baseRatio * setting_ratio;
    baseFont.setPointSizeF(shortTextRatio);
    ui->EP_Message->setFont(baseFont);
    ui->EE_EndMessage->setFont(baseFont);

    //
    ui->RT_ValeurLbl->setFont(baseFontBold);
    ui->RT_Valeur->setFont(baseFontBold);
    ui->RR_SubRoundValue->setFont(baseFontBold);
    //
    resetGrid();
    //
    currentSize= size();
}

}// namespace evl::gui
