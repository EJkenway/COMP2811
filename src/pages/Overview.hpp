#ifndef OVERVIEWPAGE_HPP
#define OVERVIEWPAGE_HPP

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>

#include "../utils/utils.hpp"

class OverviewPage : public QWidget {
    Q_OBJECT

public:
    explicit OverviewPage(QWidget* parent = nullptr);


private slots:
    void onSearchClicked();

private:
    QLineEdit* searchBox;
    QPushButton* searchButton;
    QLabel* resultLabel;

    QChartView* chartView;
    QLineSeries* lineSeries;
    QList<QPair<QPointF, Record>> pointToRecordList;
    QLabel* customToolTip = nullptr;

    void updateChart(const Records& records, const QString& keyword);
    void onPointHovered(const QPointF& point, bool state);
};

#endif