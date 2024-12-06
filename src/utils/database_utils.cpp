#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlRecord>
#include "csv.hpp"
#include "database_utils.hpp"
#include <regex>

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
// Make a SQL query to the database
Records SQL_Database(std::string sql) {
    Records results;
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "Database is not open or valid!";
        return results;
    }
    QSqlQuery query(QString::fromStdString(sql));
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
// Make a SQL query to the database
Records SQL_Database(std::string sql) {
    Records results;
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "Database is not open or valid!";
        return results;
    }
    QSqlQuery query(QString::fromStdString(sql));
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


// Load data from a CSV file into the database
void loadDataIntoDatabase(const QString& csvFilePath) {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "Database is not open or valid!";
        return;
    }

    csv::CSVReader reader(csvFilePath.toStdString());
    auto file = reader.begin();
    if (file == reader.end()) {
        qWarning() << "CSV file is empty!";
        return;
    }
    // empty the pollution table
    QSqlQuery query("DELETE FROM pollution");
    if (!query.exec()) {
        qWarning() << "Failed to delete pollution table:" << query.lastError().text();
        return;
    }
    // get the column names from the first row
    std::vector<std::string> columnNames = file->get_col_names();
    ++file; // Move to the next row to start reading the data

    // Prepare the INSERT statement dynamically for the matching columns
    QString insertQuery = "INSERT INTO pollution ("
                            "id,"
                            "sampling_point,"
                            "sampling_point_notation,"
                            "sampling_point_label,"
                            "sample_date_time,"
                            "determinand_label,"
                            "determinand_definition,"
                            "determinand_notation,"
                            "result_qualifier,"
                            "result,"
                            "coded_result_interpretation,"
                            "determinand_unit_label,"
                            "sampled_material_type,"
                            "is_compliance_sample,"
                            "sample_purpose_label,"
                            "sampling_point_easting,"
                            "sampling_point_northing)";

    QString valuesPlaceholder = "VALUES ";

    for (; file != reader.end(); ++file) {
        // get every element from each row of file
        size_t length = file->size();
        valuesPlaceholder += "(";
        for (size_t i = 0; i < length; ++i) {
            std::string value = (*file)[i].get<>();
            std::regex numericRegex("^-?\\d*\\.?\\d+$");
            // Check if the value is a string and if it is add single quotes execption to true and false
            if (value != "true" && value != "false" && !std::regex_match(value, numericRegex)){
                valuesPlaceholder += "'" + value + "'";
            }
            else{
                valuesPlaceholder += value;
            }
            if (i+1 != length){
                valuesPlaceholder += ", ";
            }
        }
        // for the last row add a closing bracket with semi colon ortherwise add a comma
        // hard copy the file variable to not affect the original file variable
        auto copy_file = file;
        copy_file++;
        if (copy_file == reader.end()){
            valuesPlaceholder += ");";
        }
        else{
            valuesPlaceholder += "), ";
        }
    }
    // Combine the insertQuery and valuesPlaceholder
    insertQuery += valuesPlaceholder;
    // Print the insert query
    qDebug() << insertQuery;
    // Execute the insert query
    QSqlQuery insertQueryExec(db);
    if (!insertQueryExec.exec(insertQuery)) {
        qWarning() << "Failed to execute insert query:" << insertQueryExec.lastError().text();
    } else {
        qDebug() << "Data inserted successfully!";
    }

    printTableColumns("pollution");
}
// Print the columns of a table
void printTableColumns(const QString& tableName) {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "Database is not open or valid!";
        return;
    }

    QSqlQuery query(db);
    QString pragmaQuery = QString("PRAGMA table_info(%1);").arg(tableName);
    if (!query.exec(pragmaQuery)) {
        qWarning() << "Failed to retrieve table info:" << query.lastError().text();
        return;
    }

    qDebug() << "Columns in table" << tableName << ":";
    while (query.next()) {
        QString columnName = query.value(1).toString(); // The column name is in the second position (index 1)
        qDebug() << columnName;
    }

    // Print the rows of the table
    QString selectQuery = QString("SELECT * FROM %1;").arg(tableName);
    if (!query.exec(selectQuery)) {
        qWarning() << "Failed to retrieve table rows:" << query.lastError().text();
        return;
    }

    qDebug() << "Rows in table" << tableName << ":";
    int columnCount = query.record().count();
    while (query.next()) {
        QStringList rowValues;
        for (int i = 0; i < columnCount; ++i) {
            rowValues << query.value(i).toString();
        }
        qDebug() << rowValues.join(", ");
    }
}
