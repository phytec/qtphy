/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QQmlContext>
#include "device_info.hpp"
#include "rauc.hpp"

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

    qmlRegisterSingletonType<DeviceInfo>("Phytec.DeviceInfo", 1, 0, "DeviceInfo",
                                         DeviceInfo::singletontypeProvider);
    qmlRegisterType<Rauc>("Phytec.Rauc", 1, 0, "Rauc");

    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:///themes");
    engine.rootContext()->setContextProperty("raucHawkbitConfigPath",
                                             parser.value("rauc-hawkbit-config"));
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
