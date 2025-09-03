#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QQueue>
#include <QQmlEngine>
#include <QtQml>
#include <QtQml/qqmlregistration.h>

struct RRInterval {
    double interval; // in milliseconds
    quint64 timestamp;
};

class ArrhythmiaDetector : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
    
    Q_PROPERTY(bool isMonitoring READ isMonitoring NOTIFY monitoringChanged)
    Q_PROPERTY(QString currentRhythm READ currentRhythm NOTIFY rhythmChanged)
    Q_PROPERTY(double averageRRInterval READ averageRRInterval NOTIFY metricsChanged)
    Q_PROPERTY(double rrVariability READ rrVariability NOTIFY metricsChanged)

public:
    explicit ArrhythmiaDetector(QObject *parent = nullptr);

    // Property getters
    bool isMonitoring() const { return m_isMonitoring; }
    QString currentRhythm() const { return m_currentRhythm; }
    double averageRRInterval() const { return m_averageRRInterval; }
    double rrVariability() const { return m_rrVariability; }

    // Invokable methods
    Q_INVOKABLE void startMonitoring();
    Q_INVOKABLE void stopMonitoring();
    Q_INVOKABLE void resetAnalysis();

    // Called by HMController
    void processEcgSample(double voltage, quint64 timestamp);

signals:
    void monitoringChanged();
    void rhythmChanged();
    void metricsChanged();
    void arrhythmiaDetected(const QString &type, int severity);

private slots:
    void analyzeRhythm();

private:
    void detectRPeak(double voltage, quint64 timestamp);
    void calculateRRInterval(quint64 currentPeakTime);
    void updateMetrics();
    QString classifyRhythm();
    int calculateSeverity(const QString &arrhythmiaType);
    
    // R-peak detection
    QQueue<double> m_voltageBuffer;
    QQueue<quint64> m_timestampBuffer;
    double m_peakThreshold;
    quint64 m_lastPeakTime;
    bool m_inRefractoryPeriod;
    
    // RR interval analysis
    QQueue<RRInterval> m_rrIntervals;
    double m_averageRRInterval;
    double m_rrVariability;
    
    // Rhythm classification
    QString m_currentRhythm;
    QTimer *m_analysisTimer;
    
    bool m_isMonitoring;
    
    static constexpr int BUFFER_SIZE = 50; // Number of samples to keep
    static constexpr int MAX_RR_INTERVALS = 20; // For analysis
    static constexpr int REFRACTORY_PERIOD_MS = 200; // Minimum time between R-peaks
    static constexpr double MIN_PEAK_HEIGHT = 0.5; // Minimum voltage for R-peak
};

Q_DECLARE_METATYPE(ArrhythmiaDetector)
