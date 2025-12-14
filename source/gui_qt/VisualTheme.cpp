/**
* @file VisualTheme.cpp
* @author Silmaen
* @date 15/11/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "pch.h"

#include "VisualTheme.h"
#include <fstream>
#include <json/json.h>

namespace evl::gui {

static const QVariant nullVariant;

VisualTheme::VisualTheme(QSettings* set) : settings{set} {
	setFromSettings();
	writeInSettings();
}

void VisualTheme::resetFactory() {
	parameters = themeDefaults;
	toUpdate = true;
}

void VisualTheme::exportJSON(const std::filesystem::path& file) const {
	std::ofstream outFile(file, std::ios::out);
	Json::Value j;
	for (const auto& [key, val]: parameters) {
		if (val.metaType() == QMetaType(QMetaType::Int))
			j[key.toStdString()] = val.toInt();
		else if (val.metaType() == QMetaType(QMetaType::Double))
			j[key.toStdString()] = val.toDouble();
		else if (val.metaType() == QMetaType(QMetaType::Bool))
			j[key.toStdString()] = val.toBool();
		else
			j[key.toStdString()] = val.toString().toStdString();
	}
	Json::Value result;
	result["theme"] = j;
	outFile << std::setw(4) << result;
	outFile.close();
}

void VisualTheme::importJSON(const std::filesystem::path& file) {
	std::ifstream inFile(file, std::ios::in);
	Json::Value j;
	inFile >> j;
	inFile.close();
	for (auto& s_key: j["theme"].getMemberNames()) {
		auto& val = j["theme"][s_key];
		if (const QString key = QString::fromUtf8(s_key); themeDefaults.contains(key)) {
			if (themeDefaults.at(key).metaType() == QMetaType(QMetaType::Int))
				parameters[key] = val.asInt();
			else if (themeDefaults.at(key).metaType() == QMetaType(QMetaType::Double))
				parameters[key] = val.asDouble();
			else if (themeDefaults.at(key).metaType() == QMetaType(QMetaType::Bool))
				parameters[key] = val.asBool();
			else
				parameters[key] = QString::fromUtf8(val.asString());
		}
	}
	toUpdate = true;
}

void VisualTheme::setFromSettings() {
	if (settings == nullptr)
		return;
	for (const auto& [key, val]: themeDefaults) {
		// conservation du type
		const QVariant v(settings->value("theme/" + key, val));
		if (val.metaType() == QMetaType(QMetaType::Int))
			parameters[key] = v.toInt();
		else if (val.metaType() == QMetaType(QMetaType::Double))
			parameters[key] = v.toDouble();
		else if (val.metaType() == QMetaType(QMetaType::Bool))
			parameters[key] = v.toBool();
		else
			parameters[key] = v.toString();
	}
	toUpdate = true;
}

void VisualTheme::writeInSettings() {
	if (settings == nullptr)
		return;
	for (auto& [key, val]: parameters) settings->setValue("theme/" + key, val);
	settings->sync();
	toUpdate = true;
}

auto VisualTheme::getParam(const QString& iKey) -> const QVariant& {
	if (themeDefaults.contains(iKey)) {
		toUpdate = false;
		return parameters.at(iKey);
	}
	return nullVariant;
}

void VisualTheme::setParam(const QString& iKey, const QVariant& iValue) {
	if (!themeDefaults.contains(iKey))
		return;
	if (themeDefaults.at(iKey).typeId() != iValue.typeId())
		return;
	parameters[iKey] = iValue;
	toUpdate = true;
}

}// namespace evl::gui
