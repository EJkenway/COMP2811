#include "Fluoride.hpp"
#include "../utils/database_utils.hpp"
#include <QVBoxLayout>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlRecord>

FluoridePage::FluoridePage(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Fluorinated Compounds - PFAS Monitoring", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Buttons for map and time-series visualization
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    mapButton = new QPushButton("Show Map", this);
    timeSeriesButton = new QPushButton("Show Time-Series", this);
    mapButton->setEnabled(false);
    timeSeriesButton->setEnabled(false);
    buttonLayout->addWidget(mapButton);
    buttonLayout->addWidget(timeSeriesButton);
    mainLayout->addLayout(buttonLayout);

    // Location Selector
    QLabel* locationLabel = new QLabel("Select Location:", this);
    locationLabel->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(locationLabel);

    locationSelector = new QComboBox(this);
    mainLayout->addWidget(locationSelector);

    // Placeholder for custom chart visualization
    chartLabel = new QLabel(this);
    chartLabel->setMinimumHeight(150);
    mainLayout->addWidget(chartLabel);

    // Additional information about PFAS
    dataInfoLabel = new QLabel(this);
    dataInfoLabel->setText("PFAS are persistent chemicals found in water bodies.\n"
                           "They can pose health risks above certain concentrations.");
    mainLayout->addWidget(dataInfoLabel);

    // Connect button signals to slots
    connect(mapButton, &QPushButton::clicked, this, &FluoridePage::showMapVisualization);
    connect(timeSeriesButton, &QPushButton::clicked, this, &FluoridePage::showTimeSeriesVisualization);
    connect(locationSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FluoridePage::onLocationSelected);
    setLocations();
}

void FluoridePage::showMapVisualization() {
    // Placeholder implementation for map visualization
    QMessageBox::information(this, "Map Visualization", "Map view is under development.");
}

void FluoridePage::setLocations() {
    // use SQL_Database to get the locations
    std::string sql = "SELECT DISTINCT sampling_point_label FROM pollution";
    Records records = SQL_Database(sql);
    // print the records
    QStringList locations;
    for (const auto& record : records) {
        QString location = record.value("sampling_point_label").toString();
        if (!locations.contains(location)) {
            locations.append(location);
        }
    }
    locationSelector->addItems(locations);
}

void FluoridePage::onLocationSelected(int index) {
    if (index >= 0) {
        // Enable the visualization buttons once a location is selected
        mapButton->setEnabled(true);
        timeSeriesButton->setEnabled(true);

        // Update the graph for the selected location
        showTimeSeriesVisualization();
    }
}

void FluoridePage::showTimeSeriesVisualization() {
    // Ensure a location is selected before drawing the graph
    QString selectedLocation = locationSelector->currentText();
    if (selectedLocation.isEmpty()) {
        qWarning() << "No location selected!";
        return;
    }

    // Use QPainter to draw a simple time-series visualization on chartLabel
    QPixmap pixmap(chartLabel->size());
    pixmap.fill(Qt::white);
    chartLabel->setPixmap(pixmap);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // set the min and max as reference for the y-axis
    double yMin = 10;
    double yMax = 130;
    // set the min and max as reference for the x-axis
    double xMin = 20;
    double xMax = 560;
    qDebug() << "Selected location:" << selectedLocation.toStdString();
    // Function to call the db and pass a sql command
    std::string sql = "SELECT * FROM pollution WHERE determinand_definition LIKE '%PF%' OR "
                  "determinand_definition LIKE '%fluoro%' OR "
                  "determinand_definition LIKE '%fluoride%' OR "
                  "determinand_definition LIKE '%FTSA%' OR "
                  "determinand_definition LIKE '%FTCA%' OR "
                  "determinand_definition LIKE '%PFOS%' OR "
                  "determinand_definition LIKE '%PFOA%' OR "
                  "determinand_definition LIKE '%PFHxS%' OR "
                  "determinand_definition LIKE '%Cl-PF%' AND "
                  "sampling_point_label = '" + selectedLocation.toStdString() + "';";

    Records FluorideRecords = SQL_Database(sql);

    // Check if the results are empty
    if (FluorideRecords.isEmpty()) {
        qWarning() << "No FluorideRecords found!";
        return;
    }


    // Make a pair of Qvector from FilterRecords of sample_date_time and result
    auto [sampleDateTimes, results] = FilterRecords(FluorideRecords);

    // Find the min and max of the results and sample_date_time
    auto [minMaxDateTime, minMaxResult] = findMinMax(sampleDateTimes, results);
    QDateTime minDateTime = minMaxDateTime[0];
    QDateTime maxDateTime = minMaxDateTime[1];
    double minResult = minMaxResult[0];
    double maxResult = minMaxResult[1];
    // Make a proportionality relationship between the min and max result and the y-axis
    double yScale = (yMax - yMin) / (maxResult - minResult);
    // Make a proportionality relationship between the min and max date time and the x-axis
    double xScale = (xMax - xMin) / (minDateTime.secsTo(maxDateTime) / 60);
    // Make points based on the sample_date_time and result
    QVector<QPoint> points = makePoints(sampleDateTimes, results);

    // Draw axes
    // Lets set 0 to coordanate x=20, y=130
    painter.setPen(Qt::black);
    painter.drawLine(xMin, yMax, xMax, yMax);
    painter.drawLine(xMin, yMin, xMin, yMax);

    // Threshold lines
    double yRed = yMax - 0.3 * yScale;
    double yYellow = yMax - 0.1 * yScale;


    // Draw data points
    for (const auto& point : points) {
        // Change the color of the data points based on the different thresholds
        if (point.y() < yRed) {
            painter.setBrush(Qt::red);
        } else if (point.y() < yYellow) {
            painter.setBrush(Qt::yellow);
        } else {
            painter.setBrush(Qt::green);
        }
        painter.drawEllipse(point, 3, 3);
    }

    chartLabel->setPixmap(pixmap);

}

// Function to make Records into Points
QVector<QPoint> makePoints(const Records& records) {
    QVector<QPoint> points;
    for (const auto& record : records) {
        if (record.contains("sample_date_time") && record.contains("result")) {
            QDateTime sampleDateTime = record.value("sample_date_time").toDateTime();
            double result = record.value("result").toDouble();
            points.append(QPoint(sampleDateTime.toSecsSinceEpoch(), result));
        }
    }
    return points;
}

// Function Max and Min of any QVector QDateTime and double which takes qvector of qvector double and qvector QDateTime
std::pair<QVector<QDateTime>, QVector<double>> findMinMax(const QVector<QDateTime> dateTimes, const QVector<double> results) {

    QDateTime minDateTime = dateTimes[0];
    QDateTime maxDateTime = dateTimes[0];

    double minResult = results[0];
    double maxResult = results[0];

    for (const auto& dateTime : dateTimes) {
        if (dateTime < minDateTime) {
            minDateTime = dateTime;
        }
        if (dateTime > maxDateTime) {
            maxDateTime = dateTime;
        }
    }
    for (const auto& result : results) {
        if (result < minResult) {
            minResult = result;
        }
        if (result > maxResult) {
            maxResult = result;
        }
    }
    QVector<QDateTime> minMaxDateTime = {minDateTime, maxDateTime};
    QVector<double> minMaxResult = {minResult, maxResult};
    return {minMaxDateTime, minMaxResult};
}

// Function to take Records and return a QVector of Qvector of Qvectors SmapleDateTime and Result
std::pair<QVector<QDateTime>, QVector<double>> FilterRecords(const Records& records) {
    QVector<QDateTime> dateTimes;
    QVector<double> results;

    for (const auto& record : records) {
        if (record.contains("sample_date_time") && record.contains("result")) {
            QDateTime sampleDateTime = record.value("sample_date_time").toDateTime();
            double result = record.value("result").toDouble();
            dateTimes.append(sampleDateTime);
            results.append(result);
        }
    }
    return {dateTimes, results};
}

// Function to take Qvector of QDateTime and double and Max and return a QVector of QPoint
QVector<QPoint> makePoints(const QVector<QDateTime>& dateTimes, const QVector<double>& results) {
    QVector<QPoint> points;
    // Max and Min of the dateTimes and results
    auto [minMaxDateTime, minMaxResult] = findMinMax(dateTimes, results);
    QDateTime minDateTime = minMaxDateTime[0];
    QDateTime maxDateTime = minMaxDateTime[1];
    double minResult = minMaxResult[0];
    double maxResult = minMaxResult[1];
    // set the min and max as reference for the y-axis
    double yMin = 10;
    double yMax = 130;
    // set the min and max as reference for the x-axis
    double xMin = 20;
    double xMax = 560;
    // Make a proportionality relationship between the min and max result and the y-axis
    double yRange = yMax - yMin;
    double resultRange = maxResult - minResult;
    double yScale = yRange / resultRange;
    // Make a proportionality relationship between the min and max date time and the x-axis
    double xRange = xMax - xMin;
    double dateRange = minDateTime.secsTo(maxDateTime) / 60;
    // Print dateRange
    double xScale = xRange / dateRange;

    for (int i = 0; i < results.size(); ++i) {
        double x = xMin + xScale * (minDateTime.secsTo(dateTimes[i]) / 60);
        double y = yMax - yScale * (results[i] - minResult);
        points.append(QPoint(x, y));
    }
    return points;
}
