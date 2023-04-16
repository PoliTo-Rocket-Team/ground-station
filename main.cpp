#include <QApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QFont>
#include <QFontDatabase>
#include "antenna.h"

#include <QFile>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/imgs/icon"));

    int font_id = QFontDatabase::addApplicationFont(":/JakartaSans/Regular");
    QFontDatabase::addApplicationFont(":/JakartaSans/Bold");
    QFontDatabase::addApplicationFont(":/JakartaSans/SemiBold");
    QFontDatabase::addApplicationFont(":/JakartaSans/Medium");
    QFontDatabase::addApplicationFont(":/JakartaSans/Light");

    QString family = QFontDatabase::applicationFontFamilies(font_id).at(0);
    QFont font(family, 12);
    app.setFont(font);

    // qRegisterMetaType<RocketData>();

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
