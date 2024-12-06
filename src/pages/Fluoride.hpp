#ifndef FLUORIDEPAGE_HPP
#define FLUORIDEPAGE_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

class FluoridePage : public QWidget {
    Q_OBJECT

public:
    explicit FluoridePage(QWidget* parent = nullptr);

private slots:
    // Slot for functions
    void showMapVisualization();
    void showTimeSeriesVisualization();
    void setLocations();
    void onLocationSelected(int index);


private:
    QLabel* complianceIndicator;
    QPushButton* mapButton;
    QPushButton* timeSeriesButton;
    QLabel* chartLabel;
    QLabel* dataInfoLabel;
    QComboBox* locationSelector; // Dropdown to select locations
};
using Records = QVector<QMap<QString, QVariant>>;
using Record = QMap<QString, QVariant>;
// Function FilterRecords which takes Records and returns a pair of QVector QDateTime and QVector double
std::pair<QVector<QDateTime>, QVector<double>> FilterRecords(const Records& records);
// define function to find the max and min of any QVector QDateTime and double
std::pair<QVector<QDateTime>, QVector<double>> findMinMax(const QVector<QDateTime> dateTimes, const QVector<double> results);
// define function to take Records and return a QVector of Qvector of Qvectors SmapleDateTime and Result
QVector<QPoint> makePoints(const QVector<QDateTime>& dateTimes, const QVector<double>& results);

#endif
