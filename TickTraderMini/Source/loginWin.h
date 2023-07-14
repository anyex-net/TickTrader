#ifndef loginWin_H
#define loginWin_H

#include <QWidget>
#include "ui_loginWin.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "interfaceCTP/ThostFtdcUserApiDataType.h"
#include "interfaceCTP/ThostFtdcUserApiStruct.h"
extern "C"{
#include "interfaceCTP/ThostFtdcTraderApi.h"
#include "interfaceCTP/ThostFtdcMdApi.h"
}
#include "tradespiimp.h"
#include "mdspiimp.h"
#include <QDebug>
#include "tools/log.h"

enum TRADE_SIGNAL{LOGIN,GOT_M1,GOT_M5,GOT_M15,GOT_D1,LOGOUT};


struct DBServerInfo {
    char name[32];
    char dll[32];
    char params[3000];
    int  length;
};

struct UserInfo
{
    char name[16];
    char pass[16];
};

struct ServerAddr
{
    QString BrokerID;
    QString tradeServ;
    QString marketServ;
    QString authCode;
    QString appID;
};

struct PrivateIns
{
    TThostFtdcInstrumentIDType InstrumentID[100];
    int clickmode;
    int notice;
    int fPoints;
    char versionUrl[64];
    char exePath[64];
};

enum enShowProc
{
    ShowProc_loginOk,
    ShowProc_instrument,
    ShowProc_order,
    ShowProc_trade,
    ShowProc_position,
    ShowProc_account
};

class loginWin : public QWidget
{
    Q_OBJECT

public:
    loginWin(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~loginWin();

    char userName[16];
    char password[16];
    char brokerID[16];
    char authCode[17];
    char appID[33];
    DBServerInfo server;
    UserInfo m_users;
    QList<ServerAddr> m_listAddr;
    int  serverAddrIndex = 0;
    QTimer * timeLimiter;
    void getUserInfos();
    void loginSucceed();
    void loginFailed(int code, QString mess);
    void logutSecced();

    CThostFtdcRspUserLoginField loginRes;

private:
    Ui::loginWinClass ui;
    int  m_iProcessNum = 0;

public slots:
    void login_clicked();
    void onShowProcess(int func, bool last);

signals:
    void pushTrade();
    void errShow(int code, QString mess);
    void loginSec();
    void pushTradeToFrom(TRADE_SIGNAL);

    void showProcess(int, bool);

protected:
    void keyPressEvent ( QKeyEvent * event );
};

#endif // loginWin_H
