#ifndef TRADEWIDGET_H
#define TRADEWIDGET_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include "ui_tradewidget.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include "loginWin.h"
#include <QMutex>
#include "notice.h"
#include <QTimer>

#define PRODUCT_VERSION 3

#define DW QApplication::desktop()->screenGeometry().width()
#define DH QApplication::desktop()->screenGeometry().height()

typedef struct
{
    QList<CThostFtdcInvestorPositionField *> posi;
    double points;      // �ص�����
    double trailPriceu; // ������λ
    bool trigFlag;      // ������־
}PosiPloy;

typedef struct
{
    QString name;        // �˻�����
    QString accountName; // ��¼��
    QString accountPass; // ��¼����
    QString routeName;   // ͨ����
    bool  updated;       // ״̬�����־
    CThostFtdcTraderApi* api;  // API����
    CTradeSpiImp *spi; // �ص�����
    CThostFtdcTradingAccountField * fund;       // ���˻���Ӧ���ʽ�
    QMap<QString,PosiPloy *> posiLst;// ���˻���Ӧ�ĳֲ���Ϣ KEY:��Լ��
    QMap<QString,CThostFtdcTradeField *> tradeLst;// ���˻���Ӧ�ĳɽ���Ϣ KEY:�ɽ���
    QMap<QString, CThostFtdcOrderField *> orderLst;// ���˻���Ӧ�Ķ�����Ϣ KEY:������
}TradeInfo;

typedef struct {
    char tif[32];                // �˻���
    TThostFtdcOrderRefType OrderRef;	/* ������ */
    TThostFtdcInvestorIDType InvestorID;	/* Ͷ���ߺ� */
    TThostFtdcInstrumentIDType InstrumentID;	/* ��Լ�� */
    TThostFtdcDirectionType Direction;	/* ������־ */
    TThostFtdcCombOffsetFlagType CombOffsetFlag;	/* ��ƽ��־ */
    TThostFtdcOrderPriceTypeType PriceType;	/* �۸����� */
    TThostFtdcPriceType Price;	/* �۸� */
    TThostFtdcVolumeType Qty;	/* ���� */
    TThostFtdcContingentConditionType ConditionMethod;	/* �������� */
    TThostFtdcExchangeIDType ExchangeID;	/* �������� */
}NEWORDERINF;

typedef struct {
    TThostFtdcOrderRefType OrderID1;	/* ������1 */
    TThostFtdcOrderRefType OrderID2;	/* ������2 */
}OCOGROUP; //oco���

class CoolSubmit;
class OrderManage;
class PosiManage;
class PriceView;
//class OrderView;
class configWidget;
class changePassword;
class dealView;
class TradeWidget : public QMainWindow
{
    Q_OBJECT

public:
    TradeWidget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
    ~TradeWidget();
    // status ��
    QLabel * loginStatusLbl;
    // ��Լ�б�
    QMap<QString, CThostFtdcInstrumentField *> insMap;
    // // ��ѡ��Լ
    QMap<QString, CThostFtdcInstrumentField *> insMap_zx;
    // ������Լ
    QMap<QString, CThostFtdcInstrumentField *> insMap_zl;
    // ��Դ����
    QMap<QString, CThostFtdcInstrumentField *> insMap_ny;
    // ������
    QMap<QString, CThostFtdcInstrumentField *> insMap_sq;
    // ֣����
    QMap<QString, CThostFtdcInstrumentField *> insMap_zs;
    // ������
    QMap<QString, CThostFtdcInstrumentField *> insMap_ds;
    // �н���
    QMap<QString, CThostFtdcInstrumentField *> insMap_zj;
    // ���ĵĺ�Լ
    QMap<QString, CThostFtdcInstrumentField *> insMap_dy;
    // ��ȡ��Լ�ı��۾���
    int getScale(QString InstrumentID);
    // ����MAP
    QMap<QString, CThostFtdcDepthMarketDataField *> quotMap;
    // ���޸ĵĶ����б�
    QList<NEWORDERINF> o2upLst;
    // oco����б�
    QList<OCOGROUP> ocoList;
    // �����ɹ�������µ��µ��б�
    void checkCancelOrder(CThostFtdcInputOrderActionField *pOrderCancelRsp);
    // ��������
    void priceEmit(CThostFtdcDepthMarketDataField *p);
    // ��Ӻ�Լ��Ϣ
    void instrEmit(CTradeSpiImp * t);
    // ��ӳֲ���Ϣ
    void posiEmit(CTradeSpiImp * t, CThostFtdcInvestorPositionField * pPosi, bool bLast = false);
    // ����˻��ʽ���Ϣ
    void fundEmit(CTradeSpiImp * t, CThostFtdcTradingAccountField * pFund);
    // ��Ӷ�����Ϣ
    void orderEmit(CTradeSpiImp * t, CThostFtdcOrderField * pOrder, bool bLast = false);
    // ��ӳɽ���Ϣ
    void tradeEmit(CTradeSpiImp * t, CThostFtdcTradeField * pTrade, bool bLast = false);
    // ������Ϣ����
    void orderMessageEmit(QString mes);
    // �����������
    OrderManage * omWidget;
    // �ֲֹ������
    PosiManage * cmWidget;
    // ���۰洰��
    PriceView * pview;
    // ��ͳ�µ�����
    //OrderView * oview;
    // �ɽ�������
    dealView * dview;
    // �����˻����ӳɹ�
    void TradeConnect(CTradeSpiImp * t);
    Ui::TradeWidgetClass ui;
    // ��ݽ��׽���
    CoolSubmit * CSubmit;
    // ����ͨ��
    void runTrade(UserInfo & element);
    // ����״̬��
    void updateStatusBar(CThostFtdcTradingAccountField * fund);
    // �򿪴�ͳ�µ�����
    //void showTradeW(CThostFtdcInstrumentField *s);
    // ����OCO״̬
    void checkOcoStatus(TradeInfo & tf, TThostFtdcOrderRefType OrderRef);
    // У�鶩��״̬
    void checkOrderStatus(CThostFtdcDepthMarketDataField * sq);
    // У�鸡��ӯ��
    void checkFloatingPL(CThostFtdcDepthMarketDataField * sq);
protected:
    void closeEvent (QCloseEvent * event);
private:
    // ״̬����ʾ��Ϣ
    QLabel * msgLabel;

    QTimer m_timer;
    QTimer m_timerFreshTips;
public slots:
    void onTimerReqPos();
    void slot_showOrderWin(TThostFtdcInstrumentIDType m_strInstr);
    void tradingAftorLogin();
    // �������״���
    void initTradeWin();
    // �쳣��Ϣ����
    void errManage(int code, QString mess);
    // ������¼����
    void comuWithTradeImp(TRADE_SIGNAL sig);
    // ��������¼
    void addPrice(CThostFtdcDepthMarketDataField *p);
    // �յ���Լ��Ϣ����
    void addInstr(CTradeSpiImp * t);
    // ��Ӷ�����¼
    void addOrder(CTradeSpiImp * t, CThostFtdcOrderField * pOrder, bool bLast);
    // ��ӳֲּ�¼
    void addPosi(CTradeSpiImp * t, CThostFtdcInvestorPositionField * pPosi, bool bLast);
    // ����˻��ʽ��¼
    void addFunds(CTradeSpiImp * t, CThostFtdcTradingAccountField * pFund);
    // ��ӳɽ���¼
    void addTrade(CTradeSpiImp * t, CThostFtdcTradeField * pTrade, bool bLast);
    // �����ɹ�����
    void doCancelOrder(CThostFtdcInputOrderActionField *pOrderCancelRsp);
    // �����˻����ӳɹ�
    void doTradeConnSec(CTradeSpiImp * t);
    // ������Ϣ����
    void messageSrv(QString mes = "");
    // ��������
    void orderManage();
    // �ֲֹ���
    void posiManage();
    // ���۰�չʾ
    void showPriceW();
    // �����ý���
    void showConfW();
    // �򿪴�ͳ�µ�����
    /*void showTradeW();*/
    // �򿪳ɽ�������
    void showDealW();
    // ���޸��������
    void showChangePassword();
    // ����
    void help();
    // չʾK��ͼ
    void showCurve();

    void onTimerClearTips();
signals:
    // ����ɾ����
    void orderDroped();
    // ��������
    void getPpricePush(CThostFtdcDepthMarketDataField *p);
    // �����˻���¼�ɹ�
    void tradeConnSec(CTradeSpiImp * t);
    // �յ���Լ��Ϣ����
    void getInstrPush(CTradeSpiImp * t);
    // �յ���������
    void getOrderPush(CTradeSpiImp * t, CThostFtdcOrderField * order, bool bLast);
    // �յ��ֲ�����
    void getPosiPush(CTradeSpiImp * t,CThostFtdcInvestorPositionField * pPosi, bool bLast);
    // �յ��ʽ�����
    void getFundPush(CTradeSpiImp * t,CThostFtdcTradingAccountField * pFund);
    // �յ��ɽ�����
    void getTradePush(CTradeSpiImp * t,CThostFtdcTradeField * pTrade, bool bLast);
    // �����ɹ�����
    void cancelOrder(CThostFtdcInputOrderActionField *pOrderCancelRsp);
    // ��������rsp��Ϣ
    void orderMessage(QString mes);
    // ӯ���仯
    void floating();
};

// �������ݽӿ�
extern CMdSpiImp * quotSpi;
// ����ӿڿ��
extern CThostFtdcMdApi* pQuotApi;
// ��¼�ӿڿ��
extern CThostFtdcTraderApi* pTraderApi;
// ��¼�ص��ӿ�
extern CTradeSpiImp *pTraderSpi;

// �˻�map
extern QMap<QString, TradeInfo> tradeInfoLst;

#endif // TRADEWIDGET_H
