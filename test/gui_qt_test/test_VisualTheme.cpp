/**
 * @file test_VisualTheme.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui_qt/VisualTheme.h"
#include "test_GuiHelpers.h"
#include <fstream>
#include <json/json.h>

namespace fs = std::filesystem;
using namespace evl::gui;

TEST(gui_VisualTheme, base) {
	const fs::path tmp = fs::temp_directory_path() / "test";
	fs::create_directories(tmp);
	const fs::path fileSettings = tmp / "settings.ini";

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
	const fs::path tmp = fs::temp_directory_path() / "test";
	fs::create_directories(tmp);
	const fs::path fileSettings = tmp / "settings.ini";
	QSettings settings{QString::fromUtf8(fileSettings.string()), QSettings::IniFormat};
	VisualTheme theme(&settings);
	settings.sync();
	theme.setParam("titleRatio", 25.0);
	theme.setParam("totocaca", 25.0);// mauvais nom
	theme.setParam("titleRatio", "juste pourri");// mauvais type
	theme.writeInSettings();

	EXPECT_EQ(settings.value("theme/titleRatio").toDouble(), 25.0);
	fs::remove_all(tmp);
	theme.resetFactory();
	EXPECT_EQ(theme.getParam("titleRatio").toDouble(), 2.0);
}

TEST(gui_VisualTheme, ExportJSON) {
	const fs::path tmp = fs::temp_directory_path() / "test";
	fs::create_directories(tmp);
	const fs::path file = tmp / "test.json";
	QSettings settings;
	const VisualTheme theme(&settings);
	theme.exportJSON(file);
	std::ifstream inFile(file, std::ios::in);
	const std::string resu{(std::istreambuf_iterator<char>(inFile)), (std::istreambuf_iterator<char>())};
	inFile.close();
	EXPECT_STREQ(resu.c_str(),
				 "   {\n\t\"theme\" : \n\t{\n\t\t\"backgroundColor\" : \"#F0F0F0\",\n\t\t\"baseRatio\" : "
				 "0.02,\n\t\t\"fadeNumbers\" : true,\n\t\t\"fadeNumbersAmount\" : 3,\n\t\t\"fadeNumbersStrength\" : "
				 "10,\n\t\t\"gridBackgroundColor\" : \"#F0F0F0\",\n\t\t\"gridTextRatio\" : 0.84999999999999998,\n\t\t"
				 "\"longTextRatio\" : 0.59999999999999998,\n\t\t\"name\" : \"default\",\n\t\t\"selectedNumberColor\" : "
				 "\"#FF7000\",\n\t\t\"shortTextRatio\" : 1.3999999999999999,\n\t\t\"textColor\" : \"#000000\",\n\t\t"
				 "\"titleRatio\" : 2.0,\n\t\t\"truncatePrice\" : true,\n\t\t\"truncatePriceLines\" : 3\n\t}\n}");
	fs::remove_all(tmp);
}

TEST(gui_VisualTheme, ImportJSON) {
	const fs::path tmp = fs::temp_directory_path() / "test";
	fs::create_directories(tmp);
	const fs::path file = tmp / "test.json";

	Json::Value j;
	j["theme"]["backgroundColor"] = "#F0F0F0";
	j["theme"]["fadeNumbers"] = true;
	j["theme"]["fadeNumbersAmount"] = 3;
	j["theme"]["baseRatio"] = 0.04;
	j["theme"]["gridTextRatio"] = 0.85;
	j["theme"]["longTextRatio"] = 0.6;
	j["theme"]["name"] = "titi";
	j["theme"]["shortTextRatio"] = 1.4;
	j["theme"]["titleRatio"] = 2.;
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
