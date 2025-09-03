
#include "hmcontroller.h"
#include "ecgdatamodel.h"
#include "bluetoothmanager.h"
#include "arrhythmiadetector.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDir>
#include <QQmlEngine>
#include <QtQml>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Heart Monitor");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("DevOnline");
    
    // Register QML types
    // qmlRegisterType<HMController>("HeartMonitor", 1, 0, "HMController");
    // qmlRegisterType<EcgDataModel>("HeartMonitor", 1, 0, "EcgDataModel");
    // qmlRegisterType<BluetoothManager>("HeartMonitor", 1, 0, "BluetoothManager");
    // qmlRegisterType<ArrhythmiaDetector>("HeartMonitor", 1, 0, "ArrhythmiaDetector");

    // Create main controller
    HMController hmController;

    // Create QML engine
    QQmlApplicationEngine eng;

    // Expose controller to QML
    eng.rootContext()->setContextProperty("hmController", &hmController);

    eng.load(QUrl("qrc:/qml/main.qml"));
    if (eng.rootObjects().isEmpty()) {
        qDebug() << "Failed to load QML root object";
    }
    
    return app.exec();
}