/**
 * @file test_WidgetNumberGrid.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/WidgetNumberGrid.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_WidgetNumberGrid, base) {
    test::initialize();
    WidgetNumberGrid w{};
    test::finalize();
}

TEST(gui_WidgetNumberGrid, push_all_button) {
    test::initialize();
    WidgetNumberGrid w{};
    for(uint8_t i= 1; i <= 91; ++i) w.setPushed(i);
    EXPECT_EQ(w.getNumberPushed(), 90);
    w.resetPushed();
    EXPECT_EQ(w.getNumberPushed(), 0);
    test::finalize();
}

TEST(gui_WidgetNumberGrid, call_all_button) {
    test::initialize();
    WidgetNumberGrid w{};
    w.buttonPushed(1);
    w.slot1();
    w.slot2();
    w.slot3();
    w.slot4();
    w.slot5();
    w.slot6();
    w.slot7();
    w.slot8();
    w.slot9();
    w.slot10();
    w.slot11();
    w.slot12();
    w.slot13();
    w.slot14();
    w.slot15();
    w.slot16();
    w.slot17();
    w.slot18();
    w.slot19();
    w.slot20();
    w.slot21();
    w.slot22();
    w.slot23();
    w.slot24();
    w.slot25();
    w.slot26();
    w.slot27();
    w.slot28();
    w.slot29();
    w.slot30();
    w.slot31();
    w.slot32();
    w.slot33();
    w.slot34();
    w.slot35();
    w.slot36();
    w.slot37();
    w.slot38();
    w.slot39();
    w.slot40();
    w.slot41();
    w.slot42();
    w.slot43();
    w.slot44();
    w.slot45();
    w.slot46();
    w.slot47();
    w.slot48();
    w.slot49();
    w.slot50();
    w.slot51();
    w.slot52();
    w.slot53();
    w.slot54();
    w.slot55();
    w.slot56();
    w.slot57();
    w.slot58();
    w.slot59();
    w.slot60();
    w.slot61();
    w.slot62();
    w.slot63();
    w.slot64();
    w.slot65();
    w.slot66();
    w.slot67();
    w.slot68();
    w.slot69();
    w.slot70();
    w.slot71();
    w.slot72();
    w.slot73();
    w.slot74();
    w.slot75();
    w.slot76();
    w.slot77();
    w.slot78();
    w.slot79();
    w.slot80();
    w.slot81();
    w.slot82();
    w.slot83();
    w.slot84();
    w.slot85();
    w.slot86();
    w.slot87();
    w.slot88();
    w.slot89();
    w.slot90();
    EXPECT_EQ(w.getNumberPushed(), 90);
    test::finalize();
}
