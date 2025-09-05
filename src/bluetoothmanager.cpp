#include "bluetoothmanager.h"
#include <QDebug>
#include <QBluetoothServiceDiscoveryAgent>
#include <QtMath>
#include <QRandomGenerator>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
    , m_discoveryAgent(nullptr)
    , m_socket(nullptr)
    , m_isScanning(false)
    , m_isConnected(false)
    , m_useSimulation(true) // Enable simulation by default for testing
    , m_simulationTime(0.0)
    , m_simulationHeartRate(72)
{
    // Initialize Bluetooth discovery agent
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothManager::deviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothManager::scanFinished);
    connect(m_discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::errorOccurred),
            this, &BluetoothManager::scanError);
    
    // Initialize simulation timer for testing
    m_simulationTimer = new QTimer(this);
    connect(m_simulationTimer, &QTimer::timeout, this, &BluetoothManager::simulateEcgData);
    m_simulationTimer->setInterval(4); // 250 Hz sampling rate (4ms intervals)
    
    qDebug() << "BluetoothManager initialized";
}

BluetoothManager::~BluetoothManager()
{
    disconnectFromDevice();
}

bool BluetoothManager::isScanning() const
{
    return m_isScanning;
}

bool BluetoothManager::isConnected() const
{
    return m_isConnected;
}

QString BluetoothManager::connectedDeviceName() const
{
    return m_connectedDeviceName;
}

QVariantList BluetoothManager::availableDevices() const
{
    QVariantList deviceList;
    for (const QBluetoothDeviceInfo &device : m_devices) {
        QVariantMap deviceMap;
        deviceMap["name"] = device.name();
        deviceMap["address"] = device.address().toString();
        deviceMap["rssi"] = device.rssi();
        deviceList.append(deviceMap);
    }
    return deviceList;
}

void BluetoothManager::startScanning()
{
    if (m_useSimulation) {
        // For simulation, immediately "connect" and start generating data
        m_isConnected = true;
        m_connectedDeviceName = "ECG Simulator";
        m_simulationTimer->start();
        emit connectionStateChanged(true);
        qDebug() << "Started ECG simulation";
        return;
    }
    
    if (m_isScanning) {
        return;
    }
    
    m_devices.clear();
    m_isScanning = true;
    m_discoveryAgent->start();
    emit scanningChanged();
    emit devicesUpdated();
    
    qDebug() << "Started Bluetooth scanning";
}

void BluetoothManager::stopScanning()
{
    if (m_isScanning) {
        m_discoveryAgent->stop();
        m_isScanning = false;
        emit scanningChanged();
    }
}

void BluetoothManager::connectToDevice(const QString &deviceAddress)
{
    if (m_useSimulation) {
        startScanning(); // This will start simulation
        return;
    }
    
    // Find the device
    QBluetoothDeviceInfo targetDevice;
    bool deviceFound = false;
    
    for (const QBluetoothDeviceInfo &device : m_devices) {
        if (device.address().toString() == deviceAddress) {
            targetDevice = device;
            deviceFound = true;
            break;
        }
    }
    
    if (!deviceFound) {
        emit error("Device not found");
        return;
    }
    
    // Create socket and connect
    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    
    connect(m_socket, &QBluetoothSocket::connected, this, &BluetoothManager::socketConnected);
    connect(m_socket, &QBluetoothSocket::disconnected, this, &BluetoothManager::socketDisconnected);
    connect(m_socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::errorOccurred),
            this, &BluetoothManager::socketError);
    connect(m_socket, &QBluetoothSocket::readyRead, this, &BluetoothManager::socketReadyRead);
    
    // Connect to standard serial port service UUID
    QBluetoothUuid serviceUuid(QBluetoothUuid::ServiceClassUuid::SerialPort);
    m_socket->connectToService(targetDevice.address(), serviceUuid);
    
    qDebug() << "Connecting to device:" << targetDevice.name();
}

void BluetoothManager::disconnectFromDevice()
{
    if (m_useSimulation) {
        m_simulationTimer->stop();
        m_isConnected = false;
        m_connectedDeviceName.clear();
        emit connectionStateChanged(false);
        qDebug() << "Stopped ECG simulation";
        return;
    }
    
    if (m_socket) {
        m_socket->disconnectFromService();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    m_isConnected = false;
    m_connectedDeviceName.clear();
    emit connectionStateChanged(false);
}

QVariantList BluetoothManager::getAvailableDevices()
{
    return availableDevices();
}

// Private slots
void BluetoothManager::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    // Filter for ECG-related devices (you may need to adjust this)
    if (device.name().contains("ECG", Qt::CaseInsensitive) ||
        device.name().contains("Heart", Qt::CaseInsensitive) ||
        device.name().contains("Polar", Qt::CaseInsensitive)) {
        
        m_devices.append(device);
        emit devicesUpdated();
        
        qDebug() << "ECG device found:" << device.name() << device.address().toString();
    }
}

void BluetoothManager::scanFinished()
{
    m_isScanning = false;
    emit scanningChanged();
    qDebug() << "Bluetooth scan finished. Found" << m_devices.size() << "ECG devices";
}

void BluetoothManager::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    m_isScanning = false;
    emit scanningChanged();
    
    QString errorString;
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        errorString = "Bluetooth is powered off";
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        errorString = "Invalid Bluetooth adapter";
        break;
    default:
        errorString = "Bluetooth scan error";
        break;
    }
    
    emit this->error(errorString);
    qWarning() << "Bluetooth scan error:" << errorString;
}

void BluetoothManager::socketConnected()
{
    m_isConnected = true;
    m_connectedDeviceName = m_socket->peerName();
    emit connectionStateChanged(true);
    
    qDebug() << "Connected to ECG device:" << m_connectedDeviceName;
}

void BluetoothManager::socketDisconnected()
{
    m_isConnected = false;
    m_connectedDeviceName.clear();
    emit connectionStateChanged(false);
    
    qDebug() << "Disconnected from ECG device";
}

void BluetoothManager::socketError(QBluetoothSocket::SocketError error)
{
    QString errorString;
    switch (error) {
    case QBluetoothSocket::SocketError::RemoteHostClosedError:
        errorString = "Remote device closed the connection";
        break;
    case QBluetoothSocket::SocketError::HostNotFoundError:
        errorString = "Device not found";
        break;
    case QBluetoothSocket::SocketError::ServiceNotFoundError:
        errorString = "Service not found on device";
        break;
    default:
        errorString = "Bluetooth connection error";
        break;
    }
    
    emit this->error(errorString);
    qWarning() << "Bluetooth socket error:" << errorString;
}

void BluetoothManager::socketReadyRead()
{
    if (!m_socket) return;
    
    QByteArray data = m_socket->readAll();
    m_incomingBuffer.append(data);
    
    // Process complete packets (assuming newline-terminated)
    while (m_incomingBuffer.contains('\n')) {
        int newlineIndex = m_incomingBuffer.indexOf('\n');
        QByteArray packet = m_incomingBuffer.left(newlineIndex);
        m_incomingBuffer.remove(0, newlineIndex + 1);
        
        processIncomingData(packet);
    }
}

void BluetoothManager::simulateEcgData()
{
    // Generate realistic ECG simulation
    double t = m_simulationTime;
    double heartRateBpm = m_simulationHeartRate + QRandomGenerator::global()->bounded(-5, 5); // Slight variation
    double heartRateHz = heartRateBpm / 60.0;
    
    // ECG waveform simulation (simplified)
    double ecgValue = 0.0;
    
    // P wave, QRS complex, T wave simulation
    double heartCycle = fmod(t * heartRateHz, 1.0);
    
    if (heartCycle < 0.1) {
        // P wave
        ecgValue = 0.2 * sin(heartCycle * 31.4159);
    } else if (heartCycle > 0.15 && heartCycle < 0.25) {
        // QRS complex
        double qrsPhase = (heartCycle - 0.15) / 0.1;
        if (qrsPhase < 0.3) {
            ecgValue = -0.1 * sin(qrsPhase * 10.47);
        } else if (qrsPhase < 0.7) {
            ecgValue = 1.0 * sin((qrsPhase - 0.3) * 7.85);
        } else {
            ecgValue = -0.3 * sin((qrsPhase - 0.7) * 10.47);
        }
    } else if (heartCycle > 0.4 && heartCycle < 0.6) {
        // T wave
        ecgValue = 0.3 * sin((heartCycle - 0.4) * 15.7);
    }
    
    // Add some noise
    ecgValue += (QRandomGenerator::global()->generateDouble() - 0.5) * 0.05;
    
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    emit newEcgData(ecgValue, timestamp);
    
    m_simulationTime += 0.004; // 4ms increment (250 Hz)
}

void BluetoothManager::processIncomingData(const QByteArray &data)
{
    double ecgValue = parseEcgValue(data);
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    
    emit newEcgData(ecgValue, timestamp);
}

double BluetoothManager::parseEcgValue(const QByteArray &data)
{
    // Parse ECG value from device-specific format
    // This is a simplified example - adjust based on your device's protocol
    QString dataStr = QString::fromUtf8(data).trimmed();
    
    // Assuming format like "ECG:1.234" or just "1.234"
    if (dataStr.startsWith("ECG:")) {
        return dataStr.mid(4).toDouble();
    }
    
    return dataStr.toDouble();
}