/**
 * @file WidgetNumberGrid.cpp
 * @author Silmaen
 * @date 25/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/WidgetNumberGrid.h"

// Les trucs de QT
#include "gui/moc_WidgetNumberGrid.cpp"
#include "ui/ui_WidgetNumberGrid.h"

namespace evl::gui {

WidgetNumberGrid::WidgetNumberGrid(QWidget* parent):
    QWidget(parent),
    ui(new Ui::WidgetNumberGrid) {
    ui->setupUi(this);
}
WidgetNumberGrid::~WidgetNumberGrid() {
    delete ui;
}

void WidgetNumberGrid::slot1() {
    setValue(1);
}
void WidgetNumberGrid::slot2() {
    setValue(2);
}
void WidgetNumberGrid::slot3() {
    setValue(3);
}
void WidgetNumberGrid::slot4() {
    setValue(4);
}
void WidgetNumberGrid::slot5() {
    setValue(5);
}
void WidgetNumberGrid::slot6() {
    setValue(6);
}
void WidgetNumberGrid::slot7() {
    setValue(7);
}
void WidgetNumberGrid::slot8() {
    setValue(8);
}
void WidgetNumberGrid::slot9() {
    setValue(9);
}
void WidgetNumberGrid::slot10() {
    setValue(10);
}
void WidgetNumberGrid::slot11() {
    setValue(11);
}
void WidgetNumberGrid::slot12() {
    setValue(12);
}
void WidgetNumberGrid::slot13() {
    setValue(13);
}
void WidgetNumberGrid::slot14() {
    setValue(14);
}
void WidgetNumberGrid::slot15() {
    setValue(15);
}
void WidgetNumberGrid::slot16() {
    setValue(16);
}
void WidgetNumberGrid::slot17() {
    setValue(17);
}
void WidgetNumberGrid::slot18() {
    setValue(18);
}
void WidgetNumberGrid::slot19() {
    setValue(19);
}
void WidgetNumberGrid::slot20() {
    setValue(20);
}
void WidgetNumberGrid::slot21() {
    setValue(21);
}
void WidgetNumberGrid::slot22() {
    setValue(22);
}
void WidgetNumberGrid::slot23() {
    setValue(23);
}
void WidgetNumberGrid::slot24() {
    setValue(24);
}
void WidgetNumberGrid::slot25() {
    setValue(25);
}
void WidgetNumberGrid::slot26() {
    setValue(26);
}
void WidgetNumberGrid::slot27() {
    setValue(27);
}
void WidgetNumberGrid::slot28() {
    setValue(28);
}
void WidgetNumberGrid::slot29() {
    setValue(29);
}
void WidgetNumberGrid::slot30() {
    setValue(30);
}
void WidgetNumberGrid::slot31() {
    setValue(31);
}
void WidgetNumberGrid::slot32() {
    setValue(32);
}
void WidgetNumberGrid::slot33() {
    setValue(33);
}
void WidgetNumberGrid::slot34() {
    setValue(34);
}
void WidgetNumberGrid::slot35() {
    setValue(35);
}
void WidgetNumberGrid::slot36() {
    setValue(36);
}
void WidgetNumberGrid::slot37() {
    setValue(37);
}
void WidgetNumberGrid::slot38() {
    setValue(38);
}
void WidgetNumberGrid::slot39() {
    setValue(39);
}
void WidgetNumberGrid::slot40() {
    setValue(40);
}
void WidgetNumberGrid::slot41() {
    setValue(41);
}
void WidgetNumberGrid::slot42() {
    setValue(42);
}
void WidgetNumberGrid::slot43() {
    setValue(43);
}
void WidgetNumberGrid::slot44() {
    setValue(44);
}
void WidgetNumberGrid::slot45() {
    setValue(45);
}
void WidgetNumberGrid::slot46() {
    setValue(46);
}
void WidgetNumberGrid::slot47() {
    setValue(47);
}
void WidgetNumberGrid::slot48() {
    setValue(48);
}
void WidgetNumberGrid::slot49() {
    setValue(49);
}
void WidgetNumberGrid::slot50() {
    setValue(50);
}
void WidgetNumberGrid::slot51() {
    setValue(51);
}
void WidgetNumberGrid::slot52() {
    setValue(52);
}
void WidgetNumberGrid::slot53() {
    setValue(53);
}
void WidgetNumberGrid::slot54() {
    setValue(54);
}
void WidgetNumberGrid::slot55() {
    setValue(55);
}
void WidgetNumberGrid::slot56() {
    setValue(56);
}
void WidgetNumberGrid::slot57() {
    setValue(57);
}
void WidgetNumberGrid::slot58() {
    setValue(58);
}
void WidgetNumberGrid::slot59() {
    setValue(59);
}
void WidgetNumberGrid::slot60() {
    setValue(60);
}
void WidgetNumberGrid::slot61() {
    setValue(61);
}
void WidgetNumberGrid::slot62() {
    setValue(62);
}
void WidgetNumberGrid::slot63() {
    setValue(63);
}
void WidgetNumberGrid::slot64() {
    setValue(64);
}
void WidgetNumberGrid::slot65() {
    setValue(65);
}
void WidgetNumberGrid::slot66() {
    setValue(66);
}
void WidgetNumberGrid::slot67() {
    setValue(67);
}
void WidgetNumberGrid::slot68() {
    setValue(68);
}
void WidgetNumberGrid::slot69() {
    setValue(69);
}
void WidgetNumberGrid::slot70() {
    setValue(70);
}
void WidgetNumberGrid::slot71() {
    setValue(71);
}
void WidgetNumberGrid::slot72() {
    setValue(72);
}
void WidgetNumberGrid::slot73() {
    setValue(73);
}
void WidgetNumberGrid::slot74() {
    setValue(74);
}
void WidgetNumberGrid::slot75() {
    setValue(75);
}
void WidgetNumberGrid::slot76() {
    setValue(76);
}
void WidgetNumberGrid::slot77() {
    setValue(77);
}
void WidgetNumberGrid::slot78() {
    setValue(78);
}
void WidgetNumberGrid::slot79() {
    setValue(79);
}
void WidgetNumberGrid::slot80() {
    setValue(80);
}
void WidgetNumberGrid::slot81() {
    setValue(81);
}
void WidgetNumberGrid::slot82() {
    setValue(82);
}
void WidgetNumberGrid::slot83() {
    setValue(83);
}
void WidgetNumberGrid::slot84() {
    setValue(84);
}
void WidgetNumberGrid::slot85() {
    setValue(85);
}
void WidgetNumberGrid::slot86() {
    setValue(86);
}
void WidgetNumberGrid::slot87() {
    setValue(87);
}
void WidgetNumberGrid::slot88() {
    setValue(88);
}
void WidgetNumberGrid::slot89() {
    setValue(89);
}
void WidgetNumberGrid::slot90() {
    setValue(90);
}

void WidgetNumberGrid::setValue(int value) {
    setPushed(value);
    emit buttonPushed(value);
}

void buttonPushed(int) {}

QPushButton* WidgetNumberGrid::getButton(int idx) {
    switch(idx) {
    case 1: return ui->pushButton_01;
    case 2: return ui->pushButton_02;
    case 3: return ui->pushButton_03;
    case 4: return ui->pushButton_04;
    case 5: return ui->pushButton_05;
    case 6: return ui->pushButton_06;
    case 7: return ui->pushButton_07;
    case 8: return ui->pushButton_08;
    case 9: return ui->pushButton_09;
    case 10: return ui->pushButton_10;
    case 11: return ui->pushButton_11;
    case 12: return ui->pushButton_12;
    case 13: return ui->pushButton_13;
    case 14: return ui->pushButton_14;
    case 15: return ui->pushButton_15;
    case 16: return ui->pushButton_16;
    case 17: return ui->pushButton_17;
    case 18: return ui->pushButton_18;
    case 19: return ui->pushButton_19;
    case 20: return ui->pushButton_20;
    case 21: return ui->pushButton_21;
    case 22: return ui->pushButton_22;
    case 23: return ui->pushButton_23;
    case 24: return ui->pushButton_24;
    case 25: return ui->pushButton_25;
    case 26: return ui->pushButton_26;
    case 27: return ui->pushButton_27;
    case 28: return ui->pushButton_28;
    case 29: return ui->pushButton_29;
    case 30: return ui->pushButton_30;
    case 31: return ui->pushButton_31;
    case 32: return ui->pushButton_32;
    case 33: return ui->pushButton_33;
    case 34: return ui->pushButton_34;
    case 35: return ui->pushButton_35;
    case 36: return ui->pushButton_36;
    case 37: return ui->pushButton_37;
    case 38: return ui->pushButton_38;
    case 39: return ui->pushButton_39;
    case 40: return ui->pushButton_40;
    case 41: return ui->pushButton_41;
    case 42: return ui->pushButton_42;
    case 43: return ui->pushButton_43;
    case 44: return ui->pushButton_44;
    case 45: return ui->pushButton_45;
    case 46: return ui->pushButton_46;
    case 47: return ui->pushButton_47;
    case 48: return ui->pushButton_48;
    case 49: return ui->pushButton_49;
    case 50: return ui->pushButton_50;
    case 51: return ui->pushButton_51;
    case 52: return ui->pushButton_52;
    case 53: return ui->pushButton_53;
    case 54: return ui->pushButton_54;
    case 55: return ui->pushButton_55;
    case 56: return ui->pushButton_56;
    case 57: return ui->pushButton_57;
    case 58: return ui->pushButton_58;
    case 59: return ui->pushButton_59;
    case 60: return ui->pushButton_60;
    case 61: return ui->pushButton_61;
    case 62: return ui->pushButton_62;
    case 63: return ui->pushButton_63;
    case 64: return ui->pushButton_64;
    case 65: return ui->pushButton_65;
    case 66: return ui->pushButton_66;
    case 67: return ui->pushButton_67;
    case 68: return ui->pushButton_68;
    case 69: return ui->pushButton_69;
    case 70: return ui->pushButton_70;
    case 71: return ui->pushButton_71;
    case 72: return ui->pushButton_72;
    case 73: return ui->pushButton_73;
    case 74: return ui->pushButton_74;
    case 75: return ui->pushButton_75;
    case 76: return ui->pushButton_76;
    case 77: return ui->pushButton_77;
    case 78: return ui->pushButton_78;
    case 79: return ui->pushButton_79;
    case 80: return ui->pushButton_80;
    case 81: return ui->pushButton_81;
    case 82: return ui->pushButton_82;
    case 83: return ui->pushButton_83;
    case 84: return ui->pushButton_84;
    case 85: return ui->pushButton_85;
    case 86: return ui->pushButton_86;
    case 87: return ui->pushButton_87;
    case 88: return ui->pushButton_88;
    case 89: return ui->pushButton_89;
    case 90: return ui->pushButton_90;
    }
    return nullptr;
}

void WidgetNumberGrid::setPushed(int index) {
    QPushButton* a= getButton(index);
    if(a == nullptr)
        return;
    a->setEnabled(false);
    a->setChecked(true);
    a->setFlat(true);
}

void WidgetNumberGrid::resetPushed(int idx) {
    QPushButton* a= getButton(idx);
    a->setChecked(false);
    a->setEnabled(true);
    a->setFlat(false);
}

void WidgetNumberGrid::resetPushed() {
    for(uint8_t i= 1; i <= 90; ++i) {
        resetPushed(i);
    }
}

}// namespace evl::gui
