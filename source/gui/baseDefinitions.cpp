/**
 * @file baseDefinitions.cpp
 * @author argawaen
 * @date 07/10/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "baseDefinitions.h"
#include <QMessageBox>
#include <QPainter>
#include <QSvgRenderer>
#include <spdlog/spdlog.h>

namespace evl::gui {

void showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

QImage loadImage(const path& from) {
    spdlog::debug("Loading image {} with type: {}.", from.string(), from.extension().string());
    QString imgName(QString::fromUtf8(from.string()));
    if(from.extension() == ".svg") {
        QSvgRenderer renderer(imgName);
        double ratio= renderer.viewBoxF().height() / renderer.viewBoxF().width();
        QImage img(1920, static_cast<int>(1920 * ratio), QImage::Format_ARGB32);
        QPainter painter(&img);
        renderer.render(&painter);
        return img;
    } else {
        return QImage(imgName);
    }
}
}// namespace evl::gui
