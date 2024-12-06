#ifndef DATABASEUTILS_HPP
#define DATABASEUTILS_HPP

#include <QString>
#include <QVariant>
#include <QList>
#include <QMap>

using Record = QMap<QString, QVariant>;
using Records = QList<Record>;

void setupDatabase();
Records searchDatabase(const QString& attribute, const QString& keyword);
std::pair<double, double> findResultRange(const Records& records);

void loadDataIntoDatabase(const QString& csvFilePath);
void printTableColumns(const QString& tableName);
Records SQL_Database(std::string sql);

#endif
