#include "Overview.hpp"
#include "../utils/utils.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QChart>
#include <QChartView>
#include <QLineSeries>

OverviewPage::OverviewPage(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel("Welcome to the Overview Page", this);
    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Enter keyword to search");

    searchButton = new QPushButton("Search", this);

    resultLabel = new QLabel("Results will be shown here", this);
    resultLabel->setWordWrap(true);

    layout->addWidget(searchBox);
    layout->addWidget(searchButton);
    layout->addWidget(resultLabel);

    lineSeries = new QLineSeries();
    QChart* chart = new QChart();
    chart->addSeries(lineSeries);
    chart->setTitle("Result Values");
    chart->createDefaultAxes();

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(chartView);
    chartView->setVisible(false);

    customToolTip = new QLabel(this);
    customToolTip->setStyleSheet("background-color: #FFF; border: 1px solid #CCC; padding: 5px;");
    customToolTip->setWindowFlags(Qt::ToolTip);
    customToolTip->hide();

    setLayout(layout);

    connect(searchButton, &QPushButton::clicked, this, &OverviewPage::onSearchClicked);
    connect(lineSeries, &QLineSeries::hovered, this, &OverviewPage::onPointHovered);
}


void OverviewPage::onSearchClicked() {
    QString keyword = searchBox->text().trimmed();

    if (keyword.isEmpty()) {
        resultLabel->setText("Please enter a keyword to search.");
        return;
    }

    Records results = searchDatabase("determinand_label", keyword);

    if (results.isEmpty()) {
        resultLabel->setText("No matching records found.");
        return;
    }

    auto [minResult, maxResult] = findResultRange(results);

    resultLabel->setText(QString(
        "Found %1 matching records.\n"
        "Max Result: %2\n"
        "Min Result: %3"
    ).arg(results.size()).arg(maxResult).arg(minResult));

    updateChart(results, keyword);
}


void OverviewPage::updateChart(const Records& records, const QString& keyword) {
    pointToRecordList.clear();
    lineSeries->clear();
    qDebug() << "Updating chart with records:";

    if (records.isEmpty()) {
        chartView->setVisible(false);
        return;
    }

    chartView->setVisible(true);

    double minResult;
    double maxResult;
    qint64 minTimestamp = std::numeric_limits<qint64>::max();
    qint64 maxTimestamp = std::numeric_limits<qint64>::lowest();

    Records sortedRecords = sortRecordsByTime(records);

    for (const auto& record : sortedRecords) {
        if (record.contains("result") && record.contains("sample_date_time")) {
            double value = record["result"].toDouble();
            QString timeString = record["sample_date_time"].toString();

            QDateTime timestamp = QDateTime::fromString(timeString, Qt::ISODate);
            if (!timestamp.isValid()) {
                qWarning() << "Invalid timestamp:" << timeString;
                continue;
            }
            
            qint64 xValue = timestamp.toMSecsSinceEpoch();

            QPointF point(xValue, value);
            lineSeries->append(point);
            minTimestamp = std::min(minTimestamp, xValue);
            maxTimestamp = std::max(maxTimestamp, xValue);
            pointToRecordList.append({point, record});
        }
    }
    std::tie(minResult, maxResult) = findResultRange(records);

    auto xAxis = new QDateTimeAxis();
    auto yAxis = new QValueAxis();

    QChart* chart = chartView->chart();
    for (QAbstractAxis* axis : chart->axes()) {
        chart->removeAxis(axis);
    }

    xAxis->setFormat("yyyy-MM-dd");
    xAxis->setRange(QDateTime::fromMSecsSinceEpoch(minTimestamp),
                    QDateTime::fromMSecsSinceEpoch(maxTimestamp));
    xAxis->setTitleText("Date");

    QString yAxisTitle;
    yAxis->setRange(0, maxResult);
    yAxisTitle = records[0]["determinand_unit_label"].toString();
    yAxis->setTitleText(QString("Unit: %1").arg(yAxisTitle));

    chart->addAxis(xAxis, Qt::AlignBottom);
    chart->addAxis(yAxis, Qt::AlignLeft);

    lineSeries->attachAxis(xAxis);
    lineSeries->attachAxis(yAxis);

    chart->setTitle(keyword);
    chart->legend()->hide();
    chartView->setMinimumHeight(400);
    chartView->update();
}

void OverviewPage::onPointHovered(const QPointF& point, bool state) {
    if (state) {
        const double threshold = 0.002;
        for (const auto& pair : pointToRecordList) {
            qDebug() << "the point: " << point;
            qDebug() << "Current: " << pair.first;
            const QPointF& storedPoint = pair.first;
            double distance = std::abs(point.y() - storedPoint.y());
            qDebug() << "Distance: " << distance;
            if (distance <= threshold) {
                qDebug() << "the record: " << pair.second;
                const Record& record = pair.second;
                QString materialType = record["sampled_material_type"].toString();
                QString isCompliance = record["is_compliance_sample"].toBool() ? "Yes" : "No";
                QString purpose = record["sample_purpose_label"].toString();
                QString determinandLabel = record["determinand_unit_label"].toString();
                QString timestamp = QDateTime::fromMSecsSinceEpoch(storedPoint.x()).toString("yyyy-MM-dd hh:mm:ss");
                double value = storedPoint.y();

                QString tooltipText = QString(
                                            "Value: %1\n"
                                            "Date: %2\n"
                                            "Determinand: %3\n"
                                            "Material Type: %4\n"
                                            "Compliance: %5\n"
                                            "Purpose: %6"
                                        ).arg(value)
                                        .arg(timestamp)
                                        .arg(determinandLabel)
                                        .arg(materialType)
                                        .arg(isCompliance)
                                        .arg(purpose);
                                customToolTip->setText(tooltipText);
                customToolTip->adjustSize();
                customToolTip->move(QCursor::pos() + QPoint(10, 10));
                customToolTip->show();
                break;
            }
        }
    } else {
        customToolTip->hide();
    }

}
