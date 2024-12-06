#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QSqlRecord>
#include <QDateTime>

#include "database_utils.hpp"

void setupDatabase() {
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        QSqlDatabase::removeDatabase("qt_sql_default_connection");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("data.db");

    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
    } else {
        qDebug() << "Database connection successful!";
    }
}

Records searchDatabase(const QString& attribute, const QString& keyword) {
    Records results;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "Database is not open or valid!";
        return results;
    }

    QString queryStr = QString("SELECT * FROM pollution WHERE %1 = :keyword").arg(attribute);
    QSqlQuery query;
    query.prepare(queryStr);
    query.bindValue(":keyword", keyword);

    if (!query.exec()) {
        qWarning() << "Database query failed:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        Record record;
        for (int i = 0; i < query.record().count(); ++i) {
            QString columnName = query.record().fieldName(i);
            record[columnName] = query.value(i);
        }
        results.append(record);
    }

    return results;
}

std::pair<double, double> findResultRange(const Records& records) {
    double minResult = std::numeric_limits<double>::max();
    double maxResult = std::numeric_limits<double>::lowest();

    for (const auto& record : records) {
        if (record.contains("result") && record["result"].canConvert<double>()) {
            double value = record["result"].toDouble();
            if (value < minResult) minResult = value;
            if (value > maxResult) maxResult = value;
        } else {
            qWarning() << "Record missing 'result' attribute or invalid type:" << record;
        }
    }

    return {minResult, maxResult};
}

Records sortRecordsByTime(const Records& records) {
    Records sortedRecords = records;

    std::stable_sort(sortedRecords.begin(), sortedRecords.end(), [](const Record& a, const Record& b) {
        QDateTime timeA = QDateTime::fromString(a["sample_date_time"].toString(), Qt::ISODate);
        QDateTime timeB = QDateTime::fromString(b["sample_date_time"].toString(), Qt::ISODate);

        if (!timeA.isValid() || !timeB.isValid()) {
            qWarning() << "Invalid timestamp in record comparison:";
            return false;
        }

        return timeA < timeB;
    });

    return sortedRecords;
}