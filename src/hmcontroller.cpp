#include "hmcontroller.h"
#include "ecgdatamodel.h"
#include "bluetoothmanager.h"
#include "arrhythmiadetector.h"

#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QUrl>
#include <QtMath>

HMController::HMController(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
    , m_isRecording(false)
    , m_currentHeartRate(0)
    , m_connectionStatus("Disconnected")
    , m_alertLevel(0)
    , m_lastHeartRateCalculation(0)
{
    // Initialize components
    m_ecgDataModel = new EcgDataModel(this);
    m_bluetoothManager = new BluetoothManager(this);
    m_arrhythmiaDetector = new ArrhythmiaDetector(this);
    
    // Initialize database
    initializeDatabase();
    
    // Setup heart rate calculation timer
    m_heartRateTimer = new QTimer(this);
    m_heartRateTimer->setInterval(2000); // Update every 2 seconds
    connect(m_heartRateTimer, &QTimer::timeout, this, &HMController::updateHeartRate);
    
    // Connect signals
    connect(m_bluetoothManager, &BluetoothManager::newEcgData,
            this, &HMController::onNewEcgReading);
    connect(m_bluetoothManager, &BluetoothManager::connectionStateChanged,
            this, &HMController::onConnectionStateChanged);
    connect(m_arrhythmiaDetector, &ArrhythmiaDetector::arrhythmiaDetected,
            this, &HMController::onArrhythmiaDetected);
    
    qDebug() << "HMController initialized";
}

HMController::~HMController()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

void HMController::initializeDatabase()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dataPath + "/heartmonitor.db");
    
    if (!m_database.open()) {
        qWarning() << "Failed to open database:" << m_database.lastError().text();
        return;
    }
    
    QSqlQuery query;
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS ecg_readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            voltage REAL NOT NULL,
            heart_rate INTEGER,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createTable)) {
        qWarning() << "Failed to create table:" << query.lastError().text();
    }
    
    // Create index for performance
    query.exec("CREATE INDEX IF NOT EXISTS idx_timestamp ON ecg_readings(timestamp)");
    
    qDebug() << "Database initialized successfully";
}

// Property getters
bool HMController::isConnected() const
{
    return m_isConnected;
}

int HMController::currentHeartRate() const
{
    return m_currentHeartRate;
}

QString HMController::connectionStatus() const
{
    return m_connectionStatus;
}

bool HMController::isRecording() const
{
    return m_isRecording;
}

EcgDataModel* HMController::ecgDataModel() const
{
    return m_ecgDataModel;
}

QString HMController::alertMessage() const
{
    return m_alertMessage;
}

int HMController::alertLevel() const
{
    return m_alertLevel;
}

// Invokable methods
void HMController::startConnection()
{
    m_bluetoothManager->startScanning();
    m_connectionStatus = "Connecting...";
    emit connectionStatusChanged();
}

void HMController::stopConnection()
{
    m_bluetoothManager->disconnectFromDevice();
    m_isConnected = false;
    m_connectionStatus = "Disconnected";
    m_heartRateTimer->stop();
    emit connectionStatusChanged();
}

void HMController::startRecording()
{
    if (!m_isConnected) {
        qWarning() << "Cannot start recording: not connected to device";
        return;
    }
    
    m_isRecording = true;
    m_heartRateTimer->start();
    emit recordingStatusChanged();
    
    qDebug() << "Recording started";
}

void HMController::stopRecording()
{
    m_isRecording = false;
    m_heartRateTimer->stop();
    emit recordingStatusChanged();
    
    qDebug() << "Recording stopped";
}

void HMController::exportData(const QString& filePath)
{
    QFile file(QUrl(filePath).toLocalFile());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit dataExported(false, "Failed to open file for writing");
        return;
    }
    
    QTextStream out(&file);
    out << "Timestamp,Voltage,HeartRate,DateTime\n";
    
    QSqlQuery query;
    query.prepare("SELECT timestamp, voltage, heart_rate, created_at FROM ecg_readings ORDER BY timestamp");
    
    if (!query.exec()) {
        emit dataExported(false, "Database query failed: " + query.lastError().text());
        return;
    }
    
    int recordCount = 0;
    while (query.next()) {
        quint64 timestamp = query.value(0).toULongLong();
        double voltage = query.value(1).toDouble();
        int heartRate = query.value(2).toInt();
        QString dateTime = query.value(3).toString();
        
        out << timestamp << "," << voltage << "," << heartRate << "," << dateTime << "\n";
        recordCount++;
    }
    
    file.close();
    emit dataExported(true, QString("Exported %1 records successfully").arg(recordCount));
}

void HMController::clearHistory()
{
    QSqlQuery query;
    if (query.exec("DELETE FROM ecg_readings")) {
        m_ecgDataModel->clearData();
        qDebug() << "History cleared";
    } else {
        qWarning() << "Failed to clear history:" << query.lastError().text();
    }
}

QVariantList HMController::getAvailableDevices()
{
    return m_bluetoothManager->getAvailableDevices();
}

// Private slots
void HMController::onNewEcgReading(double voltage, quint64 timestamp)
{
    // Store recent data for heart rate calculation
    m_recentEcgData.append(voltage);
    m_recentTimestamps.append(timestamp);
    
    // Keep only recent samples
    while (m_recentEcgData.size() > MAX_RECENT_SAMPLES) {
        m_recentEcgData.removeFirst();
        m_recentTimestamps.removeFirst();
    }
    
    // Save to database if recording
    if (m_isRecording) {
        saveEcgReading(voltage, timestamp, m_currentHeartRate);
        m_ecgDataModel->addReading(voltage, timestamp, m_currentHeartRate);
    }
    
    // Send to arrhythmia detector
    m_arrhythmiaDetector->processEcgSample(voltage, timestamp);
    
    // Emit for real-time graph
    emit newEcgData(voltage, timestamp);
}

void HMController::onConnectionStateChanged(bool connected)
{
    m_isConnected = connected;
    m_connectionStatus = connected ? "Connected" : "Disconnected";
    
    if (!connected) {
        m_isRecording = false;
        m_heartRateTimer->stop();
        emit recordingStatusChanged();
    }
    
    emit connectionStatusChanged();
}

void HMController::onArrhythmiaDetected(const QString& type, int severity)
{
    m_alertMessage = QString("Arrhythmia detected: %1").arg(type);
    m_alertLevel = severity;
    emit alertTriggered();
    
    qWarning() << "Arrhythmia alert:" << type << "severity:" << severity;
}

void HMController::updateHeartRate()
{
    if (m_recentEcgData.size() < 100) {
        return; // Need more data
    }
    
    calculateHeartRate(m_recentEcgData);
}

void HMController::saveEcgReading(double voltage, quint64 timestamp, int heartRate)
{
    QSqlQuery query;
    query.prepare("INSERT INTO ecg_readings (timestamp, voltage, heart_rate) VALUES (?, ?, ?)");
    query.addBindValue(timestamp);
    query.addBindValue(voltage);
    query.addBindValue(heartRate > 0 ? heartRate : QVariant());
    
    if (!query.exec()) {
        qWarning() << "Failed to save ECG reading:" << query.lastError().text();
    }
}

void HMController::calculateHeartRate(const QList<double>& ecgData)
{
    if (ecgData.size() < 100 || m_recentTimestamps.size() != ecgData.size()) {
        return;
    }
    
    // Simple R-peak detection algorithm
    QList<int> rPeaks;
    double threshold = 0.5; // Adjust based on your ECG signal characteristics
    
    // Find peaks that are above threshold and local maxima
    for (int i = 2; i < ecgData.size() - 2; ++i) {
        if (ecgData[i] > threshold &&
            ecgData[i] > ecgData[i-1] && ecgData[i] > ecgData[i+1] &&
            ecgData[i] > ecgData[i-2] && ecgData[i] > ecgData[i+2]) {
            
            // Check if this peak is far enough from the last one (minimum 300ms)
            if (rPeaks.isEmpty() || (m_recentTimestamps[i] - m_recentTimestamps[rPeaks.last()]) > 300) {
                rPeaks.append(i);
            }
        }
    }
    
    // Calculate heart rate from R-R intervals
    if (rPeaks.size() >= 3) {
        double totalInterval = 0;
        int intervalCount = 0;
        
        for (int i = 1; i < rPeaks.size(); ++i) {
            double interval = m_recentTimestamps[rPeaks[i]] - m_recentTimestamps[rPeaks[i-1]];
            if (interval > 300 && interval < 2000) { // Valid R-R interval (30-200 BPM range)
                totalInterval += interval;
                intervalCount++;
            }
        }
        
        if (intervalCount > 0) {
            double avgInterval = totalInterval / intervalCount;
            int newHeartRate = qRound(60000.0 / avgInterval); // Convert to BPM
            
            // Smooth the heart rate to avoid rapid fluctuations
            if (m_currentHeartRate == 0) {
                m_currentHeartRate = newHeartRate;
            } else {
                m_currentHeartRate = (m_currentHeartRate * 3 + newHeartRate) / 4;
            }
            
            emit heartRateChanged();
        }
    }
}