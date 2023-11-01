/**
 * @file test_VisualTheme.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "external/json.h"
#include "gui/VisualTheme.h"
#include "test_GuiHelpers.h"
#include <fstream>
#include <gtest/gtest.h>

namespace fs= std::filesystem;
using json  = nlohmann::json;
using namespace evl::gui;

TEST(gui_VisualTheme, base) {
    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path fileSettings= tmp / "settings.ini";

    QSettings settings{QString::fromUtf8(fileSettings.string()), QSettings::IniFormat};
    settings.setValue("theme/toto", 153);
    settings.setValue("theme/bob", "sert à rien");
    settings.setValue("titi/toto", true);
    settings.setValue("theme/name", 153);
    settings.setValue("theme/titleRatio", "sert à rien");
    settings.setValue("titi/longTextRatio", true);
    settings.setValue("theme/gridTextRatio", -666.66);
    settings.sync();
    VisualTheme theme(&settings);

    EXPECT_TRUE(theme.isModified());
    EXPECT_EQ(theme.getParam("param de merde"), QVariant());
    EXPECT_STREQ(theme.getParam("name").value<QString>().toStdString().c_str(), "153");
    EXPECT_EQ(theme.getParam("gridTextRatio").value<double>(), -666.66);
    fs::remove_all(tmp);
}

TEST(gui_VisualTheme, base2) {
    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path fileSettings= tmp / "settings.ini";
    QSettings settings{QString::fromUtf8(fileSettings.string()), QSettings::IniFormat};
    VisualTheme theme(&settings);
    settings.sync();
    theme.setParam("titleRatio", 25.0);
    theme.setParam("totocaca", 25.0);           // mauvais nom
    theme.setParam("titleRatio", "juste pouri");// mauvais type
    theme.writeInSettings();

    EXPECT_EQ(settings.value("theme/titleRatio").toDouble(), 25.0);
    fs::remove_all(tmp);
    theme.resetFactory();
    EXPECT_EQ(theme.getParam("titleRatio").toDouble(), 2.0);
}

TEST(gui_VisualTheme, ExportJSON) {
    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "test.json";
    QSettings settings;
    VisualTheme theme(&settings);
    theme.exportJSON(file);
    std::ifstream inFile(file, std::ios::in);
    std::string resu{(std::istreambuf_iterator<char>(inFile)), (std::istreambuf_iterator<char>())};
    inFile.close();
    EXPECT_STREQ(resu.c_str(), "{\n    \"theme\": {\n        \"backgroundColor\": \"#F0F0F0\",\n        \"baseRatio\": 0.02,\n        \"fadeNumbers\": true,\n        \"fadeNumbersAmount\": 3,\n        \"fadeNumbersStrength\": 10,\n        \"gridBackgroundColor\": \"#F0F0F0\",\n        \"gridTextRatio\": 0.85,\n        \"longTextRatio\": 0.6,\n        \"name\": \"default\",\n        \"selectedNumberColor\": \"#FF7000\",\n        \"shortTextRatio\": 1.4,\n        \"textColor\": \"#000000\",\n        \"titleRatio\": 2.0,\n        \"truncatePrice\": true,\n        \"truncatePriceLines\": 3\n    }\n}");
    fs::remove_all(tmp);
}

TEST(gui_VisualTheme, ImportJSON) {
    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "test.json";

    json j= {{"theme", {{"backgroundColor", "#F0F0F0"}, {"fadeNumbers", true}, {"fadeNumbersAmount", 3}, {"baseRatio", 0.04}, {"gridTextRatio", 0.85}, {"longTextRatio", 0.6}, {"name", "titi"}, {"shortTextRatio", 1.4}, {"titleRatio", 2.0}}}};
    std::ofstream outFile(file, std::ios::out);
    outFile << j;
    outFile.close();

    QSettings settings;
    VisualTheme theme(&settings);
    theme.importJSON(file);
    EXPECT_EQ(theme.getParam("baseRatio").toDouble(), 0.04);
    EXPECT_STREQ(theme.getParam("name").toString().toStdString().c_str(), "titi");
    fs::remove_all(tmp);
}
