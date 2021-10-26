/**
 * @file WidgetNumberGrid.h
 * @author Silmaen
 * @date 25/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include <QPushButton>
#include <QWidget>

namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class WidgetNumberGrid;

}// namespace Ui

namespace evl::gui {

/**
 * @brief Classe D’affichage de la boite de dialogue d’à propos.
 */
class WidgetNumberGrid: public QWidget {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit WidgetNumberGrid(QWidget* parent= nullptr);

    /**
     * @brief Destructeur.
     */
    ~WidgetNumberGrid() override;

    /**
     * @brief
     * @param index
     */
    void setPushed(int index);
    /**
     * @brief
     * @param index
     */
    void resetPushed(int index);
    /**
     * @brief
     */
    void resetPushed();

    /**
     * @brief Compte le nombre de boutons poussés
     * @return Le nombre.
     */
    uint8_t getNumberPushed();

#ifdef EVL_DEBUG
public slots:
#else
private slots:
#endif

    /**
     * @brief Fonction d’appui bouton.
     */
    void slot1();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot2();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot3();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot4();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot5();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot6();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot7();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot8();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot9();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot10();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot11();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot12();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot13();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot14();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot15();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot16();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot17();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot18();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot19();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot20();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot21();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot22();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot23();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot24();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot25();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot26();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot27();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot28();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot29();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot30();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot31();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot32();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot33();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot34();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot35();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot36();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot37();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot38();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot39();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot40();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot41();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot42();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot43();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot44();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot45();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot46();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot47();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot48();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot49();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot50();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot51();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot52();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot53();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot54();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot55();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot56();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot57();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot58();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot59();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot60();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot61();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot62();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot63();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot64();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot65();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot66();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot67();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot68();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot69();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot70();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot71();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot72();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot73();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot74();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot75();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot76();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot77();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot78();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot79();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot80();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot81();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot82();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot83();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot84();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot85();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot86();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot87();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot88();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot89();
    /**
     * @brief Fonction d’appui bouton.
     */
    void slot90();

signals:
    /**
     * @brief
     * @param Value
     */
    void buttonPushed(int Value);

private:
    /**
     * @brief
     * @param value
     */
    void setValue(int value);
    /**
     * @brief
     * @param idx
     * @return
     */
    QPushButton* getButton(int idx);
    /// Lien vers la page UI.
    Ui::WidgetNumberGrid* ui;
};

}// namespace evl::gui
