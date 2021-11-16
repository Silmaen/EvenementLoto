/**
* @file VisualTheme.cpp
* @author Silmaen
* @date 15/11/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "gui/VisualTheme.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace evl::gui {

using json= nlohmann::json;

static const QVariant nullVariant;

VisualTheme::VisualTheme(QSettings* set):
    settings{set} {
    setFromSettings();
    writeInSettings();
}

void VisualTheme::resetFactory() {
    parameters= themeDefaults;
}

void VisualTheme::exportJSON(const path& file) const {
    std::ofstream outFile(file, std::ios::out);
    json j;
    for(auto& parameter: parameters) {
        if(parameter.second.metaType() == QMetaType(QMetaType::Int))
            j[parameter.first.toStdString()]= parameter.second.toInt();
        else if(parameter.second.metaType() == QMetaType(QMetaType::Double))
            j[parameter.first.toStdString()]= parameter.second.toDouble();
        else if(parameter.second.metaType() == QMetaType(QMetaType::Bool))
            j[parameter.first.toStdString()]= parameter.second.toBool();
        else
            j[parameter.first.toStdString()]= parameter.second.toString().toStdString();
    }
    outFile << std::setw(4) << json{{"theme", j}};
    outFile.close();
}

void VisualTheme::importJSON(const path& file) {
    std::ifstream inFile(file, std::ios::in);
    json j;
    inFile >> j;
    inFile.close();
    for(auto& item: j["theme"].items()) {
        QString key= QString::fromUtf8(item.key());
        if(themeDefaults.contains(key)) {
            if(themeDefaults.at(key).metaType() == QMetaType(QMetaType::Int))
                parameters[key]= item.value().get<int>();
            else if(themeDefaults.at(key).metaType() == QMetaType(QMetaType::Double))
                parameters[key]= item.value().get<double>();
            else if(themeDefaults.at(key).metaType() == QMetaType(QMetaType::Bool))
                parameters[key]= item.value().get<bool>();
            else
                parameters[key]= QString::fromUtf8(item.value().get<string>());
        }
    }
}

void VisualTheme::setFromSettings() {
    for(auto& element: themeDefaults) {
        // conservation du type
        if(settings->value("theme/" + element.first, element.second).typeId() != element.second.typeId())
            return;
        parameters[element.first]= settings->value("theme/" + element.first, element.second);
    }
}

void VisualTheme::writeInSettings() {
    for(auto& parameter: parameters)
        settings->setValue("theme/" + parameter.first, parameter.second);
    settings->sync();
}

const QVariant& VisualTheme::getParam(const QString& key) const {
    if(themeDefaults.contains(key)) {
        return parameters.at(key);
    }
    return nullVariant;
}

void VisualTheme::setParam(const QString& key, const QVariant& value) {
    if(!themeDefaults.contains(key))
        return;
    if(themeDefaults.at(key).typeId() != value.typeId())
        return;
    parameters[key]= value;
}

}// namespace evl::gui