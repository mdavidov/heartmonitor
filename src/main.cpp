
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
    app.setWindowIcon(QIcon(":/icons/heart-monitor.png"));

    qmlRegisterType<HMController>("HeartMonitor", 1, 0, "HMController");
    qmlRegisterType<EcgDataModel>("HeartMonitor", 1, 0, "EcgDataModel");
    qmlRegisterType<BluetoothManager>("HeartMonitor", 1, 0, "BluetoothManager");
    qmlRegisterType<ArrhythmiaDetector>("HeartMonitor", 1, 0, "ArrhythmiaDetector");

    HMController hmController;
    int res = 1;

    // Ensure QML engine is destroyed before those C++ objects that are used in QML
    // (e.g. hmController) are destroyed, to avoid crashes during application shutdown.
    {
        QQmlApplicationEngine eng;

        // Expose controller to QML
        eng.rootContext()->setContextProperty("hmController", &hmController);

        eng.addImportPath(QCoreApplication::applicationDirPath() + "qrc:/qml");
        Q_INIT_RESOURCE(qml);
        Q_INIT_RESOURCE(resources);

        eng.load(QUrl("qrc:/qml/main.qml"));
        if (eng.rootObjects().isEmpty()) {
            qDebug() << "Failed to load QML root object";
        }

        res = app.exec();

    } // QQmlApplicationEngine destroyed here, before hmController is destroyed

    return res;
}