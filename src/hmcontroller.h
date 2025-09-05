
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QQmlEngine>
#include <QtQml>
#include <QtQml/qqmlregistration.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QStandardPaths>

class EcgDataModel;
class BluetoothManager;
class ArrhythmiaDetector;

class HMController : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
    
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(int currentHeartRate READ currentHeartRate NOTIFY heartRateChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingStatusChanged)
    Q_PROPERTY(EcgDataModel* ecgDataModel READ ecgDataModel CONSTANT)
    Q_PROPERTY(QString alertMessage READ alertMessage NOTIFY alertTriggered)
    Q_PROPERTY(int alertLevel READ alertLevel NOTIFY alertTriggered)

public:
    explicit HMController(QObject* parent = nullptr);
    ~HMController();

    // Property getters
    bool isConnected() const;
    int currentHeartRate() const;
    QString connectionStatus() const;
    bool isRecording() const;
    EcgDataModel* ecgDataModel() const;
    QString alertMessage() const;
    int alertLevel() const;

    // Invokable methods for QML
    Q_INVOKABLE void startConnection();
    Q_INVOKABLE void stopConnection();
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void exportData(const QString& filePath);
    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE QVariantList getAvailableDevices();

signals:
    void connectionStatusChanged();
    void heartRateChanged();
    void recordingStatusChanged();
    void alertTriggered();
    void dataExported(bool success, const QString& message);
    void newEcgData(double value, double timestamp);

private slots:
    void onNewEcgReading(double voltage, quint64 timestamp);
    void onConnectionStateChanged(bool connected);
    void onArrhythmiaDetected(const QString& type, int severity);
    void updateHeartRate();

private:
    void initializeDatabase();
    void saveEcgReading(double voltage, quint64 timestamp, int heartRate);
    void calculateHeartRate(const QList<double>& ecgData);

    EcgDataModel* m_ecgDataModel;
    BluetoothManager* m_bluetoothManager;
    ArrhythmiaDetector* m_arrhythmiaDetector;
    
    QSqlDatabase m_database;
    QTimer* m_heartRateTimer;
    
    bool m_isConnected;
    bool m_isRecording;
    int m_currentHeartRate;
    QString m_connectionStatus;
    QString m_alertMessage;
    int m_alertLevel;
    
    QList<double> m_recentEcgData;
    QList<quint64> m_recentTimestamps;
    quint64 m_lastHeartRateCalculation;
    
    static const int MAX_RECENT_SAMPLES = 500;
    static const int HEART_RATE_WINDOW_MS = 10000; // 10 seconds
};
