#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "antenna.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QScopedPointer<Antenna> antenna(new Antenna);

    QQmlApplicationEngine engine;
    qmlRegisterSingletonInstance("Antenna", 1, 0, "Antenna", antenna.get());


    const QUrl url(u"qrc:/ground-station/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
