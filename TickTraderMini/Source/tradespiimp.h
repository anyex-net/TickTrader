#ifndef TRADESPIIMP_H
#define TRADESPIIMP_H

//#include "loginWin.h"
#include "interfaceCTP/ThostFtdcUserApiDataType.h"
#include "interfaceCTP/ThostFtdcUserApiStruct.h"
extern "C"{
#include "interfaceCTP/ThostFtdcTraderApi.h"
#include "interfaceCTP/ThostFtdcMdApi.h"
}
#include <QMap>

class CTradeSpiImp : public CThostFtdcTraderSpi
{
public:
    CTradeSpiImp(CThostFtdcTraderApi* pTraderApi);
    ~CTradeSpiImp(void);

    CThostFtdcTraderApi* m_pTradeApi;

    bool loginFlags;

    QMap<QString, CThostFtdcInstrumentField *> tempCons;

    virtual void OnFrontConnected();

    virtual void OnFrontDisconnected();

    virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

    virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

//    virtual void OnMarketStatusNty(stBCESMarketStatusNty *pMarketStatusNty);

    virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

    virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 //   virtual void OnFundNty(CThostFtdcTradingAccountField *pFund);

 //   virtual void OnPosiNty(CThostFtdcInvestorPositionField *pPosi);
};

#endif // TRADESPIIMP_H
