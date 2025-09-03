
#include <QAbstractListModel>
#include <QDateTime>
#include <QQmlEngine>
#include <QtQml>
#include <QtQml/qqmlregistration.h>

struct EcgReading {
    double voltage;
    quint64 timestamp;
    int heartRate;
    QDateTime dateTime;
};

class EcgDataModel : public QAbstractListModel
{
    Q_OBJECT
    // QML_ELEMENT

public:
    enum Roles {
        VoltageRole = Qt::UserRole + 1,
        TimestampRole,
        HeartRateRole,
        DateTimeRole,
        FormattedTimeRole
    };

    explicit EcgDataModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Model management
    Q_INVOKABLE void addReading(double voltage, quint64 timestamp, int heartRate = 0);
    Q_INVOKABLE void clearData();
    Q_INVOKABLE int getReadingCount() const;
    Q_INVOKABLE QVariantMap getReading(int index) const;
    Q_INVOKABLE QVariantList getRecentReadings(int count) const;

private:
    QList<EcgReading> m_readings;
    static const int MAX_STORED_READINGS = 10000;
};
