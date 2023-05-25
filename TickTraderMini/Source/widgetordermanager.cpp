#include "widgetordermanager.h"

COrderManager::COrderManager(QWidget* parent, Qt::WindowFlags f)
    :QWidget(parent, f)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    // 关闭时自动释放
    setAttribute(Qt::WA_DeleteOnClose, false);

 //   qRegisterMetaType<CThostFtdcTradeField *>("CThostFtdcTradeField *");

    m_model = new QStandardItemModel;

    QStringList list;
    list << QString::fromLocal8Bit("序号") << QString::fromLocal8Bit("报单时间")
         << QString::fromLocal8Bit("商品") << QString::fromLocal8Bit("买卖")
         << QString::fromLocal8Bit("价格") << QString::fromLocal8Bit("数量")
         << QString::fromLocal8Bit("成交") << QString::fromLocal8Bit("状态")
         << QString::fromLocal8Bit("备注");
    m_model->setHorizontalHeaderLabels(list);

    ui.tableViewOrder->setModel(m_model);

//    connect(this, SIGNAL(addTrade(CThostFtdcTradeField*)), this, SLOT(onAddTrade(CThostFtdcTradeField*)));

}

COrderManager::~COrderManager()
{

}
