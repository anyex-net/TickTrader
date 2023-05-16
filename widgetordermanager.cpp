#include "widgetordermanager.h"

COrderManager::COrderManager(QWidget* parent, Qt::WindowFlags f)
    :QWidget(parent, f)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    // �ر�ʱ�Զ��ͷ�
    setAttribute(Qt::WA_DeleteOnClose, false);

 //   qRegisterMetaType<CThostFtdcTradeField *>("CThostFtdcTradeField *");

    m_model = new QStandardItemModel;

    QStringList list;
    list << QString::fromLocal8Bit("���") << QString::fromLocal8Bit("����ʱ��")
         << QString::fromLocal8Bit("��Ʒ") << QString::fromLocal8Bit("����")
         << QString::fromLocal8Bit("�۸�") << QString::fromLocal8Bit("����")
         << QString::fromLocal8Bit("�ɽ�") << QString::fromLocal8Bit("״̬")
         << QString::fromLocal8Bit("��ע");
    m_model->setHorizontalHeaderLabels(list);

    ui.tableViewOrder->setModel(m_model);

//    connect(this, SIGNAL(addTrade(CThostFtdcTradeField*)), this, SLOT(onAddTrade(CThostFtdcTradeField*)));

}

COrderManager::~COrderManager()
{

}
