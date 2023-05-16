#include "widgettrademanager.h"

#include "coolsubmit.h"
#include "tradewidget.h"
#include "notice.h"
#include <QMetaType>

extern loginWin * loginW;
extern QString convertBsFlag(TThostFtdcDirectionType flag);
extern QString convertOcFlag(TThostFtdcDirectionType flag);
extern QString convertPriceType(TThostFtdcDirectionType flag);
extern QString convertConditionMethod(TThostFtdcDirectionType flag);
extern QString convertOrderStatus(TThostFtdcDirectionType flag);
extern QString convertExchangeID(TThostFtdcExchangeIDType ID);
extern quint32 CreateNewRequestID();

CTradeManager::CTradeManager(QWidget* parent, Qt::WindowFlags f)
    :QWidget(parent, f)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    // 关闭时自动释放
    setAttribute(Qt::WA_DeleteOnClose, false);

    qRegisterMetaType<CThostFtdcTradeField *>("CThostFtdcTradeField *");

    m_model = new QStandardItemModel;

    QStringList list;
    list << QString::fromLocal8Bit("序号") << QString::fromLocal8Bit("成交时间")
         << QString::fromLocal8Bit("商品") << QString::fromLocal8Bit("买卖")
         << QString::fromLocal8Bit("价格") << QString::fromLocal8Bit("数量");
    m_model->setHorizontalHeaderLabels(list);

    ui.tableViewTrade->setModel(m_model);

    connect(this, SIGNAL(addTrade(CThostFtdcTradeField*)), this, SLOT(onAddTrade(CThostFtdcTradeField*)));
}

CTradeManager::~CTradeManager()
{

}

void CTradeManager::onAddTrade(CThostFtdcTradeField *info)
{
    QList<QStandardItem*> listItem;
    listItem.append(new QStandardItem(QString::number(m_model->rowCount()+1)));
    listItem.append(new QStandardItem(QString(info->TradeTime)));
    listItem.append(new QStandardItem(QString(info->InstrumentID)));
    listItem.append(new QStandardItem(QString("%1%2").arg(convertBsFlag(info->Direction)).arg(convertOcFlag(info->OffsetFlag))));
    listItem.append(new QStandardItem(QString::number(info->Price)));
    listItem.append(new QStandardItem(QString::number(info->Volume)));

    m_model->insertRow(0, listItem);
}
