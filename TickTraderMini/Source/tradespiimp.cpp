#include "tradespiimp.h"
#include "tradewidget.h"
#include <QDebug>
#include <QThread>

extern loginWin *loginW;
extern TradeWidget *g_tw;
extern quint32 CreateNewRequestID();

CTradeSpiImp::CTradeSpiImp(CThostFtdcTraderApi* pTraderApi)
    : m_pTradeApi( pTraderApi ),
      loginFlags(false)
{
}

CTradeSpiImp::~CTradeSpiImp()
{

}

void CTradeSpiImp::OnFrontConnected()
{

//    CThostFtdcInstrumentField *pIn = new CThostFtdcInstrumentField;
//    memcpy(pIn->ExchangeID, "SHFE", sizeof(pIn->ExchangeID));
//    memcpy(pIn->InstrumentID, "ag2301", sizeof(pIn->InstrumentID));
//    QString ins = QString("ag2301");
//    tempCons[ins] = pIn;
//    g_tw->instrEmit(this);
//    loginW->loginSucceed();
//    return;

    qInfo() << "CTradeSpiImp::OnFrontConnected";
    if(!loginFlags)
    {
        CThostFtdcReqUserLoginField login = {0};
        strncpy( login.BrokerID, loginW->m_users.BrokerID, sizeof(login.BrokerID));
        strncpy( login.UserID, loginW->userName, sizeof(login.UserID));
        strncpy( login.Password, loginW->password, sizeof(login.Password));
        strcpy( login.UserProductInfo, "TICKTRADER" );
        strcpy( login.MacAddress, "fe80::75ef:97de:2366:e490" );
        strcpy( login.ClientIPAddress, "10.150.1.23" );

        quint32 nRequestID = CreateNewRequestID();
        int ret = m_pTradeApi->ReqUserLogin(&login, nRequestID);
        qInfo() << "ReqUserLogin ret: " << ret;
    }
}

void CTradeSpiImp::OnFrontDisconnected()
{
    qInfo() << "CTradeSpiImp::OnFrontDisconnected";
}

void CTradeSpiImp::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    qInfo() << "CTradeSpiImp::OnRspUserLogin" << pRspInfo->ErrorID << pRspInfo->ErrorMsg;
    if(pRspInfo && pRspInfo->ErrorID != 0)
    {
        loginW->loginFailed(pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg));
        return;
    }
    if(!loginFlags && pRspUserLogin)
    {
        loginFlags = true;
        memcpy(&loginW->loginRes, pRspUserLogin, sizeof(pRspUserLogin));

        int nRequestIDs = CreateNewRequestID();
        CThostFtdcQryInstrumentField reqInfo = {0};
        m_pTradeApi->ReqQryInstrument(&reqInfo, nRequestIDs);
    }
    if(!loginW->isHidden())
        emit loginW->showProcess(ShowProc_loginOk, true);
}

void CTradeSpiImp::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo && pRspInfo->ErrorID != 0)
    {
        loginW->loginFailed(pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg));
        return;
    }

    loginW->logutSecced();
}

void CTradeSpiImp::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(!pUserPasswordUpdate)
        return;
    if(pRspInfo && pRspInfo->ErrorID == 0)
    {
        g_tw->orderMessageEmit(QString::fromLocal8Bit(pRspInfo->ErrorMsg));
    }
}

void CTradeSpiImp::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(!pInputOrder)
        return;
    if(pRspInfo && pRspInfo->ErrorID)
    {
        qInfo() << "OnRspOrderInsert: " << pRspInfo->ErrorID << QString::fromLocal8Bit(pRspInfo->ErrorMsg);
        g_tw->orderMessageEmit(QString::fromLocal8Bit(pRspInfo->ErrorMsg));
    }
}

void CTradeSpiImp::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if(pOrder)
    {
        qInfo() << "OnRtnOrder: " << pOrder->OrderRef << QString::fromLocal8Bit(pOrder->StatusMsg) << pOrder->OrderStatus
                << pOrder->OrderSubmitStatus;
        g_tw->orderEmit(this, pOrder, false, true);
    }
}

void CTradeSpiImp::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    if(pTrade)
    {
        g_tw->tradeEmit(this, pTrade, false);
    }
}

void CTradeSpiImp::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(!pInputOrderAction)
        return;
    if(pRspInfo && pRspInfo->ErrorID)
    {
        qInfo() << QString::fromLocal8Bit(pRspInfo->ErrorMsg);
        g_tw->orderMessageEmit(QString::fromLocal8Bit(pRspInfo->ErrorMsg));
    }
}

void CTradeSpiImp::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo && pRspInfo->ErrorID!=0){
        return;
    }
    if(!pDepthMarketData && bIsLast)
        return;
    if(pDepthMarketData)
    {
        CThostFtdcDepthMarketDataField * mp = g_tw->quotMap[QString::fromLocal8Bit(pDepthMarketData->InstrumentID)];
        if(mp && mp->LastPrice > 0.0001)
            return;
        g_tw->priceEmit(pDepthMarketData);
    }
}

void CTradeSpiImp::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    qInfo() << "OnRspQryOrder: " << bIsLast << (pOrder == nullptr ? 1:0);
    if(pRspInfo && pRspInfo->ErrorID != 0)
        return;

    if(pOrder && !bIsLast)
    {
        qInfo() << "OnRspQryOrder: " << pOrder->OrderRef << pOrder->InstrumentID << bIsLast << QString::fromLocal8Bit(pOrder->StatusMsg);
        g_tw->orderEmit(this, pOrder);
    }
	if(bIsLast)
		g_tw->orderEmit(this, pOrder, bIsLast);
    if(!loginW->isHidden())
        emit loginW->showProcess(ShowProc_order, bIsLast);
}

void CTradeSpiImp::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    qInfo() << "OnRspQryTrade: " << bIsLast;
    if(pRspInfo && pRspInfo->ErrorID != 0)
        return;
    if(pTrade && !bIsLast)
    {
        qInfo() << "OnRspQryTrade: " << pTrade->OrderRef << pTrade->InstrumentID << bIsLast;
        g_tw->tradeEmit(this, pTrade);
    }
	if(bIsLast)
		g_tw->tradeEmit(this, pTrade, bIsLast);
    if(!loginW->isHidden())
        emit loginW->showProcess(ShowProc_trade, bIsLast);
}

void CTradeSpiImp::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
 //   qInfo() << "OnRspQryInvestorPosition: " << bIsLast;
    if(pInvestorPosition)
        qInfo() << "OnRspQryInvestorPosition: " << pInvestorPosition->InstrumentID << pInvestorPosition->ExchangeID
                << pInvestorPosition->PosiDirection << pInvestorPosition->YdPosition
                << pInvestorPosition->Position << pInvestorPosition->LongFrozen
                << pInvestorPosition->ShortFrozen << pInvestorPosition->PositionProfit;

    if(pInvestorPosition && !bIsLast)
    {
        g_tw->posiEmit(this, pInvestorPosition);
    }
	if(bIsLast)
		g_tw->posiEmit(this, pInvestorPosition, bIsLast);
    if(!loginW->isHidden())
        emit loginW->showProcess(ShowProc_position, bIsLast);
}

void CTradeSpiImp::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    qInfo() << "OnRspQryTradingAccount: " << bIsLast;
    if(pRspInfo && pRspInfo->ErrorID != 0)
        return;
    qInfo() << "OnRspQryTradingAccount: " << pTradingAccount->Available << bIsLast;
    if(pTradingAccount)
    {
        g_tw->fundEmit(this, pTradingAccount);
    }
    if(!loginW->isHidden())
        emit loginW->showProcess(ShowProc_account, bIsLast);
}

void CTradeSpiImp::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo && pRspInfo->ErrorID!=0){
        return;
    }
    qInfo() << "pInstrument: " << pInstrument->InstrumentName << bIsLast;
    if(pInstrument)
    {
        CThostFtdcInstrumentField *pIn = new CThostFtdcInstrumentField;
        ::memcpy(pIn, pInstrument, sizeof(CThostFtdcInstrumentField));
        QString ins = QString(pInstrument->InstrumentID);
        tempCons[ins] = pIn;

        if(!loginW->isHidden())
            emit loginW->showProcess(ShowProc_instrument, bIsLast);
    }

    if(bIsLast)
    {
        g_tw->instrEmit(this);
        loginW->loginSucceed();

//        CThostFtdcSettlementInfoConfirmField sicf = { 0 };
//        memcpy( sicf.BrokerID, loginW->m_users.BrokerID, sizeof( sicf.BrokerID ) );
//        memcpy( sicf.InvestorID, loginW->userName, sizeof( sicf.InvestorID ) );
//        strcpy( sicf.ConfirmDate, m_pTradeApi->GetTradingDay( ) );

//        for( ; ; )
//        {
//            int iRequestID = CreateNewRequestID( );
//            int iResult = m_pTradeApi->ReqSettlementInfoConfirm( &sicf, iRequestID );
//            if(iResult == -2 || iResult == -3)
//                QThread::msleep(100);
//            else
//                break;
//        }

//        QThread::msleep(1000);

        for( ; ;)
        {
            QThread::msleep(500);
            int nRequestIDs = CreateNewRequestID();
            CThostFtdcQryOrderField reqInfo = {0};
            int ret = m_pTradeApi->ReqQryOrder(&reqInfo, nRequestIDs);
            qInfo() << "ReqQryOrder: " << ret;
            if(ret == -2 || ret == -3)
                QThread::msleep(100);
            else
                break;
        }
    }
}

void CTradeSpiImp::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}
