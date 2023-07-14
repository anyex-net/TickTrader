#include "tradespiimp.h"
#include "tradewidget.h"
#include <QDebug>
#include <QThread>
#include "loginWin.h"

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
        int nRequestIDs = CreateNewRequestID();
        CThostFtdcReqAuthenticateField reqInfo = {0};
        strncpy( reqInfo.BrokerID, loginW->brokerID, sizeof(reqInfo.BrokerID));
        strncpy( reqInfo.UserID, loginW->userName, sizeof(reqInfo.UserID));
        strncpy( reqInfo.UserProductInfo, "TickTrader", sizeof(reqInfo.UserProductInfo));
        strncpy( reqInfo.AuthCode, loginW->authCode, sizeof(reqInfo.AuthCode));
        strncpy( reqInfo.AppID, loginW->appID, sizeof(reqInfo.AppID));
        int ret = m_pTradeApi->ReqAuthenticate(&reqInfo, nRequestIDs);
        qInfo() << "ReqAuthenticate: " << ret << reqInfo.BrokerID << reqInfo.UserID << reqInfo.UserProductInfo
                << reqInfo.AuthCode << reqInfo.AppID;

//        QThread::msleep(2000);
//        CThostFtdcReqUserLoginField login = {0};
//        strncpy( login.BrokerID, loginW->brokerID, sizeof(login.BrokerID));
//        strncpy( login.UserID, loginW->userName, sizeof(login.UserID));
//        strncpy( login.Password, loginW->password, sizeof(login.Password));
//        strcpy( login.UserProductInfo, "TickTrader" );
//        strcpy( login.MacAddress, "fe80:75ef:97de:2366:e490" );
//        strcpy( login.ClientIPAddress, "10.150.1.23" );

//        int nRequestID = CreateNewRequestID();
//        ret = m_pTradeApi->ReqUserLogin(&login, nRequestID);
//        qInfo() << "ReqUserLogin ret: " << ret;
    }
}

void CTradeSpiImp::OnFrontDisconnected()
{
    qInfo() << "CTradeSpiImp::OnFrontDisconnected";
}

void CTradeSpiImp::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    qInfo() << "CTradeSpiImp::OnRspUserLogin" << pRspInfo->ErrorID << QString::fromLocal8Bit(pRspInfo->ErrorMsg);
    if(pRspInfo && pRspInfo->ErrorID != 0)
    {
        loginW->loginFailed(pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg));
        return;
    }
    if(!loginFlags && pRspUserLogin)
    {
        loginFlags = true;
        memcpy(&loginW->loginRes, pRspUserLogin, sizeof(pRspUserLogin));

        qInfo() << "OnRspUserLogin: " << "FrontID: " << pRspUserLogin->FrontID << "SessionID: " << pRspUserLogin->SessionID;

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

void CTradeSpiImp::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    qInfo() << "CTradeSpiImp::OnRspAuthenticate" << pRspInfo->ErrorID << QString::fromLocal8Bit(pRspInfo->ErrorMsg);
    if(pRspInfo && pRspInfo->ErrorID != 0)
    {
        loginW->loginFailed(pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg));
        return;
    }
    if(pRspAuthenticateField)
    {
        //change password
//        int nRequestID = CreateNewRequestID();
//        CThostFtdcUserPasswordUpdateField pwd = {0};
//        strncpy( pwd.BrokerID, loginW->brokerID, sizeof(pwd.BrokerID));
//        strncpy( pwd.UserID, loginW->userName, sizeof(pwd.UserID));
//        strncpy( pwd.OldPassword, loginW->password, sizeof(pwd.OldPassword));
//        strncpy( pwd.NewPassword, "ticktrader#123", sizeof(pwd.NewPassword));
//        int ret = m_pTradeApi->ReqUserPasswordUpdate(&pwd, nRequestID);
//        qInfo() << "ReqUserPasswordUpdate ret: " << ret << pwd.BrokerID << pwd.UserID << pwd.OldPassword << pwd.NewPassword;
//        return;
        //

        CThostFtdcReqUserLoginField login = {0};
        strncpy( login.BrokerID, loginW->brokerID, sizeof(login.BrokerID));
        strncpy( login.UserID, loginW->userName, sizeof(login.UserID));
        strncpy( login.Password, loginW->password, sizeof(login.Password));
        strcpy( login.UserProductInfo, "TickTrader" );
        strcpy( login.MacAddress, "fe80:75ef:97de:2366:e490" );
        strcpy( login.ClientIPAddress, "10.150.1.23" );

        int nRequestID1 = CreateNewRequestID();
        int ret1 = m_pTradeApi->ReqUserLogin(&login, nRequestID1);
        qInfo() << "CTradeSpiImp::ReqUserLogin ret: " << ret1;
    }
}

void CTradeSpiImp::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    qInfo() << "CTradeSpiImp::OnRspUserPasswordUpdate" << pRspInfo->ErrorID << QString::fromLocal8Bit(pRspInfo->ErrorMsg);
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
        qInfo() << "OnRspOrderInsert: \n" << pRspInfo->ErrorID << QString::fromLocal8Bit(pRspInfo->ErrorMsg) << "\n"
                << "OrderRef: " << pInputOrder->OrderRef << "    OrderPriceType: " << pInputOrder->OrderPriceType << "\n"
                << "Direction: " << pInputOrder->Direction << "    CombOffsetFlag: " << pInputOrder->CombOffsetFlag << "\n"
                << "CombHedgeFlag: " << pInputOrder->CombHedgeFlag << "    LimitPrice: " << pInputOrder->LimitPrice << "\n"
                << "VolumeTotalOriginal: " << pInputOrder->VolumeTotalOriginal << "    TimeCondition: " << pInputOrder->TimeCondition << "\n"
                << "VolumeCondition: " << pInputOrder->VolumeCondition << "    ExchangeID: " << pInputOrder->ExchangeID << "\n"
                << "InstrumentID: " << pInputOrder->InstrumentID << "\n";
        QString orderKey = QString("%1.%2.%3").arg(loginW->loginRes.FrontID).arg(loginW->loginRes.SessionID).arg(pInputOrder->OrderRef);
        g_tw->orderMessageEmit(QString::fromLocal8Bit(pRspInfo->ErrorMsg));
        g_tw->orderInsertRsp(orderKey);
    }
}

void CTradeSpiImp::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if(pOrder)
    {
        qInfo() << "OnRtnOrder: \n"
                << "OrderRef: " << pOrder->OrderRef << "    OrderPriceType: " << pOrder->OrderPriceType << "\n"
                << "Direction: " << pOrder->Direction << "    CombOffsetFlag: " << pOrder->CombOffsetFlag << "\n"
                << "CombHedgeFlag: " << pOrder->CombHedgeFlag << "    LimitPrice: " << pOrder->LimitPrice << "\n"
                << "VolumeTotalOriginal: " << pOrder->VolumeTotalOriginal << "    TimeCondition: " << pOrder->TimeCondition << "\n"
                << "VolumeCondition: " << pOrder->VolumeCondition << "    ExchangeID: " << pOrder->ExchangeID << "\n"
                << "InstrumentID: " << pOrder->InstrumentID << "    OrderSysID: " << pOrder->OrderSysID << "\n"
                << "OrderStatus: " << pOrder->OrderStatus << "    OrderSubmitStatus: " << pOrder->OrderSubmitStatus << "\n"
                << "OrderType: " << pOrder->OrderType << "    VolumeTraded: " << pOrder->VolumeTraded << "\n"
                << "VolumeTotal: " << pOrder->VolumeTotal << "    InsertDate: " << pOrder->InsertDate << "\n"
                << "InsertTime: " << pOrder->InsertTime << "    FrontID: " << pOrder->FrontID << "\n"
                << "SessionID: " << pOrder->SessionID << "    StatusMsg: " << QString::fromLocal8Bit(pOrder->StatusMsg) << "\n";
        g_tw->orderEmit(this, pOrder, false, true);
    }
}

void CTradeSpiImp::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    if(pTrade)
    {
        qInfo() << "OnRtnTrade: \n"
             << "OrderRef: " << pTrade->OrderRef << "    ExchangeID: " << pTrade->ExchangeID << "\n"
             << "TradeID: " << pTrade->TradeID << "    Direction: " << pTrade->Direction << "\n"
             << "OrderSysID: " << pTrade->OrderSysID << "    OffsetFlag: " << pTrade->OffsetFlag << "\n"
             << "HedgeFlag: " << pTrade->HedgeFlag << "    Price: " << pTrade->Price << "\n"
             << "Volume: " << pTrade->Volume << "    TradeDate: " << pTrade->TradeDate << "\n"
             << "TradeTime: " << pTrade->TradeTime << "    InstrumentID: " << pTrade->InstrumentID << "\n";
        g_tw->tradeEmit(this, pTrade, false, true);
    }
}

void CTradeSpiImp::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(!pInputOrderAction)
        return;
    if(pRspInfo && pRspInfo->ErrorID)
    {
        qInfo() << "OnRspOrderAction: \n"
             << "OrderRef: " << pInputOrderAction->OrderRef << "    ExchangeID: " << pInputOrderAction->ExchangeID << "\n"
             << "FrontID: " << pInputOrderAction->FrontID << "    SessionID: " << pInputOrderAction->SessionID << "\n"
             << "OrderSysID: " << pInputOrderAction->OrderSysID << "    ActionFlag: " << pInputOrderAction->ActionFlag << "\n"
             << "LimitPrice: " << pInputOrderAction->LimitPrice << "    VolumeChange: " << pInputOrderAction->VolumeChange << "\n"
             << "InstrumentID: " << pInputOrderAction->InstrumentID << "\n";
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
    if(pRspInfo && pRspInfo->ErrorID != 0)
        return;
    if(pOrder){
        qInfo() << "OnRspQryOrder: \n"
                << "OrderRef: " << pOrder->OrderRef << "    OrderPriceType: " << pOrder->OrderPriceType << "\n"
                << "Direction: " << pOrder->Direction << "    CombOffsetFlag: " << pOrder->CombOffsetFlag << "\n"
                << "CombHedgeFlag: " << pOrder->CombHedgeFlag << "    LimitPrice: " << pOrder->LimitPrice << "\n"
                << "VolumeTotalOriginal: " << pOrder->VolumeTotalOriginal << "    TimeCondition: " << pOrder->TimeCondition << "\n"
                << "VolumeCondition: " << pOrder->VolumeCondition << "    ExchangeID: " << pOrder->ExchangeID << "\n"
                << "InstrumentID: " << pOrder->InstrumentID << "    OrderSysID: " << pOrder->OrderSysID << "\n"
                << "OrderStatus: " << pOrder->OrderStatus << "    OrderSubmitStatus: " << pOrder->OrderSubmitStatus << "\n"
                << "OrderType: " << pOrder->OrderType << "    VolumeTraded: " << pOrder->VolumeTraded << "\n"
                << "VolumeTotal: " << pOrder->VolumeTotal << "    InsertDate: " << pOrder->InsertDate << "\n"
                << "InsertTime: " << pOrder->InsertTime << "    FrontID: " << pOrder->FrontID << "\n"
                << "SessionID: " << pOrder->SessionID << "    StatusMsg: " << QString::fromLocal8Bit(pOrder->StatusMsg) << "\n";
    }
    if(pOrder && !bIsLast)
    {
        g_tw->orderEmit(this, pOrder);
    }
	if(bIsLast)
		g_tw->orderEmit(this, pOrder, bIsLast);
    if(!loginW->isHidden())
        emit loginW->showProcess(ShowProc_order, bIsLast);
}

void CTradeSpiImp::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo && pRspInfo->ErrorID != 0)
        return;
    if(pTrade){
        qInfo() << "OnRspQryTrade: \n"
             << "OrderRef: " << pTrade->OrderRef << "    ExchangeID: " << pTrade->ExchangeID << "\n"
             << "TradeID: " << pTrade->TradeID << "    Direction: " << pTrade->Direction << "\n"
             << "OrderSysID: " << pTrade->OrderSysID << "    OffsetFlag: " << pTrade->OffsetFlag << "\n"
             << "HedgeFlag: " << pTrade->HedgeFlag << "    Price: " << pTrade->Price << "\n"
             << "Volume: " << pTrade->Volume << "    TradeDate: " << pTrade->TradeDate << "\n"
             << "TradeTime: " << pTrade->TradeTime << "    InstrumentID: " << QString::fromLocal8Bit(pTrade->InstrumentID) << "\n";
    }
    if(pTrade && !bIsLast)
    {
        g_tw->tradeEmit(this, pTrade);
    }
	if(bIsLast)
		g_tw->tradeEmit(this, pTrade, bIsLast);
    if(!loginW->isHidden())
        emit loginW->showProcess(ShowProc_trade, bIsLast);
}

void CTradeSpiImp::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pInvestorPosition)
        qInfo() << "OnRspQryInvestorPosition: \n"
                << "InstrumentID: " << pInvestorPosition->InstrumentID << "    ExchangeID: " << pInvestorPosition->ExchangeID << "\n"
                << "PosiDirection: " << pInvestorPosition->PosiDirection << "    HedgeFlag: " << pInvestorPosition->HedgeFlag << "\n"
                << "PositionDate: " << pInvestorPosition->PositionDate << "    YdPosition: " << pInvestorPosition->YdPosition << "\n"
                << "Position: " << pInvestorPosition->Position << "    LongFrozen: " << pInvestorPosition->LongFrozen << "\n"
                << "ShortFrozen: " << pInvestorPosition->ShortFrozen << "    OpenVolume: " << pInvestorPosition->OpenVolume << "\n"
                << "CloseVolume: " << pInvestorPosition->CloseVolume << "    OpenAmount: " << pInvestorPosition->OpenAmount << "\n"
                << "CloseAmount: " << pInvestorPosition->CloseAmount << "    PositionCost: " << pInvestorPosition->PositionCost << "\n"
                << "PreMargin: " << pInvestorPosition->PreMargin << "    UseMargin: " << pInvestorPosition->UseMargin << "\n"
                << "FrozenMargin: " << pInvestorPosition->FrozenMargin << "    FrozenCash: " << pInvestorPosition->FrozenCash << "\n"
                << "FrozenCommission: " << pInvestorPosition->FrozenCommission << "    Commission: " << pInvestorPosition->Commission << "\n"
                << "CloseProfit: " << pInvestorPosition->CloseProfit << "    PositionProfit: " << pInvestorPosition->PositionProfit << "\n"
                << "PreSettlementPrice: " << pInvestorPosition->PreSettlementPrice << "    SettlementPrice: " << pInvestorPosition->SettlementPrice << "\n"
                << "CloseProfitByDate: " << pInvestorPosition->CloseProfitByDate << "    CloseProfitByTrade: " << pInvestorPosition->CloseProfitByTrade << "\n"
                << "TodayPosition: " << pInvestorPosition->TodayPosition << "    InstrumentID: " << pInvestorPosition->InstrumentID << "\n";

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
    if(pRspInfo && pRspInfo->ErrorID != 0)
        return;
    if(pTradingAccount)
    {
        qInfo() << "OnRspQryTradingAccount: \n"
                << "CurrMargin: " << pTradingAccount->CurrMargin << "    Commission: " << pTradingAccount->Commission << "\n"
                << "CloseProfit: " << pTradingAccount->CloseProfit << "    PositionProfit: " << pTradingAccount->PositionProfit << "\n"
                << "Available: " << pTradingAccount->Available << "    WithdrawQuota: " << pTradingAccount->WithdrawQuota << "\n";
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
    qInfo() << "OnRspQryInstrument: " << QString::fromLocal8Bit(pInstrument->InstrumentName) << bIsLast;
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

        CThostFtdcSettlementInfoConfirmField sicf = { 0 };
        memcpy( sicf.BrokerID, loginW->brokerID, sizeof( sicf.BrokerID ) );
        memcpy( sicf.InvestorID, loginW->userName, sizeof( sicf.InvestorID ) );
        strcpy( sicf.ConfirmDate, m_pTradeApi->GetTradingDay( ) );

        for( ; ; )
        {
            int iRequestID = CreateNewRequestID( );
            int iResult = m_pTradeApi->ReqSettlementInfoConfirm( &sicf, iRequestID );
            if(iResult == -2 || iResult == -3)
                QThread::msleep(100);
            else
                break;
        }

        QThread::msleep(1000);

        for( ; ;)
        {
            QThread::msleep(500);
            int nRequestIDs = CreateNewRequestID();
            CThostFtdcQryOrderField reqInfo = {0};
            int ret = m_pTradeApi->ReqQryOrder(&reqInfo, nRequestIDs);
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
