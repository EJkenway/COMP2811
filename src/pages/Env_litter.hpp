#ifndef ENVPAGE_HPP
#define ENVPAGE_HPP

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class EnvPage : public QWidget {
    Q_OBJECT

public:
    explicit EnvPage(QWidget* parent = nullptr);

private slots:
    void onSearchClicked();

private:
    QLineEdit* searchBox;
    QPushButton* searchButton;
    QLabel* resultLabel;
};

#endif