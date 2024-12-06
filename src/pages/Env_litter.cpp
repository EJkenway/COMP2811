#include "Env_litter.hpp"
#include "../utils/utils.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

QChartView* createFirstPieChart(Records litter, Records sewage, Records residue) {

    QPieSeries* series = new QPieSeries();
    series->append("Other Litter (Including Plastics)", litter.size());
    series->append("Sewage Debris", sewage.size());
    series->append("Tarry Residues", residue.size());

    qreal total = 0;
    for (const QPieSlice* slice : series->slices()) {
        total += slice->value();
    }

    QPieSlice* slice = series->slices().at(0);
    slice->setExploded(true);
    slice->setBrush(Qt::blue);
    for (QPieSlice* slice : series->slices()) {
        qreal percentage = (slice->value() / total) * 100;
        slice->setLabel(QString("%1: %2%").arg(slice->label()).arg(percentage, 0, 'f', 1));
        slice->setLabelVisible(true);
    }

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Summary of Physical Pollutants");
    chart->legend()->setAlignment(Qt::AlignCenter);

    QChartView* chartView = new QChartView(chart);
    return chartView;
}

QChartView* createOtherPieCharts(Records records, QString name) {
    int numberof0s = 0, numberof1s = 0, numberof2s = 0, numberof3s = 0;
    QPieSeries* series = new QPieSeries();
    for(const auto& record : records) {
        switch(record["result"].toInt()) {
            case 0:
                numberof0s++;
            case 1:
                numberof1s++;
            case 2:
                numberof2s++;
            default:
                numberof3s++;
        }
    }
    series->append("0", numberof0s);
    series->append("1", numberof1s);
    series->append("2", numberof2s);
    series->append("3", numberof3s);
    qreal total = 0;
    for (const QPieSlice* slice : series->slices()) {
        total += slice->value();
    }
    QPieSlice* slice = series->slices().at(0);
    slice->setBrush(Qt::blue);
    for (QPieSlice* slice : series->slices()) {
        qreal percentage = (slice->value() / total) * 100;
        slice->setLabel(QString("%1: %2%").arg(slice->label()).arg(percentage, 0, 'f', 1));
        slice->setLabelVisible(true);
    }
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(name);
    chart->legend()->setAlignment(Qt::AlignCenter);
    QChartView* chartView = new QChartView(chart);
    return chartView;
}

EnvPage::EnvPage(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel("Welcome to the Environment Litter Page", this);
    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);
    // layout->centralWidget = new QWidget(this);
    // this->setCentralWidget(layout->centralWidget);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Enter keyword to search");

    searchButton = new QPushButton("Search", this);

    resultLabel = new QLabel("Results will be shown here", this);
    resultLabel->setWordWrap(true);

    Records litter = searchDatabase("determinand_label", "BWP - O.L.");
    Records sewage = searchDatabase("determinand_label", "SewageDebris");
    Records residue = searchDatabase("determinand_label", "TarryResidus");

    layout->addWidget(searchBox);
    layout->addWidget(searchButton);
    layout->addWidget(resultLabel);

    // QGridLayout* gridLayout = new QGridLayout(layout->centralWidget);
    // gridLayout->addWidget(createFirstPieChart(litter, sewage, residue), 0, 0);
    // gridLayout->addWidget(createOtherPieCharts(litter, "BWP - Other Litter (Including Plastics)"), 0, 1);
    // gridLayout->addWidget(createOtherPieCharts(sewage, "Sewage Debris"), 1 , 0);
    // gridLayout->addWidget(createOtherPieCharts(residue, "Tarry Residues"), 1, 1);
    // layout->addWidget(gridLayout);
    layout->addWidget(createFirstPieChart(litter, sewage, residue));
    layout->addWidget(createOtherPieCharts(litter, "BWP - Other Litter (Including Plastics)"));
    layout->addWidget(createOtherPieCharts(sewage, "Sewage Debris"));
    layout->addWidget(createOtherPieCharts(residue, "Tarry Residues"));
    setLayout(layout);

    connect(searchButton, &QPushButton::clicked, this, &EnvPage::onSearchClicked);
}

void EnvPage::onSearchClicked() {
    QString keyword = searchBox->text().trimmed();
    QString compliant = "No";

    if (keyword.isEmpty()) {
        resultLabel->setText("Please enter a keyword to search.");
        return;
    }

    Records results = searchDatabase("determinand_label", keyword);

    //for(auto& record : results) {
    //    if(results["sample.isComplianceSample"] == "TRUE") {
    //        compliant = "Yes";
    //   }
    //    else {
    //        compliant = "No";
    //    }
    //}

    if (results.isEmpty()) {
        resultLabel->setText("No matching records found.");
        return;
    }

    auto [minResult, maxResult] = findResultRange(results);

    resultLabel->setText(QString(
        "Found %1 matching records.\n"
        "Max Result: %2\n"
        "Min Result: %3\n"
        "Compliant?: %4"
    ).arg(results.size()).arg(maxResult).arg(minResult));
}