
#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QTimer>
#include <QVariantList>
#include <QQmlEngine>
#include <QtQml>
#include <QtQml/qqmlregistration.h>

QT_FORWARD_DECLARE_CLASS(QBluetoothServiceDiscoveryAgent)

class BluetoothManager : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
    
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStateChanged)
    Q_PROPERTY(QString connectedDeviceName READ connectedDeviceName NOTIFY connectionStateChanged)
    Q_PROPERTY(QVariantList availableDevices READ availableDevices NOTIFY devicesUpdated)

public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager();

    // Property getters
    bool isScanning() const;
    bool isConnected() const;
    QString connectedDeviceName() const;
    QVariantList availableDevices() const;

    // Invokable methods
    Q_INVOKABLE void startScanning();
    Q_INVOKABLE void stopScanning();
    Q_INVOKABLE void connectToDevice(const QString &deviceAddress);
    Q_INVOKABLE void disconnectFromDevice();
    Q_INVOKABLE QVariantList getAvailableDevices();

signals:
    void scanningChanged();
    void connectionStateChanged(bool connected);
    void devicesUpdated();
    void newEcgData(double voltage, quint64 timestamp);
    void error(const QString &errorString);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void scanFinished();
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void socketConnected();
    void socketDisconnected();
    void socketError(QBluetoothSocket::SocketError error);
    void socketReadyRead();
    void simulateEcgData(); // For testing without actual device

private:
    void processIncomingData(const QByteArray &data);
    double parseEcgValue(const QByteArray &data);
    
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QBluetoothSocket *m_socket;
    QTimer *m_simulationTimer; // For testing
    
    QList<QBluetoothDeviceInfo> m_devices;
    QString m_connectedDeviceName;
    QByteArray m_incomingBuffer;
    
    bool m_isScanning;
    bool m_isConnected;
    bool m_useSimulation; // For testing without actual device
    
    // Simulation variables
    double m_simulationTime;
    int m_simulationHeartRate;
};
