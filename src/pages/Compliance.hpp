#ifndef COMPLIANCEPAGE_HPP
#define COMPLIANCEPAGE_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPainter>
#include <QScrollArea>
#include <QWidget>
#include <qtmetamacros.h>
#include "../utils/database_utils.hpp"
class CompliancePage : public QWidget {
    Q_OBJECT

  private:
    QScrollArea *scrollArea;
    QVBoxLayout *scrollLayout;

  public:
    explicit CompliancePage(QWidget *parent = nullptr);

    void loadDataAsync(const QString &filter); //异步加载数据的接口，逻辑待实现
    void loadData(QString path);
};

//单个数据项的标签和合规状态
class ColorCircleWidget : public QWidget {
    Q_OBJECT
  public:
    explicit ColorCircleWidget(QWidget *parent = nullptr,
                               const Record &record = Record(),int i = 0)
        : QWidget(parent) {

        QStringList entryStrings;
        entryStrings << record["determinand_label"].toString() << QString::number(i);
        m_labelText = entryStrings.join("-"); // 将信息拼接为标签文本

        // 设置颜色根据合规状态
        m_circleColor = (record["is_compliance_sample"].toBool())
                            ? Qt::green
                            : Qt::red; // 假设1表示合规状态为绿色，2为红色

        // 设置Widget的大小
        setFixedSize(200, 50);

        // 创建标签
        m_label = new QLabel(m_labelText, this);
        m_label->setAlignment(Qt::AlignCenter);

        // 创建布局，将标签和颜色圆心放在一起
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->addWidget(m_label);
        setLayout(layout);
    }

  protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);

        // 设置绘制颜色
        painter.setBrush(m_circleColor);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制一个圆心
        int circleRadius = 5;                      // 圆心半径
        int circleX = width() - circleRadius - 10; // 计算圆心的X位置
        int circleY = (height() - circleRadius * 2) / 2; // 计算圆心的Y位置

        painter.drawEllipse(circleX, circleY, circleRadius * 2,
                            circleRadius * 2);
    }

  private:
    QLabel *m_label;
    QString m_labelText;
    QColor m_circleColor;
};
#endif
