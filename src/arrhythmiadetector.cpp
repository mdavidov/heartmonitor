#include "arrhythmiadetector.h"
#include "arrhythmiadetector.moc"
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>

ArrhythmiaDetector::ArrhythmiaDetector(QObject *parent)
    : QObject(parent)
    , m_peakThreshold(MIN_PEAK_HEIGHT)
    , m_lastPeakTime(0)
    , m_inRefractoryPeriod(false)
    , m_averageRRInterval(0.0)
    , m_rrVariability(0.0)
    , m_currentRhythm("Normal Sinus Rhythm")
    , m_isMonitoring(false)
{
    qRegisterMetaType<ArrhythmiaDetector>("ArrhythmiaDetector");
    m_analysisTimer = new QTimer(this);
    connect(m_analysisTimer, &QTimer::timeout, this, &ArrhythmiaDetector::analyzeRhythm);
    m_analysisTimer->setInterval(5000); // Analyze every 5 seconds
}

void ArrhythmiaDetector::startMonitoring()
{
    m_isMonitoring = true;
    m_analysisTimer->start();
    emit monitoringChanged();
    qDebug() << "Arrhythmia monitoring started";
}

void ArrhythmiaDetector::stopMonitoring()
{
    m_isMonitoring = false;
    m_analysisTimer->stop();
    emit monitoringChanged();
    qDebug() << "Arrhythmia monitoring stopped";
}

void ArrhythmiaDetector::resetAnalysis()
{
    m_voltageBuffer.clear();
    m_timestampBuffer.clear();
    m_rrIntervals.clear();
    m_lastPeakTime = 0;
    m_averageRRInterval = 0.0;
    m_rrVariability = 0.0;
    m_currentRhythm = "Normal Sinus Rhythm";
    
    emit rhythmChanged();
    emit metricsChanged();
}

void ArrhythmiaDetector::processEcgSample(double voltage, quint64 timestamp)
{
    if (!m_isMonitoring) {
        return;
    }
    
    // Add to buffer
    m_voltageBuffer.enqueue(voltage);
    m_timestampBuffer.enqueue(timestamp);
    
    // Keep buffer size manageable
    while (m_voltageBuffer.size() > BUFFER_SIZE) {
        m_voltageBuffer.dequeue();
        m_timestampBuffer.dequeue();
    }
    
    // Detect R-peaks
    detectRPeak(voltage, timestamp);
}

void ArrhythmiaDetector::detectRPeak(double voltage, quint64 timestamp)
{
    // Simple R-peak detection algorithm
    if (m_voltageBuffer.size() < 3) {
        return;
    }
    
    // Check if we're in refractory period
    if (m_inRefractoryPeriod && (timestamp - m_lastPeakTime) < REFRACTORY_PERIOD_MS) {
        return;
    }
    m_inRefractoryPeriod = false;
    
    // Check if current sample is a local maximum above threshold
    int bufferSize = m_voltageBuffer.size();
    if (bufferSize >= 3) {
        double prev = m_voltageBuffer.at(bufferSize - 2);
        double current = m_voltageBuffer.at(bufferSize - 1);
        double next = voltage;
        
        if (current > prev && current > next && current > m_peakThreshold) {
            // Found R-peak
            quint64 peakTime = m_timestampBuffer.at(bufferSize - 1);
            calculateRRInterval(peakTime);
            
            m_lastPeakTime = peakTime;
            m_inRefractoryPeriod = true;
        }
    }
}

void ArrhythmiaDetector::calculateRRInterval(quint64 currentPeakTime)
{
    if (m_lastPeakTime > 0) {
        double interval = currentPeakTime - m_lastPeakTime;
        
        // Validate interval (should be between 300ms and 2000ms for normal heart rates)
        if (interval >= 300 && interval <= 2000) {
            RRInterval rrInterval;
            rrInterval.interval = interval;
            rrInterval.timestamp = currentPeakTime;
            
            m_rrIntervals.enqueue(rrInterval);
            
            // Keep only recent intervals
            while (m_rrIntervals.size() > MAX_RR_INTERVALS) {
                m_rrIntervals.dequeue();
            }
            
            updateMetrics();
        }
    }
}

void ArrhythmiaDetector::updateMetrics()
{
    if (m_rrIntervals.isEmpty()) {
        return;
    }
    
    // Calculate average RR interval
    double sum = 0.0;
    for (const RRInterval &interval : m_rrIntervals) {
        sum += interval.interval;
    }
    m_averageRRInterval = sum / m_rrIntervals.size();
    
    // Calculate RR variability (RMSSD - root mean square of successive differences)
    if (m_rrIntervals.size() > 1) {
        double sumSquaredDifferences = 0.0;
        for (int i = 1; i < m_rrIntervals.size(); ++i) {
            double diff = m_rrIntervals.at(i).interval - m_rrIntervals.at(i-1).interval;
            sumSquaredDifferences += diff * diff;
        }
        m_rrVariability = qSqrt(sumSquaredDifferences / (m_rrIntervals.size() - 1));
    }
    
    emit metricsChanged();
}

void ArrhythmiaDetector::analyzeRhythm()
{
    if (m_rrIntervals.size() < 5) {
        return; // Need more data
    }
    
    QString newRhythm = classifyRhythm();
    
    if (newRhythm != m_currentRhythm) {
        m_currentRhythm = newRhythm;
        emit rhythmChanged();
        
        // Check if this is an arrhythmia
        if (newRhythm != "Normal Sinus Rhythm") {
            int severity = calculateSeverity(newRhythm);
            emit arrhythmiaDetected(newRhythm, severity);
        }
    }
}

QString ArrhythmiaDetector::classifyRhythm()
{
    if (m_rrIntervals.isEmpty()) {
        return "No Data";
    }
    
    // Calculate heart rate from average RR interval
    double avgHeartRate = 60000.0 / m_averageRRInterval; // BPM
    
    // Calculate coefficient of variation for rhythm regularity
    double cv = (m_rrVariability / m_averageRRInterval) * 100.0;
    
    // Simple rhythm classification
    if (avgHeartRate < 60) {
        if (cv > 15) {
            return "Bradyarrhythmia";
        } else {
            return "Sinus Bradycardia";
        }
    } else if (avgHeartRate > 100) {
        if (cv > 15) {
            return "Tachyarrhythmia";
        } else {
            return "Sinus Tachycardia";
        }
    } else {
        // Normal rate (60-100 BPM)
        if (cv > 20) {
            return "Atrial Fibrillation"; // Very irregular
        } else if (cv > 15) {
            return "Irregular Rhythm";
        } else {
            return "Normal Sinus Rhythm";
        }
    }
}

int ArrhythmiaDetector::calculateSeverity(const QString &arrhythmiaType)
{
    // Severity levels: 1=Low, 2=Medium, 3=High, 4=Critical
    
    if (arrhythmiaType == "Sinus Bradycardia" || arrhythmiaType == "Sinus Tachycardia") {
        return 1; // Low - usually benign
    } else if (arrhythmiaType == "Irregular Rhythm") {
        return 2; // Medium - needs attention
    } else if (arrhythmiaType == "Bradyarrhythmia" || arrhythmiaType == "Tachyarrhythmia") {
        return 3; // High - concerning
    } else if (arrhythmiaType == "Atrial Fibrillation") {
        return 3; // High - stroke risk
    }
    
    return 2; // Default medium severity
}