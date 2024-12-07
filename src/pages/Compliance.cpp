#include "Compliance.hpp"
#include "../utils/database_utils.hpp"
#include "iostream"
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>


CompliancePage::CompliancePage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this); //垂直排列所有子组件

    // 创建过滤输入框
    QLineEdit *materialFilterInput = new QLineEdit(this);
    materialFilterInput->setPlaceholderText("Filter by Material Name...");
    layout->addWidget(materialFilterInput);

    QLineEdit *samplingSiteFilterInput = new QLineEdit(this);
    samplingSiteFilterInput->setPlaceholderText("Filter by Sampling Site...");
    layout->addWidget(samplingSiteFilterInput);

    QLineEdit *complianceStatusFilterInput = new QLineEdit(this);
    complianceStatusFilterInput->setPlaceholderText(
        "Filter by Compliance Status...");
    layout->addWidget(complianceStatusFilterInput);

    // 创建可滚动的显示窗口展示筛选后的数据
    scrollArea = new QScrollArea(this);
    QWidget *container = new QWidget();
    scrollLayout = new QVBoxLayout(container);

    // 向显示窗口添加多个 ColorCircleWidget 实例

    // 将容器设置为滚动区域的内容
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);

    // 创建标题标签
    QLabel *label = new QLabel("Welcome to the Compliance Page", this);
    label->setAlignment(Qt::AlignCenter);

    // 添加标题和输入框到布局。
    layout->insertWidget(0, label);
    layout->insertWidget(1, materialFilterInput);
    layout->insertWidget(2, samplingSiteFilterInput);
    layout->insertWidget(3, complianceStatusFilterInput);
    loadData("a"); //待实现loadDataAsync
}

void CompliancePage::loadData(QString path)
{

    // 使用 searchDatabase 函数从数据库中检索数据。当前使用的过滤条件是 "sampling_point"
    //限制数据避免卡死...待修改异步
    Records records = searchDatabase(
        "sampling_point", "http://environment.data.gov.uk/water-quality/id/"
                          "sampling-point/NE-40000385");
    int i = 0;
    for (Record record : records)
    {

        ColorCircleWidget *widget = new ColorCircleWidget(this, record, i++);
        scrollLayout->addWidget(widget);
        // std::cout << record["determinand_label"].toString().toStdString()
        // << "\n";
    }
}
