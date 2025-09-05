#include "ecgdatamodel.h"
#include <QDebug>

EcgDataModel::EcgDataModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int EcgDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_readings.count();
}

QVariant EcgDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_readings.count())
        return QVariant();

    const EcgReading &reading = m_readings.at(index.row());

    switch (role) {
    case VoltageRole:
        return reading.voltage;
    case TimestampRole:
        return static_cast<qint64>(reading.timestamp);
    case HeartRateRole:
        return reading.heartRate;
    case DateTimeRole:
        return reading.dateTime;
    case FormattedTimeRole:
        return reading.dateTime.toString("hh:mm:ss");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> EcgDataModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[VoltageRole] = "voltage";
    roles[TimestampRole] = "timestamp";
    roles[HeartRateRole] = "heartRate";
    roles[DateTimeRole] = "dateTime";
    roles[FormattedTimeRole] = "formattedTime";
    return roles;
}

void EcgDataModel::addReading(double voltage, quint64 timestamp, int heartRate)
{
    // Manage memory by removing old readings
    if (m_readings.size() >= MAX_STORED_READINGS) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_readings.removeFirst();
        endRemoveRows();
    }

    beginInsertRows(QModelIndex(), m_readings.count(), m_readings.count());
    
    EcgReading reading;
    reading.voltage = voltage;
    reading.timestamp = timestamp;
    reading.heartRate = heartRate;
    reading.dateTime = QDateTime::fromMSecsSinceEpoch(timestamp);
    
    m_readings.append(reading);
    endInsertRows();
}

void EcgDataModel::clearData()
{
    beginResetModel();
    m_readings.clear();
    endResetModel();
}

int EcgDataModel::getReadingCount() const
{
    return m_readings.count();
}

QVariantMap EcgDataModel::getReading(int index) const
{
    QVariantMap reading;
    if (index >= 0 && index < m_readings.count()) {
        const EcgReading &r = m_readings.at(index);
        reading["voltage"] = r.voltage;
        reading["timestamp"] = static_cast<qint64>(r.timestamp);
        reading["heartRate"] = r.heartRate;
        reading["dateTime"] = r.dateTime;
        reading["formattedTime"] = r.dateTime.toString("hh:mm:ss");
    }
    return reading;
}

QVariantList EcgDataModel::getRecentReadings(int count) const
{
    QVariantList recent;
    int start = qMax(0, m_readings.count() - count);
    
    for (int i = start; i < m_readings.count(); ++i) {
        recent.append(getReading(i));
    }
    
    return recent;
}