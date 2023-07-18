#ifdef WIN32
#include "windows.h"
#endif

#include "tradewidget.h"
#include <QContextMenuEvent>
#include <string>
#include <QDateTime>
#include <qlibrary.h>
#include "coolsubmit.h"
#include "orderManage.h"
#include "posiManage.h"
#include "help.h"
#include <QSettings>
#include <QFile>
#include <QUrl>
#include <QTextStream>
#include "PriceView.h"
#include "configWidget.h"
#include "changePassword.h"
#include "dealView.h"
#include "notice.h"
#include <time.h>
#include <QThread>

extern configWidget * cview;
extern changePassword *cpView;
extern loginWin * loginW;
extern TradeWidget * g_tw;

clock_t a;

typedef struct
{
    QString name;
    QString dll;
    QString params;
}RouteInfo;

#define BCESMacroFlowResumeMethodResume 2
#define ORDERWIDGETW 380
#define GRAPHH 300

using namespace std;

static quint32 g_RequestID = 0;

// 线程锁
//QMutex mutex;

quint32 CreateNewRequestID()
{
    ++g_RequestID;
    return g_RequestID;
}

static int virtualOrderId = 100000;

quint32 CreateVirtualOrderId()
{
    ++virtualOrderId;
    return virtualOrderId;
}

CThostFtdcTraderApi* pTraderApi = NULL;
CTradeSpiImp *pTraderSpi = NULL;

CThostFtdcMdApi* pQuotApi = NULL;
CMdSpiImp * quotSpi = NULL;

QMap<QString, TradeInfo> tradeInfoLst;

// 交易所号 文字转换
QString convertExchangeID(TThostFtdcExchangeIDType ID)
{
    int x = strlen(ID);
    QString str = QString::fromLocal8Bit(ID);
    if(::strcmp(ID, "SHFE") == 0)
    {
        str = QString::fromLocal8Bit("上期所");
    }
    else if(::strcmp(ID, "DCE") == 0)
    {
        str = QString::fromLocal8Bit("大商所");
    }
    else if(::strcmp(ID, "CZCE") == 0)
    {
        str = QString::fromLocal8Bit("郑商所");
    }
    else if(::strcmp(ID, "CFFEX") == 0)
    {
        str = QString::fromLocal8Bit("中金所");
    }
    else if(::strcmp(ID, "SZ") == 0)
    {
        str = QString::fromLocal8Bit("深市");
    }
    else if(::strcmp(ID, "SH") == 0)
    {
        str = QString::fromLocal8Bit("沪市");
    }
    else if(::strcmp(ID, "INE") == 0)
    {
        str = QString::fromLocal8Bit("能源中心");
    }
    else if(::strcmp(ID, "NULL") == 0)
    {
        str = QString::fromLocal8Bit("TickTrader");
    }
    return str;
}

// 价格类型 文字转换
QString convertPriceType(TThostFtdcOrderPriceTypeType flag)
{
    QString str = "";
    switch(flag)
    {
    case THOST_FTDC_OPT_LimitPrice:
        str = QString::fromLocal8Bit("限价");
        break;
    case THOST_FTDC_OPT_AnyPrice:
        str = QString::fromLocal8Bit("市价");
        break;
//	case BCESConstPriceTypeStop:
//		str = QString::fromLocal8Bit(BCESConstCommentPriceTypeStop);
//		break;
//	case BCESConstPriceTypeLimit:
//		str = QString::fromLocal8Bit(BCESConstCommentPriceTypeLimit);
//		break;
    }
    return str;
}

// 开平 文字转换
QString convertOcFlag(TThostFtdcOffsetFlagType flag)
{
    QString str = "";
    switch(flag)
    {
    case THOST_FTDC_OF_Open:
        str = QString::fromLocal8Bit("开");
        break;
    case THOST_FTDC_OF_Close:
        str = QString::fromLocal8Bit("平");
        break;
    case THOST_FTDC_OF_CloseToday:
        str = QString::fromLocal8Bit("平今");
        break;
    case THOST_FTDC_OF_CloseYesterday:
        str = QString::fromLocal8Bit("平昨");
        break;
    case THOST_FTDC_OF_ForceClose:
        str = QString::fromLocal8Bit("强平");
        break;
    }
    return str;
}


// 买卖 文字转换
QString convertBsFlag(TThostFtdcDirectionType flag)
{
    QString str = "";
    switch(flag)
    {
    case THOST_FTDC_D_Buy:
        str = QString::fromLocal8Bit("买");
        break;
    case THOST_FTDC_D_Sell:
        str = QString::fromLocal8Bit("卖");
        break;
//	case BCESConstBSFlagExecute:
//		str = QString::fromLocal8Bit(BCESConstCommentBSFlagExecute);
//		break;
    }
    return str;
}

// 订单状态文字转换
QString convertOrderStatus(TThostFtdcOrderStatusType flag)
{
    QString str = "";
    switch(flag)
    {
    case THOST_FTDC_OST_NoTradeNotQueueing:
        str = QString::fromLocal8Bit("申报中");
        break;
    case THOST_FTDC_OST_NoTradeQueueing:
        str = QString::fromLocal8Bit("已申报");
        break;
    case THOST_FTDC_OST_AllTraded:
        str = QString::fromLocal8Bit("全部成交");
        break;
    case THOST_FTDC_OST_PartTradedQueueing:
        str = QString::fromLocal8Bit("部分成交");
        break;
    case THOST_FTDC_OST_Canceled:
        str = QString::fromLocal8Bit("已撤消");
        break;
    case THOST_FTDC_OST_NotTouched:
        str = QString::fromLocal8Bit("未触发");
        break;
    }
    return str;
}

// 条件类型文字转换
QString convertConditionMethod(TThostFtdcDirectionType flag)
{
    QString str = "";
    switch(flag)
    {
//	case THOST_FTDC_CC_Immediately:
//		str = QString::fromLocal8Bit(BCESConstCommentConditionMethodNone);
//		break;
//	case BCESConstConditionMethodEqual:
//		str = QString::fromLocal8Bit(BCESConstCommentConditionMethodEqual);
//		break;
//	case BCESConstConditionMethodMore:
//		str = QString::fromLocal8Bit(BCESConstCommentConditionMethodMore);
//		break;
//	case BCESConstConditionMethodMoreEqual:
//		str = QString::fromLocal8Bit(BCESConstCommentConditionMethodMoreEqual);
//		break;
//	case BCESConstConditionMethodLess:
//		str = QString::fromLocal8Bit(BCESConstCommentConditionMethodLess);
//		break;
//	case BCESConstConditionMethodLessEqual:
//		str = QString::fromLocal8Bit(BCESConstCommentConditionMethodLessEqual);
//		break;
    }
    return str;
}

int TradeWidget::getScale(QString InstrumentID)
{
    int pScale = 2;
    CThostFtdcInstrumentField * sbInstr = insMap[InstrumentID];
    if(!sbInstr)
        return pScale;
    if(sbInstr->PriceTick > 0.95)
        pScale = 0;
    else if(sbInstr->PriceTick > 0.095 && sbInstr->PriceTick < 0.95)
        pScale = 1;
    else if(sbInstr->PriceTick > 0.0095 && sbInstr->PriceTick < 0.095)
        pScale = 2;
    else if(sbInstr->PriceTick > 0.00095 && sbInstr->PriceTick < 0.0095)
        pScale = 3;
    return pScale;
}

TradeWidget::TradeWidget(QWidget *parent, Qt::WindowFlags flags)
    :QMainWindow(parent, flags)
{
    initTradeWin();
    CSubmit = NULL;
    omWidget = NULL;
    cmWidget = NULL;
    pview = NULL;
    //oview = NULL;
    dview = NULL;
    loginStatusLbl = new QLabel(this);
    this->ui.statusBar->addWidget(loginStatusLbl);
    //this->ui.statusBar->setStyleSheet("font-size:7px;");
    this->setMaximumWidth(320);
    // 关闭时自动释放
    setAttribute(Qt::WA_DeleteOnClose, true);
    setGeometry(DW/2-160,DH/2-250,320,500);

    m_neddReqPos.clear();
    m_reqPosEndFlag = true;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimerReqPos()));
    connect(&m_timerFreshTips, SIGNAL(timeout()), this, SLOT(onTimerClearTips()));
}

void TradeWidget::closeEvent (QCloseEvent * event)
{
    QFile file("./logIndex.txt");
    if(file.open(QIODevice::ReadWrite | QIODevice::Truncate)){
        file.write(QString(logInstance.fileNameIndex()).toLocal8Bit());
        file.close();
    }
    Notice info(Notice::NOTICE_TYPE_WARNING, QString::fromLocal8Bit("退出TickTrader？"), false, QString::fromLocal8Bit("提示"), NULL, 0);
    info.exec();
    if(info.pushButton) {
        exit(0);
    }
    event->ignore();
}


void TradeWidget::doCancelOrder(CThostFtdcInputOrderActionField *pOrderCancelRsp)
{
    for(int i=0;i<o2upLst.length();i++)
    {
        NEWORDERINF & iti = o2upLst[i];
        if(::strcmp(pOrderCancelRsp->OrderRef, iti.OrderRef) == 0)
        {
            int nRequestID = CreateNewRequestID();
            CThostFtdcInputOrderField pInputOrder;
            ::memset(&pInputOrder,0,sizeof(CThostFtdcInputOrderField));
            strncpy(pInputOrder.BrokerID, loginW->brokerID, sizeof(pInputOrder.BrokerID));
            strncpy(pInputOrder.InvestorID,iti.InvestorID, sizeof(pInputOrder.InvestorID)); /* 投资者号 */
            strncpy(pInputOrder.InstrumentID, iti.InstrumentID, sizeof(pInputOrder.InstrumentID));
            strncpy(pInputOrder.ExchangeID, iti.ExchangeID, sizeof(pInputOrder.ExchangeID));
            sprintf(pInputOrder.OrderRef, "%12i", nRequestID);
            pInputOrder.Direction = iti.Direction; /* 买卖标志 */
            pInputOrder.CombOffsetFlag[0] = iti.CombOffsetFlag[0]; /* 开平标志 */
            pInputOrder.CombHedgeFlag[0] = THOST_FTDC_BHF_Speculation; /* 投机套保标志 */
            pInputOrder.TimeCondition = THOST_FTDC_TC_GFD;
            pInputOrder.VolumeCondition = THOST_FTDC_VC_AV;
            pInputOrder.MinVolume = 1;
            pInputOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
            pInputOrder.OrderPriceType = iti.PriceType;	/* 价格类型 */
            pInputOrder.LimitPrice = iti.Price;	/* 价格 */
            pInputOrder.VolumeTotalOriginal = iti.Qty;	/* 数量 */
            pInputOrder.ContingentCondition = THOST_FTDC_CC_Immediately;	/* 限价单模式 */
  //          tradeInfoLst[iti.tif].api->ReqOrderInsert(&pInputOrder, nRequestID); // 录入订单
            if(CSubmit != NULL){
                qInfo() << "cancle order over, new insert order";
                CSubmit->OrderInsert(&pInputOrder, pInputOrder.CombOffsetFlag[0], pInputOrder.VolumeTotalOriginal, nRequestID);
            }
            o2upLst.removeAt(i);
            break;
        }
    }
// delete pOrderCancelRsp;
}

// 撤单成功后检查待下的新单列表
void TradeWidget::checkCancelOrder(CThostFtdcInputOrderActionField *pOrderCancelRsp)
{
    CThostFtdcInputOrderActionField * cr = new CThostFtdcInputOrderActionField();
    if(pOrderCancelRsp)
        ::memcpy(cr, pOrderCancelRsp,sizeof(CThostFtdcInputOrderActionField));
    emit cancelOrder(cr);
}

void TradeWidget::tradingAftorLogin()
{
//    loginW->hide();
//    this->show();
    UserInfo product;
    strncpy(product.name,loginW->userName,sizeof(product.name));
    strncpy(product.pass,loginW->password,sizeof(product.pass));
//	strncpy(product.serv,loginW->server.name,sizeof(product.serv));
    runTrade(product);
    //QLabel * noticeLbl = new QLabel(QString::fromLocal8Bit(" 正在接收行情，请等待..."));
    //QFont font;
 //   font.setFamily(QString::fromUtf8("Cambria Math"));
 //   font.setPointSize(24);
 //   noticeLbl->setFont(font);
 //   noticeLbl->setAlignment(Qt::AlignCenter);
    //setCentralWidget(noticeLbl);

    CSubmit = new CoolSubmit(g_tw->insMap.begin().key(), NULL, g_tw);
    setCentralWidget(CSubmit);
    orderMessageEmit(loginW->userName + QString::fromLocal8Bit(" \r\n登录成功！"));
    loginStatusLbl->setText(QString::fromLocal8Bit("已登录"));
    m_timerFreshTips.start(5000);
    a = clock();
}

void TradeWidget::runTrade(UserInfo & element)
{
    TradeInfo temp;
    temp.name = QString(element.name);
    TradeInfo & ti = tradeInfoLst[temp.name];
    if(ti.name != "" && ti.updated == false)
        return;

    temp.accountName= QString(element.name);
    temp.accountPass = QString(element.pass);
    temp.updated = false;

    temp.fund = new CThostFtdcTradingAccountField();
    temp.orderLst.clear();
    temp.tradeLst.clear();
    temp.posiLst.clear();
    temp.longPosis.clear();
    temp.shortPosis.clear();
    ::memset(temp.fund,0,sizeof(CThostFtdcTradingAccountField));
    strncpy(temp.fund->AccountID,temp.accountName.toLatin1().data(),sizeof(temp.fund->AccountID));
    temp.api=pTraderApi;
    temp.spi=pTraderSpi;
    tradeInfoLst[temp.name] = temp;

}

// 更新状态栏
void TradeWidget::updateStatusBar(CThostFtdcTradingAccountField * fund)
{
    //double dqqy = fund->FreeBalance+fund->Margin+fund->FeeFrozen+fund->FrozenMargin+fund->FloatingPL; // 当前权益
    //QString lsb = QString::fromLocal8Bit("当前权益:");
    //lsb.append(QString::number(dqqy,'f',0));
    //lsb.append(QString::fromLocal8Bit("，可用资金:"));
    //lsb.append(QString::number(fund->FreeBalance,'f',0));
    //lsb.append(QString::fromLocal8Bit("，浮动盈亏:"));
    //lsb.append(QString::number(fund->FloatingPL,'f',0));
    //loginStatusLbl->setText(lsb);
}

// 交易账户连接成功
void TradeWidget::TradeConnect(CTradeSpiImp * t)
{
    emit tradeConnSec(t);
}

// 交易账户连接成功
void TradeWidget::doTradeConnSec(CTradeSpiImp * t)
{
    QMapIterator<QString, TradeInfo> i(tradeInfoLst);
    while (i.hasNext())
    {
        TradeInfo & ti= tradeInfoLst[i.next().key()];
        if(ti.spi == t)
        {
            CThostFtdcReqUserLoginField LogonReq;
            memset(&LogonReq,0x00,sizeof(LogonReq));
            QString userName =ti.accountName;
            QString pwd = ti.accountPass;
            strncpy(LogonReq.UserID,userName.toLatin1().data(),sizeof(LogonReq.UserID));
            strncpy(LogonReq.Password,pwd.toLatin1().data(),sizeof(LogonReq.Password));
            memcpy(LogonReq.UserProductInfo, "openctp", sizeof(LogonReq.UserProductInfo));
//			LogonReq.ProductVersion = PRODUCT_VERSION;
            ti.api->ReqUserLogin(&LogonReq,0);
            break;
        }
    }
}

// 行情推送
void TradeWidget::priceEmit(CThostFtdcDepthMarketDataField *p)
{
    CThostFtdcDepthMarketDataField * pe = new CThostFtdcDepthMarketDataField;
    ::memcpy(pe, p,sizeof(CThostFtdcDepthMarketDataField));
    emit getPpricePush(pe);
}

// 添加合约信息
void TradeWidget::instrEmit(CTradeSpiImp * t)
{
    emit getInstrPush(t);
}

// 添加持仓消息
void TradeWidget::posiEmit(CTradeSpiImp * t, CThostFtdcInvestorPositionField * pPosi, bool bLast)
{
    if(pPosi)
    {
        CThostFtdcInvestorPositionField * posi = new CThostFtdcInvestorPositionField;
        ::memcpy(posi, pPosi, sizeof(CThostFtdcInvestorPositionField));

        emit getPosiPush(t, posi, bLast);
    }
    else
        emit getPosiPush(t, pPosi, bLast);
}

// 添加账户资金消息
void TradeWidget::fundEmit(CTradeSpiImp * t, CThostFtdcTradingAccountField * pFund)
{
    CThostFtdcTradingAccountField * fund = new CThostFtdcTradingAccountField;
    ::memcpy(fund, pFund, sizeof(CThostFtdcTradingAccountField));
    emit getFundPush(t, fund);
}

// 添加订单消息
void TradeWidget::orderEmit(CTradeSpiImp * t, CThostFtdcOrderField * pOrder, bool bLast, bool push)
{
    if(pOrder){
        CThostFtdcOrderField * order = new CThostFtdcOrderField;
        ::memcpy(order, pOrder, sizeof(CThostFtdcOrderField));
        if(order->OrderStatus==THOST_FTDC_OST_Canceled)
        {
            CThostFtdcInputOrderActionField oRsp;
            strncpy(oRsp.OrderRef, order->OrderRef, sizeof(oRsp.OrderRef));
            checkCancelOrder(&oRsp);
        }
        emit getOrderPush(t, order, bLast, push);
    }
    else
        emit getOrderPush(t, pOrder, bLast, push);
}

// 添加成交消息
void TradeWidget::tradeEmit(CTradeSpiImp * t, CThostFtdcTradeField * pTrade, bool bLast, bool push)
{
    if(pTrade){
        CThostFtdcTradeField * trade = new CThostFtdcTradeField;
        ::memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));
        emit getTradePush(t, trade, bLast, push);
    }
    else
        emit getTradePush(t, pTrade, bLast, push);
}

// 订单消息推送
void TradeWidget::orderMessageEmit(QString mes)
{
    QString mess = mes;
    emit orderMessage(mess);
}

void TradeWidget::orderInsertRsp(QString orderRef)
{
    emit orderInsertRspPush(orderRef);
}

// 添加行情记录
void TradeWidget::addPrice(CThostFtdcDepthMarketDataField *p)
{
    CThostFtdcDepthMarketDataField * sIn = quotMap[QString::fromLocal8Bit(p->InstrumentID)];
    CThostFtdcDepthMarketDataField * cIn = NULL;
    // 判断是否需要更新主力合约列表
    CThostFtdcInstrumentField * qins = insMap[QString::fromLocal8Bit(p->InstrumentID)];
    if(qins && QString(qins->ProductID) != "")
    {
        QMap<QString, CThostFtdcInstrumentField *>::const_iterator iter = insMap_zl.find(QString::fromLocal8Bit(qins->ProductID));
        CThostFtdcInstrumentField * zlins = iter != insMap_zl.end() ? iter.value():NULL;
        if(!zlins)
        {
            insMap_zl[QString::fromLocal8Bit(qins->ProductID)] = qins;
        }
        else
        {
            CThostFtdcDepthMarketDataField * qq = quotMap[QString::fromLocal8Bit(zlins->InstrumentID)];
            if(qq && qq->OpenInterest < p->OpenInterest)
                insMap_zl[QString::fromLocal8Bit(qins->ProductID)] = qins;
        }
    }
    if(!sIn)
    {
        quotMap[QString::fromLocal8Bit(p->InstrumentID)] = p;
        cIn = p;
        if(pTraderSpi->loginFlags && !CSubmit && cIn)
        {
            slot_showOrderWin(cIn->InstrumentID);
        }
        return;
    }
    else
    {
        cIn = sIn;
        //// 判断价格有无变化 没有的话直接return;
        //if(sIn->Price-p->Price+0.00001 < 0.0001)
        //{
        //	delete p;
        //	return;
        //}
        ::memcpy(sIn,p,sizeof(CThostFtdcDepthMarketDataField));
        delete p;
    }
    // 校验订单状态
    checkOrderStatus(sIn);
    checkFloatingPL(sIn);
    if(CSubmit && CSubmit->selectInstr)
    {
        if(!CSubmit->selectInstr->curInstr)
        {
            CSubmit->selectInstr->curInstr = qins;
            emit floating();
        }
        else if(strcmp(CSubmit->selectInstr->curInstr->InstrumentID , cIn->InstrumentID) == 0 || cmWidget || pview)
        {
            emit floating();
        }
    }
}

// 校验浮动盈亏
void TradeWidget::checkFloatingPL(CThostFtdcDepthMarketDataField * p)
{
    QMapIterator<QString, TradeInfo> ti(tradeInfoLst);
    while (ti.hasNext())
    {
        QString key = ti.next().key();
        TradeInfo & tti = tradeInfoLst[key];
        tti.fund->PositionProfit = 0;
        QMapIterator<QString, PosiPloy *> poi(tti.posiLst);
        while(poi.hasNext())
        {
            QString keyP = poi.next().key();
            PosiPloy * pPloy = tti.posiLst[keyP];
            QList<CThostFtdcInvestorPositionField *> uposi;
            if(pPloy)
                uposi = pPloy->posi;
            if(uposi.size() > 0)
            {
//				if(keyP == p->InstrumentID && p->LastPrice > 0)
//				{
//					double yk = insMap[QString::fromLocal8Bit(p->InstrumentID)]->TradeUnit*((p->LastPrice-uposi->BuyPrice)*uposi->BuyQty+(uposi->SellPrice - p->LastPrice)*uposi->SellQty);
//                    uposi->PositionProfit = yk;
//                    tti.fund->PositionProfit += yk;
//				}
//				else
                for(int index = 0; index < uposi.size(); index++)
                {
                    tti.fund->PositionProfit += uposi[index]->PositionProfit;
                }
            }
        }
        updateStatusBar(tti.fund);
    }
}

// 跟踪OCO状态
void TradeWidget::checkOcoStatus(TradeInfo & tf, TThostFtdcOrderRefType OrderRef)
{
    for(int oci=0;oci<ocoList.length();oci++)
    {
        OCOGROUP & ocog = ocoList[oci];
        if(::strcmp(ocog.OrderID1, OrderRef) == 0)
        {
            CThostFtdcOrderField * oco2 = tf.orderLst[ocog.OrderID2];
            if(!oco2) continue;
            oco2->OrderStatus = THOST_FTDC_OST_Canceled;
            ocoList.removeAt(oci);
            break;
        }
        if(::strcmp(ocog.OrderID2, OrderRef) == 0)
        {
            CThostFtdcOrderField * oco2 = tf.orderLst[ocog.OrderID1];
            if(!oco2) continue;
            oco2->OrderStatus = THOST_FTDC_OST_Canceled;
            ocoList.removeAt(oci);
            break;
        }
    }
}

void TradeWidget::checkOrderStatus(CThostFtdcDepthMarketDataField * cQuot)
{
    if(!cQuot) return;
    TradeInfo & tf = tradeInfoLst[loginW->userName];
    QMap<QString,PosiPloy *>::const_iterator iter = tf.posiLst.find(QString::fromLocal8Bit(cQuot->InstrumentID));
    PosiPloy * pploy = iter != tf.posiLst.end() ? iter.value():NULL;
    CThostFtdcInstrumentField * ci = insMap[QString::fromLocal8Bit(cQuot->InstrumentID)];
    if(!ci) return;
    double minMove = ci->PriceTick;
    if(!ci)  // 缺少合约信息
        return;
    QMapIterator<QString, CThostFtdcOrderField *> ot(tf.orderLst);
    while(ot.hasNext())
    {
        CThostFtdcOrderField * iti = tf.orderLst[ot.next().key()];
        if(!iti) //订单项为空
            continue;
        // 只处理未触发的订单
        if(iti->OrderStatus != THOST_FTDC_OST_NotTouched)
            continue;
        if(::strcmp(iti->InstrumentID, cQuot->InstrumentID) != 0)
            continue;
        bool sendFlag = false;
        double priceDis = cQuot->LastPrice - iti->LimitPrice;
        if(pploy && QString(iti->OrderRef).startsWith("ZZZS")) // 追踪止损
        {
            if(iti->Direction == THOST_FTDC_D_Sell && cQuot->LastPrice >= pploy->trailPriceu+0.00001)
            {
                pploy->trigFlag = true; // 多仓追踪止损触发
            }
            if(iti->Direction == THOST_FTDC_D_Buy && cQuot->LastPrice <= pploy->trailPriceu-0.00001)
            {
                pploy->trigFlag = true; // 空仓追踪止损触发
            }
            if(iti->Direction == THOST_FTDC_D_Sell && cQuot->LastPrice - iti->LimitPrice > pploy->points +0.00001 && pploy->trigFlag) // 多仓止损 当前价-止损价 变大
            {
                iti->LimitPrice = cQuot->LastPrice - pploy->points;
                pploy->trailPriceu = iti->LimitPrice + pploy->points;
            }
            if(iti->Direction == THOST_FTDC_D_Buy && iti->LimitPrice - cQuot->LastPrice > pploy->points - 0.00001 && pploy->trigFlag) // 空仓止损 止损价-当前价 变大
            {
                iti->LimitPrice = cQuot->LastPrice + pploy->points;
                pploy->trailPriceu = iti->LimitPrice-pploy->points;
            }
            if(iti->Direction == THOST_FTDC_D_Sell && pploy->trigFlag && cQuot->LastPrice <= iti->LimitPrice-0.00001)
            {
                sendFlag = true; // 多仓止损条件达到
            }
            if(iti->Direction == THOST_FTDC_D_Buy && pploy->trigFlag && cQuot->LastPrice >= iti->LimitPrice+0.00001)
            {
                sendFlag = true; // 空仓止损条件达到
            }
        }
        else
        {
            switch(iti->ContingentCondition)
            {
            case THOST_FTDC_CC_Immediately:
                {
                    if(priceDis < 0.00001 && priceDis > -0.00001)
                    {
                        sendFlag = true;
                    }
                }
                break;
            case THOST_FTDC_CC_LastPriceGreaterThanStopPrice:
                {
                    if(cQuot->LastPrice > iti->LimitPrice+minMove-0.00001)
                    {
                        sendFlag = true;
                    }
                }
                break;
            case THOST_FTDC_CC_LastPriceGreaterEqualStopPrice:
                {
                    if(cQuot->LastPrice > iti->LimitPrice-0.00001)
                    {
                        sendFlag = true;
                    }
                }
                break;
            case THOST_FTDC_CC_LastPriceLesserThanStopPrice:
                {
                    if(cQuot->LastPrice < iti->LimitPrice-minMove+0.00001)
                    {
                        sendFlag = true;
                    }
                }
                break;
            case THOST_FTDC_CC_LastPriceLesserEqualStopPrice:
                {
                    if(cQuot->LastPrice < iti->LimitPrice+0.00001)
                    {
                        sendFlag = true;
                    }
                }
                break;
            }
        }
        if(sendFlag)
        {
            CThostFtdcInputOrderField pInputOrder;
            ::memset(&pInputOrder,0,sizeof(CThostFtdcInputOrderField));
            strncpy(pInputOrder.BrokerID, loginW->brokerID, sizeof(pInputOrder.BrokerID));
            strncpy(pInputOrder.InvestorID, iti->InvestorID,sizeof(pInputOrder.InvestorID)); /* 投资者号 */
            memcpy(pInputOrder.InstrumentID, iti->InstrumentID,sizeof(pInputOrder.InstrumentID));
            pInputOrder.Direction = iti->Direction; /* 买卖标志 */
            pInputOrder.CombOffsetFlag[0] = iti->CombOffsetFlag[0]; /* 开平标志 */
            pInputOrder.CombHedgeFlag[0] = THOST_FTDC_BHF_Speculation;
            pInputOrder.VolumeTotalOriginal = iti->VolumeTotalOriginal;	/* 数量 */
            strncpy(pInputOrder.OrderRef,iti->OrderRef,sizeof(pInputOrder.OrderRef));
            pInputOrder.ContingentCondition = THOST_FTDC_CC_Immediately; // 通道只接受‘5’
            strncpy(pInputOrder.ExchangeID, iti->ExchangeID,sizeof(pInputOrder.ExchangeID));// 交易所号
            if(pploy)
            {
                pploy->trigFlag = false;
                pploy->trailPriceu = 0;
                pploy->points = 0;
            }
            if(iti->Direction == THOST_FTDC_D_Sell)
            {
                pInputOrder.LimitPrice = cQuot->LowerLimitPrice;	/* 跌停价格 */
            }
            else
            {
                pInputOrder.LimitPrice = cQuot->UpperLimitPrice;	/* 涨停价格 */
            }
            pInputOrder.OrderPriceType = iti->OrderPriceType;
            pInputOrder.TimeCondition = THOST_FTDC_TC_GFD;
            pInputOrder.VolumeCondition = THOST_FTDC_VC_AV;
            pInputOrder.MinVolume = 1;
            pInputOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
//            strcpy( pInputOrder.MacAddress, "fe80::75ef:97de:2366:e490" );
//            strcpy( pInputOrder.IPAddress, "10.150.1.23" );
            iti->OrderStatus = THOST_FTDC_OST_Canceled;
//            tf.api->ReqOrderInsert(&pInputOrder, 0); // 录入订单
            orderEmit(tf.spi, iti);
            checkOcoStatus(tf,iti->OrderRef);
        }
    }
}

// 添加合约信息
void TradeWidget::addInstr(CTradeSpiImp * t)
{
    disconnect(this, SIGNAL(getInstrPush(CTradeSpiImp *)),this, SLOT(addInstr(CTradeSpiImp *)));
    insMap = t->tempCons;
    QMapIterator<QString, CThostFtdcInstrumentField *> insItr(insMap);
    while(insItr.hasNext())
    {
        QString keyP = insItr.next().key();
        CThostFtdcInstrumentField * instr = insMap[keyP];
        // 自选合约
        // todo 与db配置做判断
        // 主力合约 行情接收后做判断
        // 上期所
        if(::strcmp(instr->ExchangeID, "SHFE") == 0)
        {
            insMap_sq[keyP] = instr;
        }
        // 郑商所
        if(::strcmp(instr->ExchangeID, "CZCE") == 0)
        {
            insMap_zs[keyP] = instr;
        }
        // 大商所
        if(::strcmp(instr->ExchangeID, "DCE") == 0)
        {
            insMap_ds[keyP] = instr;
        }
        // 中金所
        if(::strcmp(instr->ExchangeID, "CFFEX") == 0)
        {
            insMap_zj[keyP] = instr;
        }
        // 能源中心
        if(::strcmp(instr->ExchangeID, "INE") == 0)
        {
            insMap_ny[keyP] = instr;
        }
    }
    pQuotApi = CThostFtdcMdApi::CreateFtdcMdApi("md");
    quotSpi = new CMdSpiImp(pQuotApi);
    if( pQuotApi == NULL || quotSpi == NULL )
    {
        return;
    }
    pQuotApi->RegisterSpi(quotSpi);
    pQuotApi->RegisterFront((char*)loginW->m_listAddr.at(loginW->serverAddrIndex).marketServ.toStdString().c_str());
    pQuotApi->Init( );

    connect(this, SIGNAL(getPpricePush(CThostFtdcDepthMarketDataField *)),this, SLOT(addPrice(CThostFtdcDepthMarketDataField *)), Qt::QueuedConnection);
}

// 添加订单记录
void TradeWidget::addOrder(CTradeSpiImp * t, CThostFtdcOrderField * order, bool bLast, bool push)
{
    if(order)
    {
        QString msg = QString::fromLocal8Bit("下单: ") + QString::fromLocal8Bit(order->InstrumentID)
                + QString(" ") + convertBsFlag(order->Direction) + convertOcFlag(order->CombOffsetFlag[0])
                + QString(" ") + QString::number(order->LimitPrice)
                + QString(" ") + QString::number(order->VolumeTotalOriginal)
                + QString(" ") + convertOrderStatus(order->OrderStatus)
                + QString(" ") + QString::fromLocal8Bit(order->StatusMsg);
        loginStatusLbl->setText(msg);
        m_timerFreshTips.start(5000);
    }
    if(bLast)
    {
        loginStatusLbl->clear();
        for( ; ; )
        {
            QThread::msleep(1000);
            int nRequestIDs = CreateNewRequestID();
            CThostFtdcQryTradeField reqInfo = {0};
            int ret = pTraderApi->ReqQryTrade(&reqInfo, nRequestIDs);
            if(ret == -2 || ret == -3)
                QThread::msleep(100);
            else
                break;
        }
    }
    if(!order)return;
    //mutex.lock();
    bool bEqOrderStatus = false;
    CThostFtdcOrderField & pOrder = *order;
    QString orderKey = QString("%1.%2.%3").arg(pOrder.FrontID).arg(pOrder.SessionID).arg(pOrder.OrderRef);
    QMapIterator<QString, TradeInfo> i(tradeInfoLst);
    int scale = getScale(pOrder.InstrumentID);
    while (i.hasNext())
    {
        TradeInfo & ti = tradeInfoLst[i.next().key()];
        if(ti.spi == t)
        {
            CThostFtdcOrderField * olt = ti.orderLst[orderKey];
            if(!olt)
            {
                if(pOrder.InsertTime[0] == 0)
                {
                    QTime t = QTime::currentTime();
                    QDate dt = QDate::currentDate();
                    sprintf(pOrder.InsertDate,"%04d%02d%02d", dt.year(), dt.month(),dt.day());
                    sprintf(pOrder.InsertTime,"%02d:%02d:%02d", t.hour(), t.minute(),t.second());
                }
                ti.orderLst[orderKey] = order;
            }
            else
            {
                qInfo() << "search Order: " << order->OrderRef << QString::fromLocal8Bit(order->StatusMsg) << order->OrderStatus
                     << order->OrderSubmitStatus << olt->OrderStatus << olt->VolumeTotalOriginal << olt->VolumeTotal;
                if(olt->OrderStatus == order->OrderStatus)
                    bEqOrderStatus = true;
                int tempVol = olt->VolumeTotal;
                ::memcpy(olt,order,sizeof(CThostFtdcOrderField));
                olt->VolumeTotal = tempVol;
                delete order;
            }
            break;
        }
    }
    //mutex.unlock();
    if(omWidget)
    {
        omWidget->update();
    }
    if(CSubmit)
    {
        CSubmit->update();
    }

//    if(push && !bEqOrderStatus){
//        updatePostion(order);
//    }
    if(push)
    {
        QMapIterator<QString, TradeInfo> ii(tradeInfoLst);
        while (ii.hasNext())
        {
            TradeInfo & ti = tradeInfoLst[ii.next().key()];
            if(ti.spi == t)
            {
                QMap<QString, stSelfPosi>::iterator iterPosi;
                if (pOrder.CombOffsetFlag[0] != THOST_FTDC_OF_Open) {
                    if (pOrder.Direction == THOST_FTDC_D_Buy) {
                        iterPosi = ti.shortPosis.find(pOrder.InstrumentID);
                        if (iterPosi == ti.shortPosis.end()) {
                            stSelfPosi stPosi = { 0 };
                            strcpy(stPosi.InstrumentID, pOrder.InstrumentID);
                            strcpy(stPosi.ExchangeID, pOrder.ExchangeID);
                            stPosi.BSFlag = THOST_FTDC_PD_Short;
                            ti.shortPosis[stPosi.InstrumentID] = stPosi;
                            iterPosi = ti.shortPosis.find(pOrder.InstrumentID);
                        }
                    }
                    else {
                        iterPosi = ti.longPosis.find(pOrder.InstrumentID);
                        if (iterPosi == ti.longPosis.end()) {
                            stSelfPosi stPosi = { 0 };
                            strcpy(stPosi.InstrumentID, pOrder.InstrumentID);
                            strcpy(stPosi.ExchangeID, pOrder.ExchangeID);
                            stPosi.BSFlag = THOST_FTDC_PD_Long;
                            ti.longPosis[stPosi.InstrumentID] = stPosi;
                            iterPosi = ti.longPosis.find(pOrder.InstrumentID);
                        }
                    }
                }
                else {
                    if (pOrder.Direction == THOST_FTDC_D_Sell) {
                        iterPosi = ti.shortPosis.find(pOrder.InstrumentID);
                        if (iterPosi == ti.shortPosis.end()) {
                            stSelfPosi stPosi = { 0 };
                            strcpy(stPosi.InstrumentID, pOrder.InstrumentID);
                            strcpy(stPosi.ExchangeID, pOrder.ExchangeID);
                            stPosi.BSFlag = THOST_FTDC_PD_Short;
                            ti.shortPosis[stPosi.InstrumentID] = stPosi;
                            iterPosi = ti.shortPosis.find(pOrder.InstrumentID);
                        }
                    }
                    else {
                        iterPosi = ti.longPosis.find(pOrder.InstrumentID);
                        if (iterPosi == ti.longPosis.end()) {
                            stSelfPosi stPosi = { 0 };
                            strcpy(stPosi.InstrumentID, pOrder.InstrumentID);
                            strcpy(stPosi.ExchangeID, pOrder.ExchangeID);
                            stPosi.BSFlag = THOST_FTDC_PD_Long;
                            ti.longPosis[stPosi.InstrumentID] = stPosi;
                            iterPosi = ti.longPosis.find(pOrder.InstrumentID);
                        }
                    }
                }
                char szKey[100]={0};
                sprintf(szKey,"%s",orderKey.toStdString().c_str());
                auto iterOrder=ti.orderLst.find(szKey);
                if(iterOrder==ti.orderLst.end()){
                    ti.orderLst[szKey]=&pOrder;
                    iterOrder=ti.orderLst.find(szKey);

                    // 新平仓订单要冻结持仓
                    if(m_IsInitEnd){
                        if(iterOrder.value()->CombOffsetFlag[0]!=THOST_FTDC_OF_Open){
                            if(iterOrder.value()->Direction==THOST_FTDC_D_Buy){
                                if(iterOrder.value()->CombOffsetFlag[0]==THOST_FTDC_OF_CloseToday){
                                    iterPosi.value().TodayQtyFrozen+=iterOrder.value()->VolumeTotalOriginal;
                                }
                                iterPosi.value().QtyFrozen+=iterOrder.value()->VolumeTotalOriginal;
                            }else{
                                if(iterOrder.value()->CombOffsetFlag[0]==THOST_FTDC_OF_CloseToday){
                                    iterPosi.value().TodayQtyFrozen+=iterOrder.value()->VolumeTotalOriginal;
                                }
                                iterPosi.value().QtyFrozen+=iterOrder.value()->VolumeTotalOriginal;
                            }
                        }
                    }
                }else{
                    // 更新订单（只处理撤单，成交处理在成交回报中）
                    if(/*iterOrder.value()->OrderStatus==THOST_FTDC_OST_Canceled || */iterOrder.value()->OrderStatus==THOST_FTDC_OST_AllTraded){
                        return; // 已处于最终状态的订单不再做任何处理
                    }

                    // 建立成交单对应的订单索引
                    if(strlen(pOrder.OrderSysID)){
                        mOrderIndexByExchangeIDAndOrderSysID[std::string(pOrder.ExchangeID)+std::string(pOrder.OrderSysID)]=szKey;
                    }

                    if(pOrder.OrderStatus!=THOST_FTDC_OST_Canceled){
                        return;
                    }

                    iterOrder.value()->OrderStatus=THOST_FTDC_OST_Canceled;
                    strcpy(iterOrder.value()->StatusMsg,pOrder.StatusMsg);

                    // 已撤的平仓订单要释放冻结的持仓
                    if(m_IsInitEnd){
                        if(iterOrder.value()->CombOffsetFlag[0]!=THOST_FTDC_OF_Open){
                            if(iterOrder.value()->Direction==THOST_FTDC_D_Buy){
                                if(iterOrder.value()->CombOffsetFlag[0]==THOST_FTDC_OF_CloseToday){
                                    iterPosi.value().TodayQtyFrozen-=iterOrder.value()->VolumeTotal;
                                }
                                iterPosi.value().QtyFrozen-=iterOrder.value()->VolumeTotal;
                            }else{
                                if(iterOrder.value()->CombOffsetFlag[0]==THOST_FTDC_OF_CloseToday){
                                    iterPosi.value().TodayQtyFrozen-=iterOrder.value()->VolumeTotal;
                                }
                                iterPosi.value().QtyFrozen-=iterOrder.value()->VolumeTotal;
                            }
                        }
                    }
                }
                QMap<QString, stSelfPosi>::iterator iterPosiLog;
                // 多仓
                if((iterPosiLog = ti.longPosis.find(pOrder.InstrumentID)) != ti.longPosis.end()){
                    qInfo() << "RtnOrder-Pos-Update-Buy: " << iterPosiLog.value().Qty << iterPosiLog.value().QtyFrozen << iterPosiLog.value().TodayQty << iterPosiLog.value().TodayQtyFrozen;
                }
                // 空仓
                if ((iterPosiLog = ti.shortPosis.find(pOrder.InstrumentID)) != ti.shortPosis.end()) {
                    qInfo() << "RtnOrder-Pos-Update-Sell: " << iterPosiLog.value().Qty << iterPosiLog.value().QtyFrozen << iterPosiLog.value().TodayQty << iterPosiLog.value().TodayQtyFrozen;
                }
                break;
            }
        }
    }
}

void TradeWidget::updatePostion(CThostFtdcOrderField *pOrder)
{
    if(m_neddReqPos.size() < 1)
        m_neddReqPos.append(1);

/*    qInfo() << "updatePostion: " << pOrder->InstrumentID << pOrder->ExchangeID << pOrder->OrderStatus
            << pOrder->OrderSubmitStatus << pOrder->Direction << pOrder->VolumeTotalOriginal
            << pOrder->VolumeTraded << pOrder->VolumeTotal;


    CThostFtdcOrderField *order = pOrder;
    TradeInfo & tf = tradeInfoLst[loginW->userName];
    QMap<QString,PosiPloy *>::const_iterator iter = tf.posiLst.find(QString::fromLocal8Bit(order->InstrumentID));
    PosiPloy * pploy = iter != tf.posiLst.end() ? iter.value():NULL;
    QList<CThostFtdcInvestorPositionField *> posi;
    if(pploy)
    {
        posi = pploy->posi;
        for(int index = 0; index < posi.size(); index++)
        {
            if((posi[index]->PosiDirection == THOST_FTDC_PD_Long && order->Direction == THOST_FTDC_D_Buy)
             || (posi[index]->PosiDirection == THOST_FTDC_PD_Short && order->Direction == THOST_FTDC_D_Sell))
            {
                if(order->OrderStatus == THOST_FTDC_OST_AllTraded)
                {
                    if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Open)  //开仓
                        posi[index]->Position += order->VolumeTotalOriginal;
                    else
                    {
                        if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Close)    //平仓
                    {
                        posi[index]->Position -= order->VolumeTotalOriginal;
                    }
                    else if(order->CombOffsetFlag[0] == THOST_FTDC_OF_CloseYesterday) //平昨
                    {
                        posi[index]->Position -= order->VolumeTotalOriginal;
                        posi[index]->YdPosition -= order->VolumeTotalOriginal;
                    }
                    else if(order->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday) //平今
                    {
                        posi[index]->Position -= order->VolumeTotalOriginal;
                    }

                    if(order->Direction == THOST_FTDC_D_Buy)
                        posi[index]->LongFrozen = (posi[index]->LongFrozen - order->VolumeTotalOriginal + order->VolumeTotal);
                    else
                        posi[index]->ShortFrozen = (posi[index]->ShortFrozen - order->VolumeTotalOriginal + order->VolumeTotal);
                    }
                }
                else if(order->OrderStatus == THOST_FTDC_OST_PartTradedQueueing)
                {
                    if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Open)  //开仓
                        posi[index]->Position += order->VolumeTraded;
                    else
                    {
                        if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Close)    //平仓
                        {
                            posi[index]->Position -= order->VolumeTraded;
                        }
                        else if(order->CombOffsetFlag[0] == THOST_FTDC_OF_CloseYesterday) //平昨
                        {
                            posi[index]->Position -= order->VolumeTraded;
                            posi[index]->YdPosition -= order->VolumeTraded;
                        }
                        else if(order->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday) //平今
                        {
                            posi[index]->Position -= order->VolumeTraded;
                        }
                        if(order->Direction == THOST_FTDC_D_Buy)
                            posi[index]->LongFrozen = (posi[index]->LongFrozen - order->VolumeTotalOriginal + order->VolumeTotal);
                        else
                            posi[index]->ShortFrozen = (posi[index]->ShortFrozen - order->VolumeTotalOriginal + order->VolumeTotal);
                   }
                }
                else if(order->OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing)
                {
//                    if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Open)  //开仓
//                        posi[index]->Position += order->VolumeTraded;
//                    else
//                    {
//                        if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Close)    //平仓
//                        {
//                            posi[index]->Position -= order->VolumeTraded;
//                        }
//                        else if(order->CombOffsetFlag[0] == THOST_FTDC_OF_CloseYesterday) //平昨
//                        {
//                            posi[index]->Position -= order->VolumeTraded;
//                            posi[index]->YdPosition -= order->VolumeTraded;
//                        }
//                        else if(order->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday) //平今
//                        {
//                            posi[index]->Position -= order->VolumeTraded;
//                        }

//                        if(order->Direction == THOST_FTDC_D_Buy)
//                            posi[index]->LongFrozen -= order->VolumeTotal;
//                        else
//                            posi[index]->ShortFrozen -= order->VolumeTotal;
//                    }
                }
                else if(order->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
                {
                    if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Open)  //开仓
                    {
                        //
                    }
                    else
                    {
                        if(order->Direction == THOST_FTDC_D_Buy)
                            posi[index]->LongFrozen += order->VolumeTotal;
                        else
                            posi[index]->ShortFrozen += order->VolumeTotal;
                    }
                }
                else if(order->OrderStatus == THOST_FTDC_OST_Canceled)
                {
                    if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Open)  //开仓
                    {
                        //
                    }
                    else if(order->CombOffsetFlag[0] == THOST_FTDC_OF_Close
                            || order->CombOffsetFlag[0] == THOST_FTDC_OF_CloseYesterday
                            || order->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday)    //平仓
                    {
                        if(order->Direction == THOST_FTDC_D_Buy)
                            posi[index]->LongFrozen -= order->VolumeTotal;
                        else
                            posi[index]->ShortFrozen -= order->VolumeTotal;
                    }
                }
                qInfo() << "updatePostion2: " << posi[index]->InstrumentID << posi[index]->ExchangeID
                        << posi[index]->PosiDirection << posi[index]->YdPosition
                        << posi[index]->Position << posi[index]->LongFrozen
                        << posi[index]->ShortFrozen;
            }
        }
        pploy->posi = posi;
    }

    if(cmWidget)
    {
        cmWidget->update();
    }*/
}

// 添加成交记录
void TradeWidget::addTrade(CTradeSpiImp * t, CThostFtdcTradeField  * trade, bool bLast, bool push)
{
    if(bLast)
    {
        for( ; ; )
        {
            QThread::msleep(1000);
            int nRequestIDs = CreateNewRequestID();
            CThostFtdcQryInvestorPositionField reqInfo = {0};
            int ret = pTraderApi->ReqQryInvestorPosition(&reqInfo, nRequestIDs);
            if(ret == -2 || ret == -3)
                QThread::msleep(100);
            else
            {
                tradeInfoLst[loginW->userName].posiLst.clear();
                m_reqPosEndFlag = false;
                break;
            }
        }
    }
    if(!trade) return;
    //mutex.lock();
    CThostFtdcTradeField & pTrade = *trade;
    QMapIterator<QString, TradeInfo> i(tradeInfoLst);
    int scale = getScale(pTrade.InstrumentID);
    QString MOid = QString::fromLocal8Bit(pTrade.TradeID).append("_").append(pTrade.OrderRef);
    while (i.hasNext())
    {
        TradeInfo & ti = tradeInfoLst[i.next().key()];
        if(ti.spi == t)
        {
            CThostFtdcTradeField * olt = ti.tradeLst[MOid];
            if(!olt)
            {
                ti.tradeLst[MOid] = trade;
            }
            else
            {
                ::memcpy(olt,olt,sizeof(CThostFtdcTradeField));
                delete trade;
            }
            break;
        }
    }
    if(push)
    {
        QMapIterator<QString, TradeInfo> ii(tradeInfoLst);
        while (ii.hasNext())
        {
            TradeInfo & ti = tradeInfoLst[ii.next().key()];
            if(ti.spi == t)
            {// 计算订单成交均价
                std::map<std::string,std::string>::iterator iterIndex=mOrderIndexByExchangeIDAndOrderSysID.find(std::string(pTrade.ExchangeID)+std::string(pTrade.OrderSysID));
                if(iterIndex!=mOrderIndexByExchangeIDAndOrderSysID.end()){
                    QMap<QString, CThostFtdcOrderField *>::iterator iterOrder;
                    if((iterOrder=ti.orderLst.find(QString::fromStdString(iterIndex->second)))!=ti.orderLst.end()){
         //               iterOrder.value()->AvgPrice=(iterOrder->second.AvgPrice*(iterOrder->second.Qty-iterOrder->second.QtyLeft)+stTrade.Price*stTrade.Qty)/(iterOrder->second.Qty-iterOrder->second.QtyLeft+stTrade.Qty);
                        iterOrder.value()->VolumeTotal-=pTrade.Volume;
                        if(iterOrder.value()->VolumeTotal==0)
                            iterOrder.value()->OrderStatus=THOST_FTDC_OST_AllTraded;
                        strcpy(iterOrder.value()->InsertDate,pTrade.TradeDate);
                        strcpy(iterOrder.value()->UpdateTime,pTrade.TradeTime);
                    }
                }

                // 持仓处理
                QMap<QString, stSelfPosi>::iterator iterPosi;
                if ((pTrade.OffsetFlag != THOST_FTDC_OF_Open && pTrade.Direction == THOST_FTDC_D_Buy) || (pTrade.OffsetFlag == THOST_FTDC_OF_Open && pTrade.Direction == THOST_FTDC_D_Sell)) {
                    iterPosi = ti.shortPosis.find(pTrade.InstrumentID);
                    if (iterPosi == ti.shortPosis.end()) {
                        stSelfPosi stPosi = { 0 };
                        strcpy(stPosi.InstrumentID, pTrade.InstrumentID);
                        strcpy(stPosi.ExchangeID, pTrade.ExchangeID);
                        stPosi.BSFlag = THOST_FTDC_PD_Short;
                        ti.shortPosis[stPosi.InstrumentID] = stPosi;
                        iterPosi = ti.shortPosis.find(pTrade.InstrumentID);
                    }
                }
                else {
                    iterPosi = ti.longPosis.find(pTrade.InstrumentID);
                    if (iterPosi == ti.longPosis.end()) {
                        stSelfPosi stPosi = { 0 };
                        strcpy(stPosi.InstrumentID, pTrade.InstrumentID);
                        strcpy(stPosi.ExchangeID, pTrade.ExchangeID);
                        stPosi.BSFlag = THOST_FTDC_PD_Long;
                        ti.longPosis[stPosi.InstrumentID] = stPosi;
                        iterPosi = ti.longPosis.find(pTrade.InstrumentID);
                    }
                }


                if(m_IsInitEnd){
                    if(pTrade.Direction==THOST_FTDC_D_Buy){
                        if(pTrade.OffsetFlag==THOST_FTDC_OF_Open){
                            iterPosi.value().Price=(iterPosi.value().Price*iterPosi.value().Qty + pTrade.Volume*pTrade.Price)/(iterPosi.value().Qty+pTrade.Volume);
                            iterPosi.value().Qty+=pTrade.Volume;
                            iterPosi.value().TodayQty+=pTrade.Volume;
                        }else{
                            if((!strcmp(pTrade.ExchangeID,"SHFE") || !strcmp(pTrade.ExchangeID,"INE")) && pTrade.OffsetFlag==THOST_FTDC_OF_CloseToday){
                                iterPosi.value().TodayQty-=pTrade.Volume;
                                iterPosi.value().TodayQtyFrozen-=pTrade.Volume;
                            }
                            iterPosi.value().Qty-=pTrade.Volume;
                            iterPosi.value().QtyFrozen-=pTrade.Volume;
                            if(iterPosi.value().Qty==0){
                                iterPosi.value().Price=0;
                            }
                        }
                    }else{
                        if(pTrade.OffsetFlag==THOST_FTDC_OF_Open){
                            iterPosi.value().Price=(iterPosi.value().Price*iterPosi.value().Qty + pTrade.Volume*pTrade.Price)/(iterPosi.value().Qty+pTrade.Volume);
                            iterPosi.value().Qty+=pTrade.Volume;
                            iterPosi.value().TodayQty+=pTrade.Volume;
                        }else{
                            if((!strcmp(pTrade.ExchangeID,"SHFE") || !strcmp(pTrade.ExchangeID,"INE")) && pTrade.OffsetFlag==THOST_FTDC_OF_CloseToday){
                                iterPosi.value().TodayQty-=pTrade.Volume;
                                iterPosi.value().TodayQtyFrozen-=pTrade.Volume;
                            }
                            iterPosi.value().Qty-=pTrade.Volume;
                            iterPosi.value().QtyFrozen-=pTrade.Volume;
                            if(iterPosi.value().Qty==0){
                                iterPosi.value().Price=0;
                            }
                        }
                    }
                    QMap<QString, CThostFtdcInstrumentField *>::iterator iterInstrument=pTraderSpi->tempCons.find(pTrade.InstrumentID);
                    if(iterInstrument!=pTraderSpi->tempCons.end()){
                        if(iterPosi.value().BSFlag == THOST_FTDC_PD_Long){
                            iterPosi.value().Margin=iterPosi.value().Qty*iterPosi.value().Price*iterInstrument.value()->VolumeMultiple * iterInstrument.value()->LongMarginRatio +
                                pTrade.Volume*pTrade.Price*iterInstrument.value()->VolumeMultiple * iterInstrument.value()->LongMarginRatio;
                        }
                        else{
                            iterPosi.value().Margin=iterPosi.value().Qty*iterPosi.value().Price*iterInstrument.value()->VolumeMultiple * iterInstrument.value()->ShortMarginRatio +
                                pTrade.Volume*pTrade.Price*iterInstrument.value()->VolumeMultiple * iterInstrument.value()->ShortMarginRatio;
                        }
                    }
                }
                QMap<QString, stSelfPosi>::iterator iterPosiLog;
                // 多仓
                if((iterPosiLog = ti.longPosis.find(pTrade.InstrumentID)) != ti.longPosis.end()){
                    qInfo() << "RtnTrade-Pos-Update-Buy: " << iterPosiLog.value().Qty << iterPosiLog.value().QtyFrozen << iterPosiLog.value().TodayQty << iterPosiLog.value().TodayQtyFrozen;
                }
                // 空仓
                if ((iterPosiLog = ti.shortPosis.find(pTrade.InstrumentID)) != ti.shortPosis.end()) {
                    qInfo() << "RtnTrade-Pos-Update-Sell: " << iterPosiLog.value().Qty << iterPosiLog.value().QtyFrozen << iterPosiLog.value().TodayQty << iterPosiLog.value().TodayQtyFrozen;
                }
                break;
            }
        }
    }
}

void TradeWidget::onOrderInsetRspMsg(QString orderRef)
{
    QMapIterator<QString, TradeInfo> ii(tradeInfoLst);
    while (ii.hasNext())
    {
        TradeInfo & ti = tradeInfoLst[ii.next().key()];
        QMap<QString, CThostFtdcOrderField *>::iterator iterOrder=ti.orderLst.find(orderRef);
        if(iterOrder!=ti.orderLst.end()){
            if(iterOrder.value()->CombOffsetFlag[0]!=THOST_FTDC_OF_Open){
                QMap<QString, stSelfPosi>::iterator iterPosi;
                if(iterOrder.value()->Direction==THOST_FTDC_D_Buy){
                    iterPosi = ti.shortPosis.find(iterOrder.value()->InstrumentID);
                    if (iterPosi != ti.shortPosis.end()) {
                        if ((!strcmp(iterOrder.value()->ExchangeID, "SHFE") || !strcmp(iterOrder.value()->ExchangeID, "INE")) && iterOrder.value()->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday) {
                            iterPosi.value().TodayQtyFrozen -= iterOrder.value()->VolumeTotal;
                        }
                        iterPosi.value().QtyFrozen -= iterOrder.value()->VolumeTotal;
                    }
                }else{
                    iterPosi = ti.longPosis.find(iterOrder.value()->InstrumentID);
                    if (iterPosi != ti.longPosis.end()) {
                        if ((!strcmp(iterOrder.value()->ExchangeID, "SHFE") || !strcmp(iterOrder.value()->ExchangeID, "INE")) && iterOrder.value()->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday) {
                            iterPosi.value().TodayQtyFrozen -= iterOrder.value()->VolumeTotal;
                        }
                        iterPosi.value().QtyFrozen -= iterOrder.value()->VolumeTotal;
                    }
                }
            }
            iterOrder.value()->OrderStatus=THOST_FTDC_OST_Canceled;
        }
    }
}

void TradeWidget::onTimerReqPos()
{
    if(m_neddReqPos.size() <= 0 || !m_reqPosEndFlag)
        return;

    for( ; ; )
    {
        QThread::msleep(1000);
        int nRequestIDs = CreateNewRequestID();
        CThostFtdcQryInvestorPositionField reqInfo = {0};
        int ret = pTraderApi->ReqQryInvestorPosition(&reqInfo, nRequestIDs);
        if(ret == -2 || ret == -3)
            QThread::msleep(100);
        else{
            tradeInfoLst[loginW->userName].posiLst.clear();
            m_reqPosEndFlag = false;
            m_neddReqPos.removeFirst();
            break;
        }
    }
}

// 添加账户资金记录
void TradeWidget::addFunds(CTradeSpiImp * t, CThostFtdcTradingAccountField * fund)
{
    //mutex.lock();
    CThostFtdcTradingAccountField pFund = *fund;
    bool updateFlag = false;
    QMapIterator<QString, TradeInfo> i(tradeInfoLst);
    while (i.hasNext())
    {
        TradeInfo & ti = tradeInfoLst[i.next().key()];
        if(ti.spi == t)
        {
            ::memcpy(ti.fund, &pFund,sizeof(CThostFtdcTradingAccountField));
            updateFlag = true;
            // 需重新清算该资金账户的浮动盈亏
            ti.fund->PositionProfit = 0;
            QMapIterator<QString, PosiPloy *> poi(ti.posiLst);
            while(poi.hasNext())
            {
                QString keyP = poi.next().key();
                QList<CThostFtdcInvestorPositionField *> uposi;
                if(ti.posiLst[keyP])
                    uposi = ti.posiLst[keyP]->posi;
//                if(uposi.size() != 0 && ::memcmp(uposi->InvestorID,fund->AccountID,sizeof(TThostFtdcInvestorIDType)) == 0)
                if(uposi.size() > 0)
                {
                    for(int index = 0; index < uposi.size(); index++)
                        ti.fund->PositionProfit += uposi[index]->PositionProfit;
                }
            }
            updateStatusBar(ti.fund);
            break;
        }
    }
    delete fund;

//    m_timer.start(300);
}


void TradeWidget::addPosi(CTradeSpiImp * t, CThostFtdcInvestorPositionField * posi, bool bLast)
{
    if(bLast)
    {
        for( ; ; )
        {
            if(m_neddReqPos.size() > 0)
                break;

            QThread::msleep(1000);
            int nRequestIDs = CreateNewRequestID();
            CThostFtdcQryTradingAccountField reqInfo = {0};
            int ret = pTraderApi->ReqQryTradingAccount(&reqInfo, nRequestIDs);
            if(ret == -2 || ret == -3)
                QThread::msleep(100);
            else
                break;
        }
    }
    //mutex.lock();
    if(!posi)
        return;
    CThostFtdcInvestorPositionField & pPosi = *posi;
    // 绘制前先订阅
    int count = 1;
    char **InstrumentID = new char*[count];
    InstrumentID[0] = posi->InstrumentID;
    pQuotApi->SubscribeMarketData(InstrumentID, 1);
    insMap_dy[QString::fromLocal8Bit(posi->InstrumentID)] = insMap[QString::fromLocal8Bit(posi->InstrumentID)];
    delete[] InstrumentID;
    int scale = getScale(pPosi.InstrumentID);
    QMapIterator<QString, TradeInfo> i(tradeInfoLst);
    while (i.hasNext())
    {
        TradeInfo & ti = tradeInfoLst[i.next().key()];
        if(ti.spi == t)
        {
            PosiPloy * oltp = ti.posiLst[posi->InstrumentID];
            if(!oltp)
            {
                PosiPloy * pp = new PosiPloy;
                pp->trailPriceu = 0;
                pp->posi.append(posi);
                pp->points = 0;
                ti.posiLst[posi->InstrumentID] = pp;
//                CThostFtdcDepthMarketDataField * p = quotMap[QString::fromLocal8Bit(posi->InstrumentID)];
//				if(p && p->Price > 0)
//				{
//					double yk = insMap[QString::fromLocal8Bit(posi->InstrumentID)]->TradeUnit*((p->Price-posi->BuyPrice)*posi->BuyQty+(posi->SellPrice - p->Price)*posi->SellQty);
//					posi->FloatingPL = yk;
//				}
            }
            else
            {
                bool have = false;
                for(int index = 0; index<oltp->posi.size(); index++)
                {
                    CThostFtdcInvestorPositionField *pTemp = oltp->posi.at(index);
                    if(pTemp->PosiDirection == posi->PosiDirection){
                        pTemp->YdPosition += posi->YdPosition;
                        pTemp->Position += posi->Position;
                        pTemp->LongFrozen += posi->LongFrozen;
                        pTemp->ShortFrozen += posi->ShortFrozen;
                        pTemp->LongFrozenAmount += posi->LongFrozenAmount;
                        pTemp->ShortFrozenAmount += posi->ShortFrozenAmount;
                        pTemp->OpenVolume += posi->OpenVolume;
                        pTemp->CloseVolume += posi->CloseVolume;
                        pTemp->OpenAmount += posi->OpenAmount;
                        pTemp->CloseAmount += posi->CloseAmount;
                        pTemp->PositionCost += posi->PositionCost;
                        pTemp->UseMargin += posi->UseMargin;
                        pTemp->Commission += posi->Commission;
                        pTemp->CloseProfit += posi->CloseProfit;
                        pTemp->PositionProfit += posi->PositionProfit;
                        pTemp->CloseProfitByDate += posi->CloseProfitByDate;
                        pTemp->CloseProfitByTrade += posi->CloseProfitByTrade;
                        pTemp->FrozenMargin += posi->FrozenMargin;
                        pTemp->FrozenCash += posi->FrozenCash;
                        pTemp->FrozenCommission += posi->FrozenCommission;
                        oltp->posi.removeAt(index);
                        oltp->posi.append(pTemp);
                        have = true;
                        break;
                    }
                }
                if(!have)
                    oltp->posi.append(posi);
//                ::memcpy(oltp->posi, posi, sizeof(CThostFtdcInvestorPositionField));
//                CThostFtdcDepthMarketDataField * p = quotMap[QString::fromLocal8Bit(posi->InstrumentID)];
//				if(p && p->Price > 0)
//				{
//					double yk = insMap[QString::fromLocal8Bit(posi->InstrumentID)]->TradeUnit*((p->Price-posi->BuyPrice)*posi->BuyQty+(posi->SellPrice - p->Price)*posi->SellQty);
//					oltp->posi->FloatingPL = yk;
//				}
//				delete posi;
            }

            //new auto start
            if (posi) {
                stSelfPosi stPosi = {0};
                QMap<QString,stSelfPosi>::iterator iterPosi;
                if (posi->PosiDirection == THOST_FTDC_PD_Long) {
                    if ((iterPosi = ti.longPosis.find(QString(posi->InstrumentID))) == ti.longPosis.end()) {
                        strcpy(stPosi.InstrumentID, posi->InstrumentID);
                        strcpy(stPosi.ExchangeID, posi->ExchangeID);
                        stPosi.BSFlag = THOST_FTDC_PD_Long;
                        ti.longPosis[QString(posi->InstrumentID)] = stPosi;
                        iterPosi = ti.longPosis.find(QString(posi->InstrumentID));
                    }
                    iterPosi.value().Qty += posi->YdPosition;
                    iterPosi.value().Price = iterPosi.value().Qty?posi->PreSettlementPrice:0;
                } else {
                    if ((iterPosi = ti.shortPosis.find(QString(posi->InstrumentID))) == ti.shortPosis.end()) {
                        strcpy(stPosi.InstrumentID, posi->InstrumentID);
                        strcpy(stPosi.ExchangeID, posi->ExchangeID);
                        stPosi.BSFlag = THOST_FTDC_PD_Short;
                        ti.shortPosis[QString(posi->InstrumentID)] = stPosi;
                        iterPosi = ti.shortPosis.find(QString(posi->InstrumentID));
                    }
                    iterPosi.value().Qty += posi->YdPosition;
                    iterPosi.value().Price = iterPosi.value().Qty?posi->PreSettlementPrice:0;
                }
                iterPosi.value().Margin+=posi->PreMargin;
            }
            if(bLast){
                // 通过成交明细更新持仓
                for(auto iter=ti.tradeLst.begin();iter!=ti.tradeLst.end();iter++){
                    QMap<QString, CThostFtdcInstrumentField *>::iterator iterInstrument=pTraderSpi->tempCons.find(iter.value()->InstrumentID);
                    if(iterInstrument==pTraderSpi->tempCons.end()){
                        continue;
                    }
                    QMap<QString, stSelfPosi>::iterator iterPosi;
                    if(iter.value()->Direction == THOST_FTDC_D_Buy){
                        if(iter.value()->OffsetFlag == THOST_FTDC_OF_Open){
                            iterPosi = ti.longPosis.find(iter.value()->InstrumentID);
                            if (iterPosi == ti.longPosis.end()) {
                                stSelfPosi stPosi = { 0 };
                                strcpy(stPosi.InstrumentID, iter.value()->InstrumentID);
                                stPosi.BSFlag = THOST_FTDC_PD_Long;
                                ti.longPosis[stPosi.InstrumentID] = stPosi;
                                iterPosi = ti.longPosis.find(iter.value()->InstrumentID);
                            }
                            iterPosi.value().Price=(iterPosi.value().Price*iterPosi.value().Qty + iter.value()->Volume*iter.value()->Price)/(iterPosi.value().Qty + iter.value()->Volume);
                            iterPosi.value().Qty+=iter.value()->Volume;
                            iterPosi.value().TodayQty+=iter.value()->Volume;
                            iterPosi.value().Margin = iterPosi.value().Qty * iterPosi.value().Price * iterInstrument.value()->VolumeMultiple * iterInstrument.value()->LongMarginRatio;
                        }else{
                            iterPosi = ti.shortPosis.find(iter.value()->InstrumentID);
                            if (iterPosi == ti.shortPosis.end()) {
                                stSelfPosi stPosi = { 0 };
                                strcpy(stPosi.InstrumentID, iter.value()->InstrumentID);
                                stPosi.BSFlag = THOST_FTDC_PD_Short;
                                ti.shortPosis[stPosi.InstrumentID] = stPosi;
                                iterPosi = ti.shortPosis.find(iter.value()->InstrumentID);
                            }
                            if(iter.value()->OffsetFlag==THOST_FTDC_OF_CloseToday){
                                iterPosi.value().TodayQty-=iter.value()->Volume;
                            }
                            iterPosi.value().Qty-=iter.value()->Volume;
                            if(iterPosi.value().Qty==0){
                                iterPosi.value().Price=0;
                            }
                            iterPosi.value().Margin = iterPosi.value().Qty * iterPosi.value().Price * iterInstrument.value()->VolumeMultiple * iterInstrument.value()->ShortMarginRatio;
                        }
                    }else{
                        if(iter.value()->OffsetFlag == THOST_FTDC_OF_Open){
                            iterPosi = ti.shortPosis.find(iter.value()->InstrumentID);
                            if (iterPosi == ti.shortPosis.end()) {
                                stSelfPosi stPosi = { 0 };
                                strcpy(stPosi.InstrumentID, iter.value()->InstrumentID);
                                stPosi.BSFlag = THOST_FTDC_PD_Short;
                                ti.shortPosis[stPosi.InstrumentID] = stPosi;
                                iterPosi = ti.shortPosis.find(iter.value()->InstrumentID);
                            }
                            iterPosi.value().Price=(iterPosi.value().Price*iterPosi.value().Qty + iter.value()->Volume*iter.value()->Price)/(iterPosi.value().Qty+iter.value()->Volume);
                            iterPosi.value().Qty+=iter.value()->Volume;
                            iterPosi.value().TodayQty+=iter.value()->Volume;
                            iterPosi.value().Margin = iterPosi.value().Qty * iterPosi.value().Price * iterInstrument.value()->VolumeMultiple * iterInstrument.value()->ShortMarginRatio;
                        }else{
                            iterPosi = ti.longPosis.find(iter.value()->InstrumentID);
                            if (iterPosi == ti.longPosis.end()) {
                                stSelfPosi stPosi = { 0 };
                                strcpy(stPosi.InstrumentID, iter.value()->InstrumentID);
                                stPosi.BSFlag = THOST_FTDC_PD_Long;
                                ti.longPosis[stPosi.InstrumentID] = stPosi;
                                iterPosi = ti.longPosis.find(iter.value()->InstrumentID);
                            }
                            if(iter.value()->OffsetFlag==THOST_FTDC_OF_CloseToday){
                                iterPosi.value().TodayQty-=iter.value()->Volume;
                            }
                            iterPosi.value().Qty-=iter.value()->Volume;
                            if(iterPosi.value().Qty==0){
                                iterPosi.value().Price=0;
                            }
                            iterPosi.value().Margin = iterPosi.value().Qty * iterPosi.value().Price * iterInstrument.value()->VolumeMultiple * iterInstrument.value()->LongMarginRatio;
                        }
                    }
                }

                // 通过委托明细冻结持仓
                for(auto iter=ti.orderLst.begin();iter!=ti.orderLst.end();iter++){
                    QMap<QString, CThostFtdcInstrumentField *>::iterator iterInstrument=pTraderSpi->tempCons.find(iter.value()->InstrumentID);
                    if(iterInstrument==pTraderSpi->tempCons.end()){
                        continue;
                    }
                    if(iter.value()->OrderStatus==THOST_FTDC_OST_NoTradeQueueing || iter.value()->OrderStatus==THOST_FTDC_OST_PartTradedQueueing){
                        QMap<QString, stSelfPosi>::iterator iterPosi;
                        if (iter.value()->CombOffsetFlag[0] != THOST_FTDC_OF_Open) {
                            if (iter.value()->Direction == THOST_FTDC_D_Buy) {
                                iterPosi = ti.shortPosis.find(iter.value()->InstrumentID);
                                if (iterPosi == ti.shortPosis.end()) {
                                    stSelfPosi stPosi = { 0 };
                                    strcpy(stPosi.InstrumentID, iter.value()->InstrumentID);
                                    strcpy(stPosi.ExchangeID, iter.value()->ExchangeID);
                                    stPosi.BSFlag = THOST_FTDC_PD_Short;
                                    ti.shortPosis[stPosi.InstrumentID] = stPosi;
                                    iterPosi = ti.shortPosis.find(iter.value()->InstrumentID);
                                }
                            }
                            else {
                                iterPosi = ti.longPosis.find(iter.value()->InstrumentID);
                                if (iterPosi == ti.longPosis.end()) {
                                    stSelfPosi stPosi = { 0 };
                                    strcpy(stPosi.InstrumentID, iter.value()->InstrumentID);
                                    strcpy(stPosi.ExchangeID, iter.value()->ExchangeID);
                                    stPosi.BSFlag = THOST_FTDC_PD_Long;
                                    ti.longPosis[stPosi.InstrumentID] = stPosi;
                                    iterPosi = ti.longPosis.find(iter.value()->InstrumentID);
                                }
                            }
                        }

                        if(iter.value()->CombOffsetFlag[0] != THOST_FTDC_OF_Open){
                            if(iter.value()->Direction == THOST_FTDC_D_Buy){
                                if(iter.value()->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday){
                                    iterPosi.value().TodayQtyFrozen+=iter.value()->VolumeTotal;
                                }
                                iterPosi.value().QtyFrozen+=iter.value()->VolumeTotal;
                            }else{
                                if(iter.value()->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday){
                                    iterPosi.value().TodayQtyFrozen+=iter.value()->VolumeTotal;
                                }
                                iterPosi.value().QtyFrozen+=iter.value()->VolumeTotal;
                            }
                        }
                    }
                }
            }
            //new auto end
            break;
        }
    }

    m_reqPosEndFlag = true;
    //mutex.unlock();
    if(cmWidget && bLast)
    {
        cmWidget->update();
    }
}

void TradeWidget::comuWithTradeImp(TRADE_SIGNAL sig)
{
    msgLabel->setText("");
    switch(sig)
    {
    case LOGIN:
        loginW->show();
        break;
    case LOGOUT:
        loginW->show();
        break;
    default:
        break;
    }
}

// 异常信息处理
void TradeWidget::errManage(int code, QString mess)
{
    //关闭定时器
    // timeLimiter->stop();
    switch(code)
    {
        case 1005:
        {
            //QMessageBox::StandardButton button;
            //button = QMessageBox::question(this, QString::fromLocal8Bit("提示"), mess.append(QString::fromLocal8Bit("， 是否重新登录？")), QMessageBox::Yes | QMessageBox::No);
            //if (button == QMessageBox::No) {
            //	exit(0);
            //}
            //else if (button == QMessageBox::Yes) {
            //	loginW->show();
            //}
            Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, mess, false, QString::fromLocal8Bit("提示"), NULL, 0);
            nt.exec();
            loginW->show();
            break;
        }
        case 1003:
            {
                //QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), mess);
                Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, mess, false, QString::fromLocal8Bit("提示"), NULL, 0);
                nt.exec();
                exit(0);
                break;
            }
        default:
            {
                //QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), mess);
                Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, mess, false, QString::fromLocal8Bit("提示"), NULL, 0);
                nt.exec();
                break;
            }
    }
}

void TradeWidget::initTradeWin()
{
    ui.setupUi(this);

    ui.toolBar->hide();
    ui.menu_file->removeAction(ui.actionOrder);
    ui.menu_file->removeAction(ui.actionCM);
    ui.menu_file->removeAction(ui.priceAction);
    ui.menu_file->removeAction(ui.actionK);
    ui.menu->removeAction(ui.configAction);
    ui.menu->removeAction(ui.passwordAction);

    // 退出程序
    connect(ui.actionlogOut, SIGNAL(triggered(bool)), this, SLOT(close()), Qt::QueuedConnection);
    // 订单管理
    connect(ui.actionOrder, SIGNAL(triggered(bool)), this, SLOT(orderManage()), Qt::QueuedConnection);
    // 持仓管理
    connect(ui.actionCM, SIGNAL(triggered(bool)), this, SLOT(posiManage()), Qt::QueuedConnection);
    // 报价板
    connect(ui.priceAction, SIGNAL(triggered(bool)), this, SLOT(showPriceW()), Qt::QueuedConnection);
    // 传统下单界面
    /*connect(ui.tradeAction, SIGNAL(triggered(bool)), this, SLOT(showTradeW()), Qt::QueuedConnection);*/
    // 设置
    connect(ui.configAction, SIGNAL(triggered(bool)), this, SLOT(showConfW()), Qt::QueuedConnection);
    // 修改密码界面
    connect(ui.passwordAction, SIGNAL(triggered(bool)), this, SLOT(showChangePassword()), Qt::QueuedConnection);
    // 成交单
    connect(ui.dealAction, SIGNAL(triggered(bool)), this, SLOT(showDealW()), Qt::QueuedConnection);
    // 帮助
    connect(ui.helpAction, SIGNAL(triggered(bool)), this, SLOT(help()), Qt::QueuedConnection);
    // K线图
//	connect(ui.actionK, SIGNAL(triggered(bool)),this, SLOT(showCurve()), Qt::QueuedConnection);

    connect(loginW,SIGNAL(loginSec()),this, SLOT(tradingAftorLogin()), Qt::QueuedConnection);
    connect(loginW, SIGNAL(pushTradeToFrom(TRADE_SIGNAL)),this, SLOT(comuWithTradeImp(TRADE_SIGNAL)), Qt::QueuedConnection);
    connect(loginW, SIGNAL(errShow(int, QString)),this, SLOT(errManage(int, QString)), Qt::QueuedConnection);
    // API 信号槽
    connect(this, SIGNAL(getOrderPush(CTradeSpiImp *, CThostFtdcOrderField *, bool, bool)),this, SLOT(addOrder(CTradeSpiImp *, CThostFtdcOrderField *, bool, bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(getPosiPush(CTradeSpiImp *, CThostFtdcInvestorPositionField *, bool)),this, SLOT(addPosi(CTradeSpiImp *, CThostFtdcInvestorPositionField *, bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(getFundPush(CTradeSpiImp *, CThostFtdcTradingAccountField *)),this, SLOT(addFunds(CTradeSpiImp *, CThostFtdcTradingAccountField *)), Qt::QueuedConnection);
    connect(this, SIGNAL(getTradePush(CTradeSpiImp *, CThostFtdcTradeField *, bool, bool)),this, SLOT(addTrade(CTradeSpiImp *, CThostFtdcTradeField *, bool, bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(getInstrPush(CTradeSpiImp *)),this, SLOT(addInstr(CTradeSpiImp *)), Qt::QueuedConnection);
    connect(this, SIGNAL(cancelOrder(CThostFtdcInputOrderActionField *)),this, SLOT(doCancelOrder(CThostFtdcInputOrderActionField *)), Qt::QueuedConnection);
    connect(this, SIGNAL(tradeConnSec(CTradeSpiImp *)),this, SLOT(doTradeConnSec(CTradeSpiImp *)), Qt::QueuedConnection);
    connect(this, SIGNAL(orderInsertRspPush(QString)),this, SLOT(onOrderInsetRspMsg(QString)), Qt::QueuedConnection);

    // 订单消息处理
    connect(this, SIGNAL(orderMessage(QString)),this, SLOT(messageSrv(QString)), Qt::QueuedConnection);
}

TradeWidget::~TradeWidget()
{
    QFile file("./logIndex.txt");
    if(file.open(QIODevice::ReadWrite | QIODevice::Truncate)){
        file.write(QString(logInstance.fileNameIndex()).toLocal8Bit());
        file.close();
    }

    // 切断API推送
    disconnect(this, SIGNAL(getOrderPush(CTradeSpiImp *, CThostFtdcOrderField *, bool, bool)),this, SLOT(addOrder(CTradeSpiImp *, CThostFtdcOrderField *, bool, bool)));
    disconnect(this, SIGNAL(getPosiPush(CTradeSpiImp *, CThostFtdcInvestorPositionField *)),this, SLOT(addPosi(CTradeSpiImp *, CThostFtdcInvestorPositionField *)));
    disconnect(this, SIGNAL(getFundPush(CTradeSpiImp *, CThostFtdcTradingAccountField *)),this, SLOT(addFunds(CTradeSpiImp *, CThostFtdcTradingAccountField *)));
    disconnect(this, SIGNAL(getTradePush(CTradeSpiImp *, CThostFtdcTradeField *, bool, bool)),this, SLOT(addTrade(CTradeSpiImp *, CThostFtdcTradeField *, bool, bool)));
    disconnect(this, SIGNAL(getInstrPush(CTradeSpiImp *)),this, SLOT(addInstr(CTradeSpiImp *)));
    disconnect(this, SIGNAL(getPpricePush(CThostFtdcDepthMarketDataField *)),this, SLOT(addPrice(CThostFtdcDepthMarketDataField *)));
    disconnect(this, SIGNAL(cancelOrder(CThostFtdcInputOrderActionField *)),this, SLOT(doCancelOrder(CThostFtdcInputOrderActionField *)));
    disconnect(this, SIGNAL(tradeConnSec(CTradeSpiImp *)),this, SLOT(doTradeConnSec(CTradeSpiImp *)));
    disconnect(this, SIGNAL(orderInsertRspPush(QString)),this, SLOT(onOrderInsetRspMsg(QString)));

    exit(0);
}

void TradeWidget::slot_showOrderWin(TThostFtdcInstrumentIDType m_strInstr)
{
    if(!pTraderApi)
    {
        //QMessageBox::about(NULL,"提示","未登录，请先登录！");
        Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("未登录，请先登录！"), false, QString::fromLocal8Bit("提示"), NULL, 0);
        nt.exec();
        return;
    }
    QString m_strInstrument = QString(m_strInstr);
    CThostFtdcDepthMarketDataField * t = quotMap[m_strInstrument];
    if(!t)
    {
        //QMessageBox::about(NULL,"提示","缺少行情，暂停下单！");
        Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("缺少行情，暂停下单！"), false, QString::fromLocal8Bit("提示"), NULL, 0);
        nt.exec();
        return;
    }
}

// 订单消息处理
void TradeWidget::messageSrv(QString mes)
{
    //int lins = 11;
    //while(lins < mes.length())
    //{
    //	mes = mes.insert(lins,"\r\n");
    //	lins += 13;
    //}
//    QWidget * messWin = new QWidget();
//    messWin->setWindowFlags(Qt::Popup);
//    messWin->setWindowFlags(Qt::FramelessWindowHint);
//    messWin->setAttribute(Qt::WA_TranslucentBackground);
//    QLabel * backL = new QLabel(messWin);
//    backL->setPixmap(QPixmap(":/image/images/notice.png"));
//    QLabel * mesL = new QLabel(messWin);
//    mesL->setText(mes);
//    const QRect wR = QApplication::desktop()->screenGeometry();
//    messWin->setGeometry(wR.right()-140,wR.bottom()-140,120,120);
//    mesL->setGeometry(0,0,messWin->width(),messWin->height()-20);
//    backL->setGeometry(0,0,messWin->width(),messWin->height());
//    mesL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//    messWin->show();
//    QTimer::singleShot(3000, messWin, SLOT(close()));
    loginStatusLbl->setText(mes);
    m_timerFreshTips.start(5000);
}

void TradeWidget::onTimerClearTips()
{
    loginStatusLbl->clear();
}

void TradeWidget::orderManage()
{
    // QMessageBox::about(NULL,"提示","建设中！");
    if(!omWidget)
        omWidget = new OrderManage(this);
    omWidget->setGeometry(DW/2-215,DH/2-125,430,250);
    omWidget->show();
//#ifdef WIN32
//    ::SetWindowPos(HWND(omWidget->winId()), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
//#endif
}

// 持仓管理
void TradeWidget::posiManage()
{
    // QMessageBox::about(NULL,"提示","建设中！");
    if(!cmWidget)
        cmWidget = new PosiManage(this);
    cmWidget->setGeometry(DW/2-215,DH/2-125,430,250);
    cmWidget->show();
//#ifdef WIN32
//	::SetWindowPos(HWND(cmWidget->winId()), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
//#endif
}

// 报价版展示
void TradeWidget::showPriceW()
{
    // QMessageBox::about(NULL,"提示","报价板建设中！");
    if(quotMap.count() == 0)
    {
        // 数量不能为0
        Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("暂缺行情！"), false, QString::fromLocal8Bit("提示"), NULL, 0);
        nt.exec();
        return;
    }
    if(!pview)
        pview = new PriceView(this);
    pview->setGeometry(DW/2-430/2,DH/2-125,430,250);
    pview->show();
//#ifdef WIN32
//	::SetWindowPos(HWND(pview->winId()), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
//#endif
}


// 打开传统下单界面
//void TradeWidget::showTradeW()
//{
//	if(this->CSubmit)
//		showTradeW(this->CSubmit->selectInstr->curInstr);
//}

// 传统下单界面展示
//void TradeWidget::showTradeW(CThostFtdcInstrumentField *s)
//{
//	//QMessageBox::about(NULL,"提示","传统下单界面建设中！");
//	if(quotMap[s->InstrumentID] == 0)
//	{
//		return;
//	}
//	if(!oview)
//	oview = new OrderView(this);
//	oview->selectInstr->curInstr = s;
//	oview->setGeometry(DW/2-180,DH/2-85,360,170);
//	oview->show();
//	::SetWindowPos(HWND(oview->winId()), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
//}

// 打开设置界面
void TradeWidget::showConfW()
{
    if(!cview)
    cview = new configWidget(this);
    cview->setGeometry(DW/2-165,DH/2-100,330,200);
    cview->initByPrivats();
    cview->show();
//#ifdef WIN32
//	::SetWindowPos(HWND(cview->winId()), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
//#endif
}

// 打开修改密码界面
void TradeWidget::showChangePassword()
{
    if(!cpView)
    cpView = new changePassword(NULL);
    cpView->setGeometry(DW/2-165,DH/2-100,330,200);
    cpView->show();
//#ifdef WIN32
//	::SetWindowPos(HWND(cpView->winId()), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
//#endif
}

// 成交单界面
void TradeWidget::showDealW() {
    if (!dview)
        dview = new dealView(this);
    dview->setGeometry(DW/2-180,DH/2-125,360,250);
    dview->show();
//#ifdef WIN32
//    ::SetWindowPos(HWND(dview->winId()), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
//#endif
}

// 帮助
void TradeWidget::help()
{
    Help * hp = new Help(this);
    hp->setGeometry(DW/2-200,DH/2-124,400,248);
    hp->show();
}

// 展示K线图
void TradeWidget::showCurve()
{
    if(!CSubmit)
    {
        Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("主窗口构建中！"), false, QString::fromLocal8Bit("提示"), NULL, 0);
        nt.exec();
        return;
    }
    CThostFtdcInstrumentField * ci = CSubmit->selectInstr->curInstr;
//	if(!ci || !pHisApi)
    if(!ci)
    {
        Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("主窗口构建中！"), false, QString::fromLocal8Bit("提示"), NULL, 0);
        nt.exec();
        return;
    }
//	CurveUnit * CUnit = new CurveUnit(ci);
//	CUnit->setGeometry(DW/2-215,DH/2-125,430,250);
//	CUnit->setActionm5();
//	CUnitList.append(CUnit);
//	CUnit->show();
}
