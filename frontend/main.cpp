//
// Created by sigris on 12.04.2026.
//

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include "GameClient/GameClient.h"
#include "MapRender/MapRenderer.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setOrganizationName("MiniCiv");
    app.setApplicationName("Client");

    qmlRegisterType<MapRenderer>("MiniCiv", 1, 0, "MapRenderer");

    QQmlApplicationEngine engine;
    GameClient client;
    engine.rootContext()->setContextProperty("game", &client);
    client.connectToServer("127.0.0.1", 8080);

    const QUrl url(u"qrc:/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(url);

    return engine.rootObjects().isEmpty() ? -1 : app.exec();
}