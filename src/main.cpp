/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QQmlContext>
#include <QSettings>
#include <QFile>
#include "device_info.hpp"
#include "rauc.hpp"

void WriteDefaultSettings()
{
    QSettings settings;
    QString group = settings.group();
    QList<QString> enabled_pages = {
                                    "Image Viewer",
                                    "Multimedia",
                                    "RAUC – Update Client",
                                    "Multitouch",
                                    "Device Information",
                                    "Widget Factory",
                                    "About PHYTEC"
    };

    settings.beginWriteArray("enabled_pages", enabled_pages.size());
    for (int i = 0; i < enabled_pages.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("name", enabled_pages.at(i));
    }
    settings.endArray();
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("PHYTEC Messtechnik GmbH");
    app.setApplicationName("qtphy");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOptions({
            {
                {"c", "rauc-hawkbit-config"},
                "RAUC hawkBit client configuration path",
                "path",
                "/etc/rauc-hawkbit-updater/config.conf"
            }
    });
    parser.process(app);

    QSettings settings;
    QFile file(settings.fileName());
    qDebug() << "Config file: " << file.fileName();
    if (!file.exists()) {
        qDebug() << "WARNING: Settings file " << file.fileName() << "not found!";
        qDebug() << "Writing new settings file with default values.";
        WriteDefaultSettings();
    }

    QVariantList pages;
    int size = settings.beginReadArray("enabled_pages");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        pages.append(settings.value("name").toString());
    }

    qmlRegisterSingletonType<DeviceInfo>("Phytec.DeviceInfo", 1, 0, "DeviceInfo",
                                         DeviceInfo::singletontypeProvider);
    qmlRegisterType<Rauc>("Phytec.Rauc", 1, 0, "Rauc");

    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:///themes");
    engine.rootContext()->setContextProperty("raucHawkbitConfigPath",
                                             parser.value("rauc-hawkbit-config"));
    engine.rootContext()->setContextProperty("enabledPages", pages);
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
