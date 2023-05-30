#include <QtCore/qmath.h>
#include "InstrManage.h"
#include "coolsubmit.h"
#include <QMovie>
#include <QPainterPath>
#include "configWidget.h"
#include <QTime>
#include "notice.h"
#include "orderManage.h"
#include <math.h>
#include <QDebug>

// ������¼����
extern loginWin * loginW;
// �߳���
// extern QMutex mutex;
extern quint32 CreateNewRequestID();
extern quint32 CreateVirtualOrderId();

// ���ý���
extern configWidget * cview;
#define srcollW 16

#define cos_15 qCos(M_PI/8)
#define sin_15 qSin(M_PI/8)
#define cos_15f qCos(-M_PI/8)
#define sin_15f qSin(-M_PI/8)

enum CMOUSE_STATUS{
    NORMAL,             // ƽ��״̬
    DROPSELL,           // ɾ����
    DROPBUY,            // ɾ��
    CHANGESELL,         // ������
    CHANGEBUY,          // ����
    SROLLBAR            // ��ק������
};

enum TRADE_STATUS{
    LIMIT,              // �޼۵�
    FLIMIT,             // ֹ���޼۵�
    OCOMODE,            // OCO
    ZZMODE              // ׷��ֹ��ģʽ
};

QMap<double, StopParam> stopMap;

CMOUSE_STATUS mouseStatus;
TRADE_STATUS  tradeStatus;

CoolSubmit::CoolSubmit(QString Instr, CThostFtdcDepthMarketDataField * t, TradeWidget * tWidget, QWidget *parent, Qt::WindowFlags f)
    :QWidget(parent, f)
{
    linPix = 20;
    setWhatsThis("CoolSubmit");
    mouseStatus = NORMAL;
    tradeStatus = LIMIT;
    tw = tWidget;
    moveWPixs = 0;
    moveHPixs = 0;
    continueFlag = 0;
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    CThostFtdcInstrumentField * instrment = tw->insMap[Instr];
    qInfo() << Instr;
    selectInstr = new InstrManage(instrment, this);
    connect(selectInstr, SIGNAL(changed()), this, SLOT(initByInstr()));
    connect(tw, SIGNAL(floating()), this, SLOT(update()));
    sQuot = new CThostFtdcDepthMarketDataField;
    ::memset(sQuot,0,sizeof(CThostFtdcDepthMarketDataField));
    selectInstr->hide();
    initByInstr();
    numL = new QLineEdit(this);
    numL->setText("1");
    numL->hide();
    numL->setAlignment(Qt::AlignHCenter);
    price1 = 0;
    price2 = 0;
    price3 = 0;
    price4 = 0;
    //orderModeC = new QComboBox(this);
    //orderModeC->addItem(QString::fromLocal8Bit("�޼�"));
    //orderModeC->addItem(QString::fromLocal8Bit("�޼�ֹ��"));
    //orderModeC->addItem(QString::fromLocal8Bit("�м�ֹ��"));
    //orderModeC->addItem(QString::fromLocal8Bit("׷��ֹ��"));
    //orderModeC->setCurrentIndex(0);
    //connect(orderModeC,SIGNAL(currentIndexChanged(int)),this,SLOT(updateFundIndex()));
    //wheelTimer = new QTimer();
    //wheelTimer->setInterval(50);
    //connect(wheelTimer, SIGNAL(timeout()), this, SLOT(onTimerOut()));
    ractions = new QMenu(this);
    QAction *cancelAction = ractions->addAction(QString::fromLocal8Bit("����"));
    connect(cancelAction, SIGNAL(triggered(bool)), this, SLOT(rcancelAction()));
}

// �л���Լ
void CoolSubmit::initByInstr()
{
    pScale = 2;
    wheelSteps = 0;
    clkborderWidth = 1;
    currentAccout = loginW->userName;
    maxWheelSteps = 100;
    minWheelSteps = -100;
    continueFlag = 0;
    double dis = 0; // �Ƿ�
    double disper = 0; // �Ƿ��ٷֱ�
    minMove = 1;
    tradeStatus = LIMIT;
    this->setFocus(Qt::OtherFocusReason);
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
    if(!ci)
    {
        // ȱ����
        //QMessageBox::about(NULL,QString::fromLocal8Bit("��ʾ"),QString::fromLocal8Bit("ȱ�����飡"));
        //Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("ȱ�����飡"), false, QString::fromLocal8Bit("��ʾ"), NULL, 0);
        //nt.exec();
        return;
    }
    if(pQuotApi && tw->insMap_dy[QString::fromLocal8Bit(ci->InstrumentID)] == NULL)
    {
        // ����ǰ�ȶ���
        int count = 1;
        char **InstrumentID = new char*[count];
        InstrumentID[0] = ci->InstrumentID;
        pQuotApi->SubscribeMarketData(InstrumentID, 1);
        tw->insMap_dy[QString::fromLocal8Bit(ci->InstrumentID)] = ci;
        delete[] InstrumentID;
    }
    CThostFtdcDepthMarketDataField * cq = tw->quotMap[QString::fromLocal8Bit(ci->InstrumentID)];
    minMove = ci->PriceTick;
    if(cq == 0 || minMove <0.0001)
    {
        QTimer::singleShot(1000,this,SLOT(initByInstr()));
        return;
    }
    ::memcpy(sQuot,cq,sizeof(CThostFtdcDepthMarketDataField));
    if(sQuot->LastPrice < 0.0001)
    {
    }else if(sQuot->PreSettlementPrice > 0.0001) // ������
    {
        dis = sQuot->LastPrice-sQuot->PreSettlementPrice;
        disper = 100*dis/sQuot->PreSettlementPrice;
    }
    else if(sQuot->PreClosePrice > 0.0001) //�����̼�
    {
        dis = sQuot->LastPrice-sQuot->PreClosePrice;
        disper = 100*dis/sQuot->PreClosePrice;
    }
    QString title_ = QString::fromLocal8Bit(ci->InstrumentID).append(" (")
        .append(QString::fromLocal8Bit(ci->InstrumentName))
        .append(") ").append(QString::number(dis)).append("(")
        .append(QString::number(disper,'f',1)).append("%)");
    this->setWindowTitle(title_);
    if(minMove > 0.95)
        pScale = 0;
    if(minMove > 0.095 && minMove < 0.95)
        pScale = 1;
    if(minMove > 0.0095 && minMove < 0.095)
        pScale = 2;
    if(minMove > 0.00095 && minMove < 0.0095)
        pScale = 3;
    if(sQuot->HighestPrice > sQuot->LowestPrice)
    {
        maxWheelSteps = sQuot->HighestPrice/minMove;
        minWheelSteps = sQuot->LowestPrice/minMove;
    }
    if(sQuot->LastPrice > 0.00001)
    {
        wheelSteps = sQuot->LastPrice/minMove;
    }
    else if(sQuot->PreSettlementPrice > 0.00001) // ȱ�����������������ս����
    {
        wheelSteps = sQuot->PreSettlementPrice/minMove;
    }
    else if (sQuot->PreClosePrice > 0.00001) // ȱ���������������������̼�
    {
        wheelSteps = sQuot->PreClosePrice/minMove;
    }
}

// ��ʱ����
void CoolSubmit::onTimerOut()
{
    int y = QWidget::mapFromGlobal(cursor().pos()).y();
    if(y > submitBR.bottom())
    {
        wheeldown();
    }
    else if(y < submitBR.top())
    {
        wheelup();
    }
}

CoolSubmit::~CoolSubmit()
{
    delete sQuot;
    sQuot = NULL;
}

// �����������¼�
void CoolSubmit::wheelEvent(QWheelEvent * event)
{
    continueFlag = 0;
    if(event->delta() > 0)
        wheelup();
    else if(event->delta() < 0)
        wheeldown();
}


// ��������
void CoolSubmit::wheelup()
{
    if(wheelSteps >= maxWheelSteps)
        return;
    wheelSteps ++;
    update();
}

// ��������
void CoolSubmit::wheeldown()
{
    if(wheelSteps <= minWheelSteps)
        return;
    wheelSteps--;
    update();
}

// ��¼��갴��ʱ��״̬
void CoolSubmit::mousePressEvent(QMouseEvent * event)
{
    if(event->button() != Qt::LeftButton)
    {
        return;
    }
    messageInfo = "";
    continueFlag = 0;
    moveWPixs = event->x();
    moveHPixs = event->y();
    clkborderWidth = 3;
    update();
}


// ����ƶ�ʱ���¼�����
void CoolSubmit::mouseMoveEvent(QMouseEvent * event)
{
    if(event->buttons() != Qt::LeftButton)
    {
        if(submitBR.contains(event->pos()) || submitSR.contains(event->pos()))
        {
            setCursor(QCursor(Qt::PointingHandCursor));
        }
        else
        {
            setCursor(QCursor(Qt::ArrowCursor));
        }
        update();
        return;
    }
    setCursor(QCursor(Qt::ArrowCursor));
    if(submitBR.contains(moveWPixs, moveHPixs) && submitBR.contains(event->pos()))
    {
        setCursor(QCursor(Qt::PointingHandCursor));
        mouseStatus = CHANGEBUY;
    }
    if(submitSR.contains(moveWPixs, moveHPixs) && submitSR.contains(event->pos()))
    {
        setCursor(QCursor(Qt::PointingHandCursor));
        mouseStatus = CHANGESELL;
    }
    if(submitBR.contains(moveWPixs, moveHPixs) && !submitBR.contains(event->pos()))
    {
        mouseStatus = DROPBUY;
    }
    if(submitSR.contains(moveWPixs, moveHPixs) && !submitSR.contains(event->pos()))
    {
        mouseStatus = DROPSELL;
    }
    if(scrollR.contains(moveWPixs, moveHPixs))
    {
        mouseStatus = SROLLBAR;
    }
//    if(submitSR.contains(moveWPixs, moveHPixs) && AskPriceR.contains(event->pos()) && tradeStatus == LIMIT)
//    {
//        tradeStatus = FLIMIT;
//    }
//    if(submitBR.contains(moveWPixs, moveHPixs) && BidPriceR.contains(event->pos()) && tradeStatus == LIMIT)
//    {
//        tradeStatus = FLIMIT;
//    }
    if(sBarR.contains(moveWPixs, moveHPixs))
    {
        int yH = event->y();
        if(yH > linPix && yH < height() - 30)
            wheelSteps = maxWheelSteps-(yH-linPix)*abs(maxWheelSteps-minWheelSteps)/(height()-1*linPix);
    }
    continueFlag = 0;
    update();
}

// ��¼����ɿ�ʱ��״̬
void CoolSubmit::mouseReleaseEvent(QMouseEvent * event)
{
    clkborderWidth = 1;
    int x = QWidget::mapFromGlobal(cursor().pos()).x();
    int y = QWidget::mapFromGlobal(cursor().pos()).y();
    if(event->button() != Qt::LeftButton)
    {
        return;
    }
    if(cview && cview->pi.clickmode != 1)
        return;
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
    if(!ci)
        return;
    CThostFtdcDepthMarketDataField * cQuot = tw->quotMap[QString::fromLocal8Bit(ci->InstrumentID)];
    if(!cQuot)
        return;
    TradeInfo & tf = tradeInfoLst[currentAccout];
    QMap<QString,PosiPloy *>::const_iterator iter = tf.posiLst.find(QString::fromLocal8Bit(ci->InstrumentID));
    PosiPloy * pPloy = iter != tf.posiLst.end() ? iter.value():NULL;
    QList<CThostFtdcInvestorPositionField *> posi;
    if(pPloy)
        posi = pPloy->posi;
    int dis = moveHPixs/linPix-1;
    double price =(wheelSteps + linNum/2-dis)*minMove;
    int dis2 = event->y()/linPix-1;
    double price2 =(wheelSteps + linNum/2-dis2)*minMove;
    if(submitCountsR.contains(QPoint(moveWPixs,moveHPixs)) && event->x() == moveWPixs && event->y() == moveHPixs)
    {
        numL->show();
        numL->setFocus(Qt::OtherFocusReason);
    }
    else
    {
        numL->hide();
    }
    if(selectInstrR.contains(QPoint(moveWPixs,moveHPixs)) && event->x() == moveWPixs && event->y() == moveHPixs && !selectInstr->isVisible())
    {
        selectInstr->init(&tw->insMap);
    }
    else
    {
        selectInstr->hide();
    }
    if(qtyRect.contains(QPoint(moveWPixs,moveHPixs)) && posi.size() > 0)
    {
//		int qty = 0;
//		if(posi->SellQty == 0)
//		{
//			qty = posi->BuyQty;
//		}
//		else if(posi->BuyQty == 0)
//		{
//			qty = posi->SellQty;
//		}
//		else
//		{
//			qty = posi->BuyQty > posi->SellQty?posi->BuyQty - posi->SellQty:posi->SellQty - posi->BuyQty;
//		}
        if(posi.size() == 1)
            numL->setText(QString::number(posi[0]->Position));
        else
            numL->setText(QString::number(0));
    }
//    qInfo() << moveWPixs << moveHPixs << rcount2.topLeft().x() << rcount2.topLeft().y() << rcount2.bottomRight().x() << rcount2.bottomRight().y();
    if(rcount1.contains(QPoint(moveWPixs,moveHPixs)))
    {
        numL->setText("1");
    }
    if(rcount2.contains(QPoint(moveWPixs,moveHPixs)))
    {
        numL->setText("5");
    }
    if(rcount3.contains(QPoint(moveWPixs,moveHPixs)))
    {
        numL->setText("10");
    }
    if(rcount5.contains(QPoint(moveWPixs,moveHPixs)))
    {
        numL->setText("50");
    }
    if(rcount10.contains(QPoint(moveWPixs,moveHPixs)))
    {
        numL->setText("100");
    }
    if(rcount20.contains(QPoint(moveWPixs,moveHPixs)))
    {
        numL->setText("500");
    }
    if(cAllBuy.contains(QPoint(moveWPixs,moveHPixs))) // ��������
    {
        if(cview->pi.notice) {
            Notice info(Notice::NOTICE_TYPE_WARNING, QString::fromLocal8Bit("���������򵥣�"), cview->pi.notice, QString::fromLocal8Bit("����ȷ��"), NULL, 0);
            info.exec();
            if(info.pushButton) {
                dropAll(THOST_FTDC_D_Buy);
            }
        } else {
            dropAll(THOST_FTDC_D_Buy);
        }
    }
    if(cAllSell.contains(QPoint(moveWPixs,moveHPixs))) // ��������
    {
        if(cview->pi.notice) {
            Notice info(Notice::NOTICE_TYPE_WARNING, QString::fromLocal8Bit("��������������"), cview->pi.notice, QString::fromLocal8Bit("����ȷ��"), NULL, 0);
            info.exec();
            if(info.pushButton) {
                dropAll(THOST_FTDC_D_Sell);
            }
        } else {
            dropAll(THOST_FTDC_D_Sell);
        }
    }
    if(cAll.contains(QPoint(moveWPixs,moveHPixs))) // ������
    {
        if(cview->pi.notice) {
            Notice info(Notice::NOTICE_TYPE_WARNING, QString::fromLocal8Bit("�������ж�����"), cview->pi.notice, QString::fromLocal8Bit("����ȷ��"), NULL, 0);
            info.exec();
            if(info.pushButton) {
                dropAll(NULL);
            }
        } else {
            dropAll(NULL);
        }
    }
    if(scrollR.contains(QPoint(moveWPixs,moveHPixs)) && event->x() == moveWPixs && event->y() == moveHPixs)
    {
        int yH = event->y();
//		wheelSteps = maxWheelSteps-(yH-linPix)*abs(maxWheelSteps-minWheelSteps)/(height()-1*linPix);
    }
    setCursor(QCursor(Qt::ArrowCursor));
    qInfo() << " tradeStatus " << tradeStatus << "mouseStatus " << mouseStatus;
    switch(tradeStatus)
    {
    case LIMIT:
        switch(mouseStatus)
        {
        case NORMAL:
            if(submitSR.contains(QPoint(moveWPixs,moveHPixs)))
            {
                insertOrder(THOST_FTDC_D_Sell, price);
            }
            if(submitBR.contains(QPoint(moveWPixs,moveHPixs)))
            {
                insertOrder(THOST_FTDC_D_Buy, price);
            }
            break;
        case DROPSELL:
            {
                dropOrder(THOST_FTDC_D_Sell, price);
                mouseStatus = NORMAL;
            }
            break;
        case DROPBUY:
            {
                dropOrder(THOST_FTDC_D_Buy, price);
                mouseStatus = NORMAL;
            }
            break;
        case CHANGESELL:
            {
                if(price != price2)
                {
                    updateOrder(THOST_FTDC_D_Sell, price, price2);
                }
                mouseStatus = NORMAL;
            }
            break;
        case CHANGEBUY:
            {
                if(price != price2)
                {
                    updateOrder(THOST_FTDC_D_Buy, price, price2);
                }
                mouseStatus = NORMAL;
            }
            break;
        case SROLLBAR:
            mouseStatus = NORMAL;
            break;
        }
        break;
 /*   case FLIMIT:
        {
            char b2s = THOST_FTDC_D_Buy;
            double fpoints = price2>price?price2-price:0;
            if(submitSR.contains(QPoint(moveWPixs,moveHPixs)))
            {
                fpoints = price2<price?price-price2:0;
                b2s = THOST_FTDC_D_Sell;
            }
            if(cQuot->LastPrice <= price+0.0001)
            {
                methodFlag = THOST_FTDC_CC_LastPriceGreaterEqualStopPrice;
            }
            else if(cQuot->LastPrice >= price-0.0001)
            {
                methodFlag = THOST_FTDC_CC_LastPriceLesserEqualStopPrice;
            }
            fpoints = fpoints/minMove;
            insertLOrder(b2s, price, (int)fpoints);
            tradeStatus = LIMIT;
        }
        break;*/
    }
    moveWPixs = 0;
    moveHPixs = 0;
    update();
}

void CoolSubmit::contextMenuEvent(QContextMenuEvent* e)
{
    if(submitSR.contains(e->pos()) || submitBR.contains(e->pos()))
    {
        if(tradeStatus != LIMIT)
            return;
        curposx = QWidget::mapFromGlobal(cursor().pos()).x();
        curposy = QWidget::mapFromGlobal(cursor().pos()).y();
        int dis = e->pos().y()/linPix-1;
        double price =(wheelSteps + linNum/2-dis)*minMove;
        ractions->exec(cursor().pos());
    }
}

// ����˫���¼�
void CoolSubmit::mouseDoubleClickEvent(QMouseEvent * event)
{
    if(curPriceR.contains(event->pos()) || BidPriceR.contains(event->pos())
             || AskPriceR.contains(event->pos()))
    {
        qInfo() << "double click";
        continueFlag = 0;
        initByInstr();
    }
}

// �޸Ķ���
void CoolSubmit::updateOrder(char bos, double price, double price2)
{
    TradeInfo * ti = &(tradeInfoLst[currentAccout]);
    QMapIterator<QString, CThostFtdcOrderField *> i(ti->orderLst);
    while (i.hasNext()) {
        CThostFtdcOrderField * sOrder = ti->orderLst[i.next().key()];
        if(sOrder && sOrder->LimitPrice-price<0.000005 && sOrder->LimitPrice-price>-0.000005 && sOrder->Direction == bos)
        {
            if(sOrder->ContingentCondition == THOST_FTDC_CC_Immediately && sOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
            {
                CThostFtdcInstrumentField * ci = tw->insMap[sOrder->InstrumentID];
                if(!ci) continue;
                NEWORDERINF noi;
                ::memset(&noi,0,sizeof(NEWORDERINF));
                sprintf(noi.tif, "%s", currentAccout.toLatin1().data());
                strncpy(noi.OrderRef,sOrder->OrderRef,sizeof(noi.OrderRef)); /* ������ */
                strncpy(noi.InvestorID,sOrder->InvestorID,sizeof(noi.InvestorID)); /* Ͷ���ߺ� */
                strncpy(noi.InstrumentID,sOrder->InstrumentID,sizeof(noi.InstrumentID)); /* ��Լ�� */
                noi.Direction = sOrder->Direction; /* ������־ */
                noi.CombOffsetFlag[0] = sOrder->CombOffsetFlag[0]; /* ��ƽ��־ */
                noi.PriceType = sOrder->OrderPriceType;	/* �۸����� */
                noi.Price = price2;	/* �۸� */
                noi.ConditionMethod = sOrder->ContingentCondition;	/* �������� */
                noi.Qty = sOrder->VolumeTotal;	/* ʣ������ */
                strncpy(noi.ExchangeID, sOrder->ExchangeID,sizeof(noi.ExchangeID));
                tw->o2upLst.append(noi);
                int nRequestID = CreateNewRequestID();
                CThostFtdcInputOrderActionField pCancelReq;
                ::memset(&pCancelReq,0,sizeof(CThostFtdcInputOrderActionField));
                strncpy(pCancelReq.BrokerID, loginW->m_users.BrokerID, sizeof(pCancelReq.BrokerID));
                strncpy(pCancelReq.OrderRef,sOrder->OrderRef,sizeof(pCancelReq.OrderRef)); /* ������ */
                strncpy(pCancelReq.InvestorID,sOrder->InvestorID,sizeof(pCancelReq.InvestorID)); /* Ͷ���ߺ� */
                strncpy(pCancelReq.InstrumentID,sOrder->InstrumentID,sizeof(pCancelReq.InstrumentID)); /* ��Լ�� */
                strncpy(pCancelReq.ExchangeID,ci->ExchangeID,sizeof(pCancelReq.ExchangeID)); /* �������� */


                pCancelReq.RequestID = nRequestID;
                pCancelReq.FrontID = sOrder->FrontID;
                pCancelReq.SessionID = sOrder->SessionID;
                memcpy( pCancelReq.OrderSysID, sOrder->OrderSysID, sizeof( pCancelReq.OrderSysID ) );

                pCancelReq.ActionFlag = THOST_FTDC_AF_Delete;

                ti->api->ReqOrderAction(&pCancelReq, nRequestID); // ��������
            }
            if(QString(sOrder->OrderRef).startsWith("ZZZS"))
            {
                PosiPloy * pp = ti->posiLst[sOrder->InstrumentID];
                if(pp)
                {
                    if(sOrder->Direction == THOST_FTDC_D_Buy)
                        pp->points +=  price2 - sOrder->LimitPrice;
                    else
                        pp->points -=  price2 - sOrder->LimitPrice;
                }
            }
            if(sOrder->OrderStatus == THOST_FTDC_OST_NotTouched)  // δ���� ֹ��
            {
                sOrder->LimitPrice = price2;
            }
        }
    }
}

// ɾ������
void CoolSubmit::dropOrder(char bos, double price)
{
    qInfo() << "chedan" << bos << price;
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
    if(!ci) return;
    TradeInfo & ti = tradeInfoLst[currentAccout];
    QMapIterator<QString, CThostFtdcOrderField *> i(ti.orderLst);
    while (i.hasNext()) {
        CThostFtdcOrderField * sOrder = ti.orderLst[i.next().key()];
        qInfo() << sOrder->OrderStatus << sOrder->LimitPrice;
        if(sOrder && QString::fromLocal8Bit(sOrder->InstrumentID) == ci->InstrumentID && sOrder->LimitPrice-price<0.000005  && sOrder->LimitPrice-price>-0.000005 && sOrder->Direction == bos)
        {
            if(sOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing || sOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing) // ��������
            {
                qInfo() << "enter";

                int nRequestID = CreateNewRequestID();
                CThostFtdcInputOrderActionField pCancelReq;
                ::memset(&pCancelReq,0,sizeof(CThostFtdcInputOrderActionField));
                strncpy(pCancelReq.BrokerID, loginW->m_users.BrokerID, sizeof(pCancelReq.BrokerID));
                strncpy(pCancelReq.OrderRef,sOrder->OrderRef,sizeof(pCancelReq.OrderRef)); /* ������ */
                strncpy(pCancelReq.InvestorID,sOrder->InvestorID,sizeof(pCancelReq.InvestorID)); /* Ͷ���ߺ� */
                strncpy(pCancelReq.InstrumentID,sOrder->InstrumentID,sizeof(pCancelReq.InstrumentID)); /* ��Լ�� */
                strncpy(pCancelReq.ExchangeID,ci->ExchangeID,sizeof(pCancelReq.ExchangeID)); /* �������� */

                pCancelReq.RequestID = nRequestID;
                pCancelReq.FrontID = sOrder->FrontID;
                pCancelReq.SessionID = sOrder->SessionID;
                memcpy( pCancelReq.OrderSysID, sOrder->OrderSysID, sizeof( pCancelReq.OrderSysID ) );

                pCancelReq.ActionFlag = THOST_FTDC_AF_Delete;

                qInfo() << "action333: " << pCancelReq.OrderRef;

                ti.api->ReqOrderAction(&pCancelReq, nRequestID);
            }
            if(sOrder->OrderStatus == THOST_FTDC_OST_NotTouched) // ����ֹ��
            {
                TradeInfo & tf = tradeInfoLst[currentAccout];
                QMap<QString,PosiPloy *>::const_iterator iter = tf.posiLst.find(QString::fromLocal8Bit(ci->InstrumentID));
                PosiPloy * pploy = iter != tf.posiLst.end() ? iter.value():NULL;
                if(pploy && QString(sOrder->OrderRef).startsWith("ZZZS"))
                {
                    pploy->trailPriceu = 0;
                    pploy->points = 0;
                    pploy->trigFlag = false;
                }
                sOrder->OrderStatus = THOST_FTDC_OST_Canceled;
                tw->checkOcoStatus(tf, sOrder->OrderRef);
                if(tw->omWidget)
                    tw->omWidget->update();
            }
        }
    }
}


// ɾ�����򶩵�
void CoolSubmit::dropAll(char bos)
{
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
    if(!ci) return;
    TradeInfo & ti = tradeInfoLst[currentAccout];
    QMapIterator<QString, CThostFtdcOrderField *> i(ti.orderLst);
    while (i.hasNext()) {
        CThostFtdcOrderField * sOrder = ti.orderLst[i.next().key()];
        if(sOrder && QString::fromLocal8Bit(sOrder->InstrumentID) == ci->InstrumentID)
        {
            if(bos && sOrder->Direction != bos) continue;
            if(sOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing) // ��������
            {
                int nRequestID = CreateNewRequestID();
                CThostFtdcInputOrderActionField pCancelReq;
                ::memset(&pCancelReq,0,sizeof(CThostFtdcInputOrderActionField));
                strncpy(pCancelReq.BrokerID, loginW->m_users.BrokerID, sizeof(pCancelReq.BrokerID));
                strncpy(pCancelReq.OrderRef,sOrder->OrderRef,sizeof(pCancelReq.OrderRef)); /* ������ */
                strncpy(pCancelReq.InvestorID,sOrder->InvestorID,sizeof(pCancelReq.InvestorID)); /* Ͷ���ߺ� */
                strncpy(pCancelReq.InstrumentID,sOrder->InstrumentID,sizeof(pCancelReq.InstrumentID)); /* ��Լ�� */
                strncpy(pCancelReq.ExchangeID,ci->ExchangeID,sizeof(pCancelReq.ExchangeID)); /* �������� */

                pCancelReq.RequestID = nRequestID;
                pCancelReq.FrontID = sOrder->FrontID;
                pCancelReq.SessionID = sOrder->SessionID;
                memcpy( pCancelReq.OrderSysID, sOrder->OrderSysID, sizeof( pCancelReq.OrderSysID ) );

                pCancelReq.ActionFlag = THOST_FTDC_AF_Delete;

                ti.api->ReqOrderAction(&pCancelReq, nRequestID);
            }
            if(sOrder->OrderStatus == THOST_FTDC_OST_NotTouched) // ����ֹ��
            {
                sOrder->OrderStatus = THOST_FTDC_OST_Canceled;
                tw->checkOcoStatus(ti, sOrder->OrderRef);
            }
        }
    }
}


void CoolSubmit::keyPressEvent ( QKeyEvent * event )
{
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
    if(!ci) return;
    messageInfo = "";
    CThostFtdcDepthMarketDataField * cQuot = tw->quotMap[QString::fromLocal8Bit(ci->InstrumentID)];
    if(!cQuot) return;
    switch(event->key())
    {
    case Qt::Key_Space:
        {
            continueFlag = 0;
            initByInstr();
        }
        break;
    case Qt::Key_Escape:
        {
            continueFlag = 0;
            numL->setText("1");
        }
        break;
    //case Qt::Key_Slash:
    //	{
    //		continueFlag = 2;
    //		selectInstr->searchBox->setFocus(Qt::OtherFocusReason);
    //		selectInstr->show();
    //	}
    //	break;
    case Qt::Key_B:
        {
            continueFlag = 0;
            insertOrder(THOST_FTDC_D_Buy, cQuot->AskPrice1);
        }
        break;
    case Qt::Key_S:
        {
            continueFlag = 0;
            insertOrder(THOST_FTDC_D_Sell, cQuot->BidPrice1);
        }
        break;
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
        {
            if(continueFlag == 0)
            {
                QChar qc(event->key());
                numL->setText(qc);
                continueFlag = 1;
            }
            else if(continueFlag == 1)
            {
                QString text = numL->text();
                numL->setFocus(Qt::OtherFocusReason);
                numL->setText(text.append(event->key()));
            }
            else
            {
                selectInstr->init(&tw->insMap);
                QString text = selectInstr->searchBox->text();
                selectInstr->searchBox->setText(text.append(event->key()));
            }
        }
            break;
    }
    update();
}

void CoolSubmit::getOrderOffsetFlag(CThostFtdcInputOrderField *pInputOrder, TThostFtdcVolumeType &OpenQty, TThostFtdcVolumeType &CloseQty, TThostFtdcVolumeType &CloseTodayQty)
{
    OpenQty=0,CloseQty=0,CloseTodayQty=0;

    TThostFtdcVolumeType SellQty, SellQtyFrozen, TodaySellQty, TodaySellQtyFrozen;
    TThostFtdcVolumeType BuyQty, BuyQtyFrozen, TodayBuyQty, TodayBuyQtyFrozen;

    CThostFtdcInvestorPositionField sPosiBuy = {0};
    CThostFtdcInvestorPositionField sPosiSell = {0};

    TradeInfo & tf = tradeInfoLst[currentAccout];
    QMap<QString,PosiPloy *>::const_iterator iter = tf.posiLst.find(QString::fromLocal8Bit(pInputOrder->InstrumentID));
    PosiPloy * pploy = iter != tf.posiLst.end() ? iter.value():NULL;
    QList<CThostFtdcInvestorPositionField *> posi;
    if(pploy)
    {
        posi = pploy->posi;

        ::memset(&sPosiBuy, 0 ,sizeof(CThostFtdcInvestorPositionField));
        ::memset(&sPosiSell, 0 ,sizeof(CThostFtdcInvestorPositionField));
        for(int index = 0; index < posi.size(); index++)
        {
            if(posi[index]->PosiDirection == THOST_FTDC_PD_Long)
                ::memcpy(&sPosiBuy, posi[index] ,sizeof(CThostFtdcInvestorPositionField));
            else if(posi[index]->PosiDirection == THOST_FTDC_PD_Short)
                ::memcpy(&sPosiSell, posi[index] ,sizeof(CThostFtdcInvestorPositionField));
        }
    }

    // ���
    BuyQty = sPosiBuy.Position;
    BuyQtyFrozen = sPosiBuy.ShortFrozen;
    TodayBuyQty = sPosiBuy.Position - sPosiBuy.YdPosition;
    TodayBuyQtyFrozen = (sPosiBuy.ShortFrozen - TodayBuyQty) >= 0 ? (sPosiBuy.ShortFrozen - TodayBuyQty):0;

    // �ղ�
    SellQty = sPosiSell.Position;
    SellQtyFrozen = sPosiSell.LongFrozen;
    TodaySellQty = sPosiSell.Position - sPosiSell.YdPosition;
    TodaySellQtyFrozen = (sPosiSell.LongFrozen - TodaySellQty) >= 0 ? (sPosiSell.LongFrozen - TodaySellQty):0;

    qInfo() << "duo: " << BuyQty << BuyQtyFrozen << TodayBuyQty << TodayBuyQtyFrozen
            << "kong: " << SellQty << SellQtyFrozen << TodaySellQty << TodaySellQtyFrozen;

    if(pInputOrder->Direction==THOST_FTDC_D_Buy){
        // ��

        if(!strcmp(pInputOrder->ExchangeID, "SHFE") || !strcmp(pInputOrder->ExchangeID,"INE")){
            // ��ƽ��ָ��

            //ƽ���
            if(TodaySellQty>TodaySellQtyFrozen){
                // �н��
                if(pInputOrder->VolumeTotalOriginal>(TodaySellQty-TodaySellQtyFrozen)){
                    CloseTodayQty=TodaySellQty-TodaySellQtyFrozen;
                }else{
                    CloseTodayQty=pInputOrder->VolumeTotalOriginal;
                }
            }

            // ƽ���
            if(pInputOrder->VolumeTotalOriginal>CloseTodayQty){
                // �����
                if(SellQty-TodaySellQty>SellQtyFrozen-TodaySellQtyFrozen){
                    // �����
                    if(pInputOrder->VolumeTotalOriginal-CloseTodayQty>(SellQty-TodaySellQty)-(SellQtyFrozen-TodaySellQtyFrozen)){
                        CloseQty=(SellQty-TodaySellQty)-(SellQtyFrozen-TodaySellQtyFrozen);
                    }else{
                        CloseQty=pInputOrder->VolumeTotalOriginal-CloseTodayQty;
                    }
                }
            }
        }else{
            //��ƽ��ָ��
            if(SellQty>SellQtyFrozen){
                // �гֲ�
                if((pInputOrder->VolumeTotalOriginal)>(SellQty-SellQtyFrozen)){
                    CloseQty=SellQty-SellQtyFrozen;
                }else{
                    CloseQty=pInputOrder->VolumeTotalOriginal;
                }
            }
        }

        //ʣ����������ִ���
        if(pInputOrder->VolumeTotalOriginal>(CloseTodayQty+CloseQty))
            OpenQty=pInputOrder->VolumeTotalOriginal-(CloseTodayQty+CloseQty);
    }else{
        // ��
        if(!strcmp(pInputOrder->ExchangeID,"SHFE") || !strcmp(pInputOrder->ExchangeID,"INE")){
            // ��ƽ��ָ��

            //ƽ���
            if(TodayBuyQty>TodayBuyQtyFrozen){
                // �н��
                if(pInputOrder->VolumeTotalOriginal>(TodayBuyQty-TodayBuyQtyFrozen)){
                    CloseTodayQty=TodayBuyQty-TodayBuyQtyFrozen;
                }else{
                    CloseTodayQty=pInputOrder->VolumeTotalOriginal;
                }
            }

            // ƽ���
            if(pInputOrder->VolumeTotalOriginal>CloseTodayQty){
                // �����
                if(BuyQty-TodayBuyQty>BuyQtyFrozen-TodayBuyQtyFrozen){
                    // �����
                    if(pInputOrder->VolumeTotalOriginal-CloseTodayQty>(BuyQty-TodayBuyQty)-(BuyQtyFrozen-TodayBuyQtyFrozen)){
                        CloseQty=(BuyQty-TodayBuyQty)-(BuyQtyFrozen-TodayBuyQtyFrozen);
                    }else{
                        CloseQty=pInputOrder->VolumeTotalOriginal-CloseTodayQty;
                    }
                }
            }
        }else{
            //��ƽ��ָ��
            if(BuyQty>BuyQtyFrozen){
                // �гֲ�
                if((pInputOrder->VolumeTotalOriginal)>(BuyQty-BuyQtyFrozen)){
                    CloseQty=BuyQty-BuyQtyFrozen;
                }else{
                    CloseQty=pInputOrder->VolumeTotalOriginal;
                }
            }
        }

        //ʣ����������ִ���
        if(pInputOrder->VolumeTotalOriginal>(CloseTodayQty+CloseQty))
            OpenQty=pInputOrder->VolumeTotalOriginal-(CloseTodayQty+CloseQty);
    }

    qInfo() << OpenQty << CloseQty << CloseTodayQty;
}

// ���붩��
void CoolSubmit::insertOrder(char bos, double price, int fpoints, int qty)
{
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
    if(qty == 0)
        qty = numL->text().toInt();
    if(!ci) return;
    if(cview->pi.notice)
    {
        QString mess = "";
        if(bos == THOST_FTDC_D_Buy)
        {
            mess = QString::fromLocal8Bit("ȷ���� ").append(QString::number(price,'f',pScale)).append(QString::fromLocal8Bit(" �۸����� ")).append(QString::number(qty)).append(QString::fromLocal8Bit(" �� ")).append(ci->InstrumentID).append(QString::fromLocal8Bit(" ��"));
        }
        else
        {
            mess = QString::fromLocal8Bit("ȷ���� ").append(QString::number(price,'f',pScale)).append(QString::fromLocal8Bit(" �۸����� ")).append(QString::number(qty)).append(QString::fromLocal8Bit(" �� ")).append(ci->InstrumentID).append(QString::fromLocal8Bit(" ��"));
        }
        // �µ���ʾ
        Notice info(Notice::NOTICE_TYPE_STANDARD, mess, cview->pi.notice - 1, QString::fromLocal8Bit("����ȷ��"), NULL, 0);
        info.exec();
        if(!info.pushButton) {
            return;
        } else {
            cview->setNotice(2-info._noticeStatus);
        }
    }
    TradeInfo & ti = tradeInfoLst[currentAccout];
    CThostFtdcInputOrderField pInputOrder;
    ::memset(&pInputOrder,0,sizeof(CThostFtdcInputOrderField));
    strncpy(pInputOrder.BrokerID, loginW->m_users.BrokerID, sizeof(pInputOrder.BrokerID));
    strncpy(pInputOrder.InvestorID,ti.accountName.toLatin1().data(),sizeof(pInputOrder.InvestorID)); /* Ͷ���ߺ� */
    strncpy(pInputOrder.InstrumentID, ci->InstrumentID,sizeof(pInputOrder.InstrumentID));     /* ��Լ�� */
    strncpy(pInputOrder.ExchangeID, ci->ExchangeID,sizeof(pInputOrder.ExchangeID));   /* �������� */   
    pInputOrder.Direction = bos; /* ������־ */
    pInputOrder.LimitPrice = price;	/* �۸� */
    pInputOrder.VolumeTotalOriginal = qty;	/* ���� */
    pInputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Open; /* ��ƽ��־ �� */
    pInputOrder.CombHedgeFlag[0] = THOST_FTDC_BHF_Speculation;
    pInputOrder.TimeCondition = THOST_FTDC_TC_GFD;
    pInputOrder.VolumeCondition = THOST_FTDC_VC_AV;
    pInputOrder.MinVolume = 1;
    pInputOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    int oId = CreateVirtualOrderId();
    pInputOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice;   /* �޼� */
    pInputOrder.ContingentCondition = THOST_FTDC_CC_Immediately;	/* �޼۵�ģʽ */

    int OpenQty, CloseQty, CloseTodayQty;
    getOrderOffsetFlag(&pInputOrder, OpenQty, CloseQty, CloseTodayQty);
    if(CloseTodayQty > 0){
        int nRequestID = CreateNewRequestID();
        sprintf(pInputOrder.OrderRef, "%12i", nRequestID);
        pInputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
        pInputOrder.VolumeTotalOriginal = CloseTodayQty;
        ti.api->ReqOrderInsert(&pInputOrder, nRequestID);
    }

    if(CloseQty > 0){
        int nRequestID = CreateNewRequestID();
        sprintf(pInputOrder.OrderRef, "%12i", nRequestID);
        if(!strcmp(pInputOrder.ExchangeID, "SHFE") || !strcmp(pInputOrder.ExchangeID,"INE")){
            pInputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
            pInputOrder.VolumeTotalOriginal = CloseQty;
        }
        else{
            pInputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
            pInputOrder.VolumeTotalOriginal = CloseQty;
        }
        ti.api->ReqOrderInsert(&pInputOrder, nRequestID);
    }

    if(OpenQty > 0){
        int nRequestID = CreateNewRequestID();
        sprintf(pInputOrder.OrderRef, "%12i", nRequestID);
        pInputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
        pInputOrder.VolumeTotalOriginal = OpenQty;
        ti.api->ReqOrderInsert(&pInputOrder, nRequestID);
    }
//    if(ti.api->ReqOrderInsert(&pInputOrder, nRequestID) == 0);
}

// ����ֹ�𶩵�
void CoolSubmit::insertLOrder(char bos, double price, int fpoints, int qty)
{
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
    if(qty == 0)
        qty = numL->text().toInt();
    if(!ci) return;
    if(cview->pi.notice)
    {
        QString mess = "";
        if(bos == THOST_FTDC_D_Buy)
        {
            mess = QString::fromLocal8Bit("ȷ���� ").append(QString::number(price,'f',pScale)).append(QString::fromLocal8Bit(" �۸����� ")).append(QString::number(qty)).append(QString::fromLocal8Bit(" �� ")).append(ci->InstrumentID).append(QString::fromLocal8Bit(" ��"));
        }
        else
        {
            mess = QString::fromLocal8Bit("ȷ���� ").append(QString::number(price,'f',pScale)).append(QString::fromLocal8Bit(" �۸����� ")).append(QString::number(qty)).append(QString::fromLocal8Bit(" �� ")).append(ci->InstrumentID).append(QString::fromLocal8Bit(" ��"));
        }
        // �µ���ʾ
        Notice info(Notice::NOTICE_TYPE_STANDARD, mess, cview->pi.notice - 1, QString::fromLocal8Bit("����ȷ��"), NULL, 0);
        info.exec();
        if(!info.pushButton) {
            return;
        } else {
            cview->setNotice(2-info._noticeStatus);
        }
    }
    TradeInfo & ti = tradeInfoLst[currentAccout];
    int nRequestID = CreateNewRequestID();
    CThostFtdcInputOrderField pInputOrder;
    ::memset(&pInputOrder,0,sizeof(CThostFtdcInputOrderField));
    strncpy(pInputOrder.InvestorID,ti.accountName.toLatin1().data(),sizeof(pInputOrder.InvestorID)); /* Ͷ���ߺ� */
    strncpy(pInputOrder.InstrumentID, ci->InstrumentID,sizeof(pInputOrder.InstrumentID));     /* ��Լ�� */
    strncpy(pInputOrder.ExchangeID, ci->ExchangeID,sizeof(pInputOrder.ExchangeID));   /* �������� */
    pInputOrder.Direction = bos; /* ������־ */
    pInputOrder.LimitPrice = price;	/* �۸� */
    pInputOrder.VolumeTotalOriginal = qty;	/* ���� */
    int oId = CreateVirtualOrderId();
    sprintf(pInputOrder.OrderRef, "%12i", nRequestID);
    pInputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Open; /* ��ƽ��־ �� */
    pInputOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice;   /* �޼� */
    pInputOrder.CombHedgeFlag[0] = THOST_FTDC_BHF_Speculation;
    pInputOrder.TimeCondition = THOST_FTDC_TC_IOC;
    pInputOrder.MinVolume = 1;
    // �������Ȿ�ض���
    CThostFtdcOrderField pOrder;
    ::memset(&pOrder,0,sizeof(CThostFtdcOrderField));
    strncpy(pOrder.InvestorID, pInputOrder.InvestorID,sizeof(pOrder.InvestorID));
    strncpy(pOrder.OrderRef, pInputOrder.OrderRef,sizeof(pOrder.OrderRef));
    strncpy(pOrder.InstrumentID, pInputOrder.InstrumentID,sizeof(pOrder.InstrumentID));
    pOrder.Direction = pInputOrder.Direction; // ������־
    pOrder.CombOffsetFlag[0] = pInputOrder.CombOffsetFlag[0]; // ��ƽ��־
    pOrder.CombHedgeFlag[0] = THOST_FTDC_BHF_Speculation;
    pOrder.LimitPrice = pInputOrder.LimitPrice; // �۸�
    pOrder.VolumeTotalOriginal = pInputOrder.VolumeTotalOriginal; // ����
    pOrder.VolumeTotalOriginal = pInputOrder.VolumeTotalOriginal; // ʣ������
    pOrder.ContingentCondition = methodFlag; // ��������
    pOrder.OrderPriceType = pInputOrder.OrderPriceType; // �۸�����
//	pOrder.FilledPoints = fpoints;
    strncpy(pOrder.OrderRef,pInputOrder.OrderRef,sizeof(pOrder.OrderRef));
    strncpy(pOrder.ExchangeID, pInputOrder.ExchangeID,sizeof(pOrder.ExchangeID));// ��������
    pOrder.OrderStatus = THOST_FTDC_OST_NotTouched;  // δ����
    tw->orderEmit(ti.spi, &pOrder);
    if(bos == THOST_FTDC_D_Buy)
        messageInfo = QString::fromLocal8Bit("���λ��");
    else
        messageInfo = QString::fromLocal8Bit("����λ��");
    messageInfo.append(QString::number(price, 'f', pScale));
}

// ����
void CoolSubmit::rcancelAction()
{
    double price = (wheelSteps + linNum/2-curposy/linPix+1)*minMove;
    if(submitSR.contains(curposx,curposy))
    {
        dropOrder(THOST_FTDC_D_Sell, price);
    }
    else
    {
        dropOrder(THOST_FTDC_D_Buy, price);
    }
}

//
void CoolSubmit::paintEvent(QPaintEvent * event)
{
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
    if(!ci)
    {
        ci = new CThostFtdcInstrumentField;
        ::memset(ci, 0, sizeof(CThostFtdcInstrumentField));
    }
    CThostFtdcDepthMarketDataField * cQuot = tw->quotMap[QString::fromLocal8Bit(ci->InstrumentID)];
    if(!cQuot)
    {
        cQuot = new CThostFtdcDepthMarketDataField;
        ::memset(cQuot, 0, sizeof(CThostFtdcDepthMarketDataField));
    }
    else if(::strcmp(sQuot->InstrumentID,"") == 0)
    {
        initByInstr();
    }
    if(sQuot->LastPrice <0.001 && cQuot->LastPrice > 0.001)
    {
        initByInstr();
        ::memcpy(sQuot, cQuot, sizeof(CThostFtdcDepthMarketDataField));
    }
    TradeInfo & tf = tradeInfoLst[currentAccout];
    QMap<QString,PosiPloy *>::const_iterator iter = tf.posiLst.find(QString::fromLocal8Bit(ci->InstrumentID));
    PosiPloy * pploy = iter != tf.posiLst.end() ? iter.value():NULL;
    QList<CThostFtdcInvestorPositionField *> posi;
    if(pploy)
        posi = pploy->posi;
    CThostFtdcInvestorPositionField sPosiBuy;
    CThostFtdcInvestorPositionField sPosiSell;
    ::memset(&sPosiBuy, 0 ,sizeof(CThostFtdcInvestorPositionField));
    ::memset(&sPosiSell, 0 ,sizeof(CThostFtdcInvestorPositionField));
    for(int index = 0; index < posi.size(); index++)
    {
        if(posi[index]->PosiDirection == THOST_FTDC_PD_Long)
            ::memcpy(&sPosiBuy, posi[index] ,sizeof(CThostFtdcInvestorPositionField));
        else if(posi[index]->PosiDirection == THOST_FTDC_PD_Short)
            ::memcpy(&sPosiSell, posi[index] ,sizeof(CThostFtdcInvestorPositionField));
    }
    double dis = 0; // �Ƿ�
    double disper = 0; // �Ƿ��ٷֱ�
    if(cQuot->PreSettlementPrice > 0.0001) // ������
    {
        dis = cQuot->LastPrice-cQuot->PreSettlementPrice;
        disper = 100*dis/cQuot->PreSettlementPrice;
    }
    else if(cQuot->PreClosePrice > 0.0001) //�����̼�
    {
        dis = cQuot->LastPrice-cQuot->PreClosePrice;
        disper = 100*dis/cQuot->PreClosePrice;
    }
    //QString title_ = QString::fromLocal8Bit(ci->InstrumentID).append(" ").append(QString::number(dis)).append("(").append(QString::number(disper,'f',1)).append("%)");
    QString zht = QString::fromLocal8Bit("TickTrader[");
    zht.append(loginW->userName);
    zht.append("] ");
    //zht.append(title_);
    tw->setWindowTitle(zht);
    // ��ʼ��ͼ
    int x = QWidget::mapFromGlobal(cursor().pos()).x();
    int y = QWidget::mapFromGlobal(cursor().pos()).y();
    int pW = width();
    int pH = height();
    int marR = pW*25/100;
    // �µ���������
    linNum = pH/linPix-1;
    QPainter painter(this);
    // �������
    QFont font;
    font.setFamily(QString::fromLocal8Bit("΢���ź�"));
    font.setPointSize(9);
    qApp->setFont(font);		// font����ΪĬ������
    QFont def = painter.font(); // ����Ĭ������

    // �������ͼ����
    //painter.fillRect(QRect(0,0,width(),height()),QColor(0,0,0));
    painter.fillRect(QRect(0,0,width(),height()),QColor(173,173,173));
    // ��߿�
    painter.setPen(QColor(167, 167, 167));
    // ����
    QLinearGradient linearGradient(0, 0, 0,linPix);
    linearGradient.setColorAt(0,QColor(187, 187, 187));
    linearGradient.setColorAt(1,QColor(187, 187, 187));
    painter.setBrush(QBrush(linearGradient));
    painter.drawRect(QRect(0,0,pW-marR+15,linPix));
    painter.setBrush(QColor(225, 225, 225));
    //if(messageInfo != "")
    //	pH -= linPix;
    painter.setPen(QColor(68, 68, 68));
    painter.drawLine(0, linPix, pW-marR+15, linPix);
    int pixGrp[6] = {18,12,16,18,18,18};
    int qty  = 0;
    QString lnames[6] = {QString::fromLocal8Bit("��Ʒ"),QString::fromLocal8Bit("�ֲ�"),QString::fromLocal8Bit("����"),QString::fromLocal8Bit("ӯ��"),QString::fromLocal8Bit("�����ʽ�"),QString::fromLocal8Bit("�˻�")};
    if(!tf.fund)
    {
        tf.fund = new CThostFtdcTradingAccountField;
        ::memset(tf.fund,0,sizeof(CThostFtdcTradingAccountField));
    }
    QString vnames[6] = {QString::fromLocal8Bit(ci->InstrumentID).append(",").append(QString::fromLocal8Bit(ci->InstrumentName))
        ,"0","0","0",QString::number(tf.fund->Available,'f',pScale),currentAccout};

    double BuyPrice = sPosiBuy.Position == 0 ? 0.00:sPosiBuy.PositionCost/(sPosiBuy.Position*1.00f)/(ci->VolumeMultiple*1.00f);
    double SellPrice = sPosiSell.Position == 0 ? 0.00:sPosiSell.PositionCost/(sPosiSell.Position*1.00f)/(ci->VolumeMultiple*1.00f);
    if(sPosiSell.Position == 0)
    {
        qty = sPosiBuy.Position;
        vnames[1] = QString::number(qty);
        vnames[2] = QString::number(BuyPrice,'f',pScale);
    }
    else if(sPosiBuy.Position == 0)
    {
        qty = -(int)sPosiSell.Position;
        vnames[1] = QString::number(qty);
        vnames[2] = QString::number(SellPrice,'f',pScale);
    }
    else
    {
        qty = sPosiBuy.Position-sPosiSell.Position;
        QString cc = QString::number(qty).append("(").append(QString::number(sPosiBuy.Position))
                .append("/-").append(QString::number(sPosiSell.Position)).append(")");
        vnames[1] = cc;
        if(qty >0)
            vnames[2] = QString::number(BuyPrice,'f',pScale);
        else
            vnames[2] = QString::number(SellPrice,'f',pScale);
    }
    double yk = (cQuot->LastPrice-BuyPrice)*ci->VolumeMultiple*sPosiBuy.Position+(SellPrice-cQuot->LastPrice)*ci->VolumeMultiple*sPosiSell.Position;
    vnames[3] = QString::number(yk,'f',pScale);
    int lPix = 0;
    int rPix = 0;
    // �����µ���
    int marL = srcollW;
    int orderW = pW-marL-marR;
    painter.fillRect(QRect(marL,linPix,orderW,pH-linPix),QColor(240, 240, 240));
    //���Ƶ�һ��
    painter.setPen(QPen(QColor(203, 203, 203),1));
    painter.drawLine(marL, linPix, pW-marR, linPix);
    //int oPix[6] = {20,15,15,20,15,15};
    int oPix[5] = {18,20,24,20,18};
    //QString oName[6] = {QString::fromLocal8Bit("ӯ��"), QString::fromLocal8Bit("����"),QString::fromLocal8Bit("����"),QString::fromLocal8Bit("�۸�"),QString::fromLocal8Bit("����"),QString::fromLocal8Bit("����")};
    QString oName[5] = {QString::fromLocal8Bit("����"),QString::fromLocal8Bit("����"),QString::fromLocal8Bit("�۸�"),QString::fromLocal8Bit("����"),QString::fromLocal8Bit("����")};
    //QColor clrs[6] = {QColor(160,160,164), QColor(241,241,241),QColor(52,0,202),QColor(128,129,129),QColor(190,0,0),QColor(241,241,241)};
    QColor clrs[6] = {QColor(221,223,225),QColor(173,173,173),QColor(221,223,225),QColor(173,173,173),QColor(221,223,225)};
    int olPix = 0;
    int orPix = 0;
    int priceW = 0;
    int bidPriceW = 0;
    int askPriceW = 0;
    font.setFamily(QString::fromUtf8("΢���ź�"));
    font.setBold(true);
    painter.setFont(font);
    // ����ڿ�
    painter.fillRect(QRect(0,linPix,marL,pH-1*linPix-1),QColor(237,237,237));
    for(int i=0;i<5;i++)
    {
        if(i>0)
            olPix += orderW*oPix[i-1]/100;
        orPix += orderW*oPix[i]/100;
        if(i+1 == 5)
            orPix = orderW;
        painter.setPen(QPen(QColor(203, 203, 203), 1));
        painter.drawLine(marL+olPix, 0, marL+olPix, pH);	//����������
        painter.setPen(QPen(QColor(34, 34, 34), 1));
        painter.drawText(QRect(marL+olPix,0,orPix-olPix,linPix),Qt::AlignCenter, oName[i]);
        QRect colorR = QRect(marL+olPix+1,linPix+1,orPix-olPix-1,pH-1*linPix-1);
        painter.fillRect(colorR,clrs[i]);
        //if(i==0)
        //{
        //	ProfitR = colorR;
        //}
        if(i==0)
        {
            submitBR = colorR;
        }
        if(i==1)
        {
            BidPriceR = colorR;
            bidPriceW = colorR.x();
        }
        if(i==2)
        {
            curPriceR = colorR;
            priceW = colorR.x();
        }
        if(i==3)
        {
            AskPriceR = colorR;
            askPriceW = colorR.x();
        }
        if(i==4)
        {
            submitSR = colorR;
        }
    }
    painter.setFont(def);
    // ���ƹ�����
    scrollR = QRect(marL+orderW,linPix*1,srcollW,pH-1*linPix);
    painter.setPen(QColor(167, 167, 167));
    painter.fillRect(scrollR,QColor(134, 134, 134));
    painter.setPen(QColor(167, 167, 167));
    painter.drawLine(marL+orderW,/*linPix*1*/0,marL+orderW,pH);
    int sroH = linPix*1+(maxWheelSteps-wheelSteps)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    if(y>1+linPix*1 && y<pH-1 && mouseStatus == SROLLBAR && sroH+(y-moveHPixs) >= linPix*1 && sroH+(y-moveHPixs) <= pH)
    {
        sroH += (y-moveHPixs);
        wheelSteps -= (y-moveHPixs)*(maxWheelSteps-minWheelSteps)/(pH-1*linPix);
        moveHPixs = y;
    }
    // ������K��
    int dayOpen = (sQuot->OpenPrice)/minMove;
    int dayMax = (sQuot->HighestPrice)/minMove;
    int dayMin = (sQuot->LowestPrice)/minMove;
    int dayClose = (sQuot->LastPrice)/minMove;
    int openH = linPix*1+(maxWheelSteps-dayOpen)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    int closeH = linPix*1+(maxWheelSteps-dayClose)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    int maxH = linPix*1+(maxWheelSteps-dayMax)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    int minH = linPix*1+(maxWheelSteps-dayMin)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+1,linPix,srcollW-2,maxH),QColor(237,237,237));
    if(dayOpen  != 0)
    {
        if(dayOpen > dayClose)
        {
            painter.setPen(QColor(255, 255, 255));
            //painter.drawLine(marL+orderW+(srcollW)/2-1,maxH,marL+orderW+(srcollW)/2-1,minH);
            painter.fillRect(QRect(marL+orderW,maxH,srcollW,minH-maxH),QColor(175,175,175));
            //painter.fillRect(QRect(marL+orderW+(srcollW-4)/2-1,openH,5,closeH-openH),QColor(240, 240, 240));
            //painter.setPen(QPen(QColor(52, 116, 24),1));
            //painter.drawRect(QRect(marL+orderW+(srcollW-4)/2-1,openH,4,closeH-openH));
        }
        else
        {
            painter.setPen(QColor(255, 255, 255));
            //painter.drawLine(marL+orderW+(srcollW)/2-1,maxH,marL+orderW+(srcollW)/2-1,minH);
            painter.fillRect(QRect(marL+orderW,maxH,srcollW,minH-maxH),QColor(175,175,175));
            //painter.fillRect(QRect(marL+orderW+(srcollW-4)/2-1,closeH,5,openH-closeH),QColor(240, 240, 240));
            //painter.setPen(QPen(QColor(210, 54, 54),1));
            //painter.drawRect(QRect(marL+orderW+(srcollW-4)/2-1,closeH,4,openH-closeH));
        }
    }
    painter.setPen(QColor(134, 134, 134));
    painter.fillRect(QRect(marL+orderW+1,minH,srcollW-2,height()-minH),QColor(237,237,237));
    // �����
    int curBuy = (sQuot->BidPrice1-0.00001)/minMove;
    int buyH = linPix*1+(maxWheelSteps-curBuy)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+1,buyH,6,2),QColor(178, 40, 40));
    // ������
    int curSell = (sQuot->AskPrice1+0.00001)/minMove;
    int sellH = linPix*1+(maxWheelSteps-curSell)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+11,sellH,6,2),QColor(10, 120, 171));
    // �����2
    int curBuy2 = (sQuot->BidPrice2-0.00001)/minMove;
    int buyH2 = linPix*1+(maxWheelSteps-curBuy2)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+1,buyH2,6,2),QColor(178, 40, 40));
    // ������2
    int curSell2 = (sQuot->AskPrice2+0.00001)/minMove;
    int sellH2 = linPix*1+(maxWheelSteps-curSell2)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+11,sellH2,6,2),QColor(10, 120, 171));
    // �����3
    int curBuy3 = (sQuot->BidPrice3-0.00001)/minMove;
    int buyH3 = linPix*1+(maxWheelSteps-curBuy3)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+1,buyH3,6,2),QColor(178, 40, 40));
    // ������3
    int curSell3 = (sQuot->AskPrice3+0.00001)/minMove;
    int sellH3 = linPix*1+(maxWheelSteps-curSell3)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+11,sellH3,6,2),QColor(10, 120, 171));
    // �����4
    int curBuy4 = (sQuot->BidPrice4-0.00001)/minMove;
    int buyH4 = linPix*1+(maxWheelSteps-curBuy4)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+1,buyH4,6,2),QColor(178, 40, 40));
    // ������4
    int curSell4= (sQuot->AskPrice4+0.00001)/minMove;
    int sellH4 = linPix*1+(maxWheelSteps-curSell4)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+11,sellH4,6,2),QColor(10, 120, 171));
    // �����5
    int curBuy5 = (sQuot->BidPrice5-0.00001)/minMove;
    int buyH5 = linPix*1+(maxWheelSteps-curBuy5)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+1,buyH5,6,2),QColor(178, 40, 40));
    // ������5
    int curSell5= (sQuot->AskPrice5+0.00001)/minMove;
    int sellH5 = linPix*1+(maxWheelSteps-curSell5)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
    painter.fillRect(QRect(marL+orderW+11,sellH5,6,2),QColor(10, 120, 171));
    if(qty > 0)// ��ֲ�
    {
        int qtyDix = (sPosiBuy.OpenAmount/(sPosiBuy.OpenVolume*1.00f))/minMove;
        int qtyH = linPix*1+(maxWheelSteps-qtyDix)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
        painter.fillRect(QRect(marL+orderW,qtyH,srcollW,2),QColor(210, 54, 54));
    }
    if(qty < 0)// ���ֲ�
    {
        int qtyDix = (sPosiSell.OpenAmount/(sPosiSell.OpenVolume*1.00f))/minMove;
        int qtyH = linPix*1+(maxWheelSteps-qtyDix)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
        painter.fillRect(QRect(marL+orderW,qtyH,srcollW,2),QColor(13, 150, 214));
    }
    painter.setPen(QColor(34, 34, 34));
    int linH = linPix*2;
    int buyPosiW = marL;
    int maxWeel = (maxWheelSteps-wheelSteps)*(height()-linPix)/(maxWheelSteps-minWheelSteps)+linPix;
    sBarR = QRect(marL+orderW+2,maxWeel,srcollW - 4,30);
    linearGradient.setStart(marL+orderW, maxWeel);						// 1
    linearGradient.setFinalStop(marL+orderW+srcollW, maxWeel);
    linearGradient.setColorAt(0,QColor(215,215,215));
    linearGradient.setColorAt(1,QColor(180,180,180));
    // ���ƹ�����
    painter.fillRect(sBarR,QBrush(linearGradient));
    double tPrice = (wheelSteps+linNum/2)*minMove;
    while(linH < height())
    {
        painter.setPen(QPen(QColor(143,143,143), 1));			//���������
        painter.drawLine(0, linH, pW-marR, linH);
        if(qty > 0)
        {
            if(tPrice - (sPosiBuy.OpenAmount/(sPosiBuy.OpenVolume*1.00f)) <ci->PriceTick-0.000005 && tPrice - (sPosiBuy.OpenAmount/(sPosiBuy.OpenVolume*1.00f)) > -0.000005)
            {
                painter.fillRect(QRect(priceW,linH-linPix,orderW*oPix[2]/100-1,linPix), QColor(13,150,214));
            }
        }
        else
        {
            if(tPrice - (sPosiSell.OpenAmount/(sPosiSell.OpenVolume*1.00f)) <ci->PriceTick-0.000005 && tPrice - (sPosiSell.OpenAmount/(sPosiSell.OpenVolume*1.00f)) > -0.000005)
            {
                painter.fillRect(QRect(priceW,linH-linPix,orderW*oPix[2]/100-1,linPix), QColor(210,54,54));
            }
        }
        if(tPrice > 0 && tPrice - cQuot->LastPrice <0.000005 && tPrice - cQuot->LastPrice > -0.000005)
        {
            painter.setPen(QPen(QColor(121, 121, 121), 1));
            QLinearGradient linearGradient(priceW,linH-linPix,priceW,linH);
            linearGradient.setColorAt(0,QColor(238, 226, 25));
            linearGradient.setColorAt(1,QColor(238, 226, 25));
            painter.fillRect(QRect(priceW,linH-linPix,orderW*oPix[2]/100-1,linPix), QBrush(linearGradient));
        }
        if(tPrice > 0 && tPrice - cQuot->BidPrice2 <0.000005 && tPrice - cQuot->BidPrice2 > -0.000005)
        {
            QLinearGradient linearGradient(bidPriceW,linH-linPix,bidPriceW,linH);
            linearGradient.setColorAt(0,QColor(187,52,54));
            linearGradient.setColorAt(1,QColor(187,52,54));
            painter.fillRect(QRect(bidPriceW,linH-linPix+1,orderW*oPix[1]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(bidPriceW,linH-linPix,orderW*oPix[1]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->BidVolume2));
        }
        if(tPrice > 0 && tPrice - cQuot->AskPrice2 <0.000005 && tPrice - cQuot->AskPrice2 > -0.000005)
        {
            QLinearGradient linearGradient(askPriceW,linH-linPix,askPriceW,linH);
            linearGradient.setColorAt(0,QColor(72,129,204));
            linearGradient.setColorAt(1,QColor(72,129,204));
            painter.fillRect(QRect(askPriceW,linH-linPix+1,orderW*oPix[3]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(askPriceW,linH-linPix,orderW*oPix[3]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->AskVolume2));
        }
        if(tPrice > 0 && tPrice - cQuot->BidPrice3 <0.000005 && tPrice - cQuot->BidPrice3 > -0.000005)
        {
            QLinearGradient linearGradient(bidPriceW,linH-linPix,bidPriceW,linH);
            linearGradient.setColorAt(0,QColor(187,52,54));
            linearGradient.setColorAt(1,QColor(187,52,54));
            painter.fillRect(QRect(bidPriceW,linH-linPix+1,orderW*oPix[1]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(bidPriceW,linH-linPix,orderW*oPix[1]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->BidVolume3));
        }
        if(tPrice > 0 && tPrice - cQuot->AskPrice3 <0.000005 && tPrice - cQuot->AskPrice3 > -0.000005)
        {
            QLinearGradient linearGradient(askPriceW,linH-linPix,askPriceW,linH);
            linearGradient.setColorAt(0,QColor(72,129,204));
            linearGradient.setColorAt(1,QColor(72,129,204));
            painter.fillRect(QRect(askPriceW,linH-linPix+1,orderW*oPix[3]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(askPriceW,linH-linPix,orderW*oPix[3]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->AskVolume3));
        }
        if(tPrice > 0 && tPrice - cQuot->BidPrice4 <0.000005 && tPrice - cQuot->BidPrice4 > -0.000005)
        {
            QLinearGradient linearGradient(bidPriceW,linH-linPix,bidPriceW,linH);
            linearGradient.setColorAt(0,QColor(187,52,54));
            linearGradient.setColorAt(1,QColor(187,52,54));
            painter.fillRect(QRect(bidPriceW,linH-linPix+1,orderW*oPix[1]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(bidPriceW,linH-linPix,orderW*oPix[1]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->BidVolume4));
        }
        if(tPrice > 0 && tPrice - cQuot->AskPrice4 <0.000005 && tPrice - cQuot->AskPrice4 > -0.000005)
        {
            QLinearGradient linearGradient(askPriceW,linH-linPix,askPriceW,linH);
            linearGradient.setColorAt(0,QColor(72,129,204));
            linearGradient.setColorAt(1,QColor(72,129,204));
            painter.fillRect(QRect(askPriceW,linH-linPix+1,orderW*oPix[3]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(askPriceW,linH-linPix,orderW*oPix[3]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->AskVolume4));
        }
        if(tPrice > 0 && tPrice - cQuot->BidPrice5 <0.000005 && tPrice - cQuot->BidPrice5 > -0.000005)
        {
            QLinearGradient linearGradient(bidPriceW,linH-linPix,bidPriceW,linH);
            linearGradient.setColorAt(0,QColor(187,52,54));
            linearGradient.setColorAt(1,QColor(187,52,54));
            painter.fillRect(QRect(bidPriceW,linH-linPix+1,orderW*oPix[1]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(bidPriceW,linH-linPix,orderW*oPix[1]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->BidVolume5));
        }
        if(tPrice > 0 && tPrice - cQuot->AskPrice5 <0.000005 && tPrice - cQuot->AskPrice5 > -0.000005)
        {
            QLinearGradient linearGradient(askPriceW,linH-linPix,askPriceW,linH);
            linearGradient.setColorAt(0,QColor(72,129,204));
            linearGradient.setColorAt(1,QColor(72,129,204));
            painter.fillRect(QRect(askPriceW,linH-linPix+1,orderW*oPix[3]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(askPriceW,linH-linPix,orderW*oPix[3]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->AskVolume5));
        }
        if(tPrice > 0 && tPrice - cQuot->BidPrice1 <0.000005 && tPrice - cQuot->BidPrice1 > -0.000005)
        {
            QLinearGradient linearGradient(bidPriceW,linH-linPix,bidPriceW,linH);
            linearGradient.setColorAt(0,QColor(187,52,54));
            linearGradient.setColorAt(1,QColor(187,52,54));
            painter.fillRect(QRect(bidPriceW,linH-linPix+1,orderW*oPix[1]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(bidPriceW,linH-linPix,orderW*oPix[1]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->BidVolume1));
        }
        if(tPrice > 0 && tPrice - cQuot->AskPrice1 <0.000005 && tPrice - cQuot->AskPrice1 > -0.000005)
        {
            QLinearGradient linearGradient(askPriceW,linH-linPix,askPriceW,linH);
            linearGradient.setColorAt(0,QColor(72,129,204));
            linearGradient.setColorAt(1,QColor(72,129,204));
            painter.fillRect(QRect(askPriceW,linH-linPix+1,orderW*oPix[3]/100-1,linPix-1), QBrush(linearGradient));
            painter.setPen(QPen(QColor(240, 240, 240), 1));
            painter.drawText(QRect(askPriceW,linH-linPix,orderW*oPix[3]/100-1,linPix),Qt::AlignCenter, QString::number(cQuot->AskVolume1));
        }
        painter.setPen(QPen(QColor(34, 34, 34), 1));
        //yk = (tPrice-sPosi.BuyPrice)*cInstr->TradeUnit*sPosi.BuyQty+(sPosi.SellPrice-tPrice)*cInstr->TradeUnit*sPosi.SellQty;
        if(tPrice > 0.001)
        {
            painter.drawText(QRect(priceW,linH-linPix,orderW*oPix[2]/100,linPix),Qt::AlignCenter, QString::number(tPrice,'f',pScale));
        }
        if(tPrice == price1 && price1 >0.0001)
        {
            int fpoints = (price2-price1)/minMove;
            int px = 0;
            int px0 = 0;
            if( bos1 == THOST_FTDC_D_Buy)
            {
                px = bidPriceW + orderW*oPix[1]/200 - 10;
                px0 = submitBR.x() + orderW*oPix[0]/200 + 10;
            }
            else
            {
                px = askPriceW + orderW*oPix[3]/200 + 10;
                px0 = submitSR.x() + orderW*oPix[4]/200 - 10;
            }
            int py = linH-linPix/2 - fpoints*linPix;
            int py0 = linH-linPix/2;
            double srt = qSqrt((px-px0)*(px-px0)+(py-py0)*(py-py0));
            if(srt < 1) srt = 1;
            int px1 = px-10*((px-px0)*cos_15-(py-py0)*sin_15)/srt;
            int py1 = py-10*((px-px0)*sin_15+(py-py0)*cos_15)/srt;
            int px2 = px-10*((px-px0)*cos_15f-(py-py0)*sin_15f)/srt;
            int py2 = py-10*((px-px0)*sin_15f+(py-py0)*cos_15f)/srt;
            painter.drawLine(px0,py0,px,py);
            QPainterPath jt;
            jt.moveTo(px,py);
            jt.lineTo(px1,py1);
            jt.lineTo(px2,py2);
            jt.lineTo(px,py);
            painter.fillPath(jt,QColor(0, 0, 0));
        }
        linH += linPix;
        tPrice -=minMove;
    }
    if(submitBR.contains(QPoint(x,y)))
    {
        int dis2 = y/linPix-1;
        if(dis2 > 0)
        {
            painter.setPen(QPen(QColor(34, 34, 34), clkborderWidth));
            painter.drawRect(submitBR.x(),(dis2+1)*linPix,submitBR.width(),linPix);
        }
    }
    if(submitSR.contains(QPoint(x,y)))
    {
        int dis2 = y/linPix-1;
        if(dis2 > 0)
        {
            painter.setPen(QPen(QColor(34, 34, 34), clkborderWidth));
            painter.drawRect(submitSR.x(),(dis2+1)*linPix,submitSR.width(),linPix);
        }
    }
    painter.setPen(QPen(QColor(34, 34, 34), 1));
    buyOrder.clear();
    sellOrder.clear();
    // ���ƶ���
    QMapIterator<QString, CThostFtdcOrderField *> ot(tf.orderLst);
    bool checkTrall = false;
    while(ot.hasNext())
    {
        CThostFtdcOrderField * iti = tf.orderLst[ot.next().key()];
        if(!iti)
            continue;
        if(iti && QString::fromLocal8Bit(iti->InstrumentID) == selectInstr->curInstr->InstrumentID)
        {
            if(iti->OrderStatus == THOST_FTDC_OST_NoTradeQueueing || iti->OrderStatus == THOST_FTDC_OST_NotTouched)
            {
                // �ҵ�����
                int itiP = iti->LimitPrice/minMove;
                int itiH = linPix*1+(maxWheelSteps-itiP)*(pH-1*linPix)/abs(maxWheelSteps-minWheelSteps);
                QPainterPath itiPath;
                if(iti->Direction == THOST_FTDC_D_Buy)
                    itiPath.addRect(marL+orderW+1,itiH,4,2);
                else
                    itiPath.addRect(marL+orderW+11,itiH,4,2);

                painter.fillPath(itiPath, QColor(87, 167, 56));

                double steps = ( - iti->LimitPrice)/minMove;
                int oLin  = 0;
                double dis = steps-(int)steps;
                if(dis > 0)
                {
                    if(dis < 0.5)
                    {
                        oLin = (int)steps + 1 +(wheelSteps+linNum/2);
                    }
                    else
                    {
                        oLin = (int)steps + 1 + 1 +(wheelSteps+linNum/2);
                    }
                }
                else
                {
                    if(dis > -0.5)
                    {
                        oLin = (int)steps + 1 +(wheelSteps+linNum/2);
                    }
                    else
                    {
                        oLin = (int)steps -1 + 1 +(wheelSteps+linNum/2);
                    }
                }
                if(oLin < 1)
                    continue;
                int qty = iti->VolumeTotalOriginal;
                if(iti->Direction == THOST_FTDC_D_Sell)
                {
                    int sss = sellOrder[oLin];
                    sellOrder[oLin] += qty;
                    painter.fillRect(QRect(submitSR.x(),oLin*linPix,submitSR.width(),linPix), QColor(225, 225, 255));
                    painter.setPen(QPen(QColor(68, 68, 68), 2));
                    painter.drawText(QRect(submitSR.x(),oLin*linPix+1,submitSR.width(),linPix-1),Qt::AlignCenter, QString::number(sellOrder[oLin]));
                }
                if(iti->Direction == THOST_FTDC_D_Buy)
                {
                    buyOrder[oLin] += qty;
                    painter.fillRect(QRect(submitBR.x(),oLin*linPix,submitBR.width(),linPix), QColor(225, 225, 255));
                    painter.setPen(QPen(QColor(68, 68, 68), 2));
                    painter.drawText(QRect(submitBR.x(),oLin*linPix+1,submitBR.width(),linPix-1),Qt::AlignCenter, QString::number(buyOrder[oLin]));
                }
                if(iti->ContingentCondition != THOST_FTDC_CC_Immediately)
                {
                    int px = 0;
                    int px0 = 0;
                    int py = 0;
                    if( iti->Direction == THOST_FTDC_D_Buy)
                    {
                        px = bidPriceW + orderW*oPix[1]/200 - 10;
                        px0 = submitBR.x() + orderW*oPix[0]/200 + 10;
            //			py = oLin*linPix+1+linPix/2 - iti->FilledPoints*linPix;
                    }
                    else
                    {
                        px = askPriceW + orderW*oPix[3]/200 + 10;
                        px0 = submitSR.x() + orderW*oPix[4]/200 - 10;
            //			py = oLin*linPix+1+linPix/2 + iti->FilledPoints*linPix;
                    }
                    int py0 = oLin*linPix+1+linPix/2;
                    double srt = qSqrt((px-px0)*(px-px0)+(py-py0)*(py-py0));
                    if(srt < 1) srt = 1;
                    int px1 = px-10*((px-px0)*cos_15-(py-py0)*sin_15)/srt;
                    int py1 = py-10*((px-px0)*sin_15+(py-py0)*cos_15)/srt;
                    int px2 = px-10*((px-px0)*cos_15f-(py-py0)*sin_15f)/srt;
                    int py2 = py-10*((px-px0)*sin_15f+(py-py0)*cos_15f)/srt;
                    painter.drawLine(px0,py0,px,py);
                    QPainterPath jt;
                    jt.moveTo(px,py);
                    jt.lineTo(px1,py1);
                    jt.lineTo(px2,py2);
                    jt.lineTo(px,py);
                    painter.fillPath(jt,QColor(0, 0, 0));
                }
            }
        }
    }
    //mutex.unlock();
    // ���ƶ�����Ŀ����
    int oNPix = linH-linPix*((linNum+9)/2+2);
    oNPix = oNPix<linPix?linPix:oNPix;
    int x6 = pW-marR+srcollW+1+1;
    int w6 = marR-srcollW-2-2;
    /*painter.fillRect(QRect(x6,oNPix,w6,linPix*10+5),QColor(145, 145, 145));
    painter.setBrush(QColor(145, 145, 145));*/
    // ��Ʒ����
    selectInstrR = QRect(x6, oNPix/2+1, w6, linPix-2);
    linearGradient.setStart(x6, oNPix/2);
    linearGradient.setFinalStop(x6, oNPix/2+linPix);
    if(selectInstrR.contains(x,y)) {
        painter.setPen(QColor(0, 0, 0));
        linearGradient.setColorAt(0,QColor(255,255,255));
        linearGradient.setColorAt(1,QColor(255,255,255));
    } else {
        painter.setPen(QColor(10, 10, 10));
        linearGradient.setColorAt(0,QColor(255,255,255));
        linearGradient.setColorAt(1,QColor(255,255,255));
    }
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(selectInstrR, 2, 2);
    painter.drawText(selectInstrR,Qt::AlignCenter, QString::fromLocal8Bit(ci->InstrumentID));
    //painter.fillRect(selectInstrR, QColor(145, 145, 145));
    //painter.setPen(QPen(QColor(34, 34, 34), 1));
    //painter.drawLine(x6, oNPix+linPix*1, pW-7, oNPix+linPix*1);
    // painter.drawText(selectInstrR,Qt::AlignCenter, lnames[0].append(":").append(vnames[0]));
    selectInstr->setGeometry(QRect(x6, oNPix/2+1, w6, 9999));

    // �Ƿ�����
    painter.setPen(QColor(119, 119, 119));
    disperRect = QRect(x6, oNPix/2+linPix+5, w6, linPix-2);
    linearGradient.setStart(x6, oNPix/2+linPix+5);
    linearGradient.setFinalStop(x6, oNPix/2+2*linPix+5);
    linearGradient.setColorAt(0,QColor(255, 255, 255));
    linearGradient.setColorAt(1,QColor(230, 230, 230));
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(disperRect, 2, 2);
    QColor disperColor = QColor(34, 34, 34);
    //QString disText = QString::number(dis).append("(").append(QString::number(disper,'f',1)).append("%)");
    QString disText = QString::number(disper,'f',1).append("%");
    if ( disper > 0.0 ) {
        disperColor = QColor(210, 54, 54);
        //disText = QString::fromLocal8Bit("+").append(QString::number(dis)).append("(+").append(QString::number(disper,'f',1)).append("%)");
        disText = QString::fromLocal8Bit("+").append(QString::number(disper,'f',1).append("%"));
    } else if ( disper < 0.0 ) {
        disperColor = QColor(87, 167, 56);
    } else {
        disperColor = QColor(34, 34, 34);
    }
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(disperColor);
    painter.drawText(disperRect,Qt::AlignCenter, disText);

    // �ֲ�����
    painter.setPen(QColor(119, 119, 119));
    qtyRect = QRect(x6,oNPix/2+3*linPix+1,w6,linPix-2);
    QColor qtyColor = QColor(145, 145, 145);
    linearGradient.setStart(x6,oNPix/2+3*linPix+1);
    linearGradient.setFinalStop(x6,oNPix/2+4*linPix+1);
    if ( qty > 0 ) {
        qtyColor = QColor(187,52,54);
        painter.setPen(qtyColor);
    } else if ( qty < 0) {
        qtyColor = QColor(72,129,204);
        painter.setPen(qtyColor);
    } else {
        qtyColor = QColor(145,145,145);
    }
    linearGradient.setColorAt(0,qtyColor);
    linearGradient.setColorAt(1,qtyColor);
    painter.setBrush(QBrush(linearGradient));
    painter.drawRect(qtyRect);
    painter.setPen(QColor(240, 240, 240));
    painter.drawText(qtyRect,Qt::AlignCenter, vnames[1]);
    //painter.setPen(QColor(34, 34, 34));
    //painter.drawLine(x6, oNPix+linPix*2, pW-7, oNPix+linPix*2);
    //// �����ʽ�
    //painter.setPen(QPen(QColor(0,0,0), 1));
    //painter.drawLine(x6, oNPix+linPix*3, pW, oNPix+linPix*3);
    //painter.drawText(QRect(x6,oNPix+linPix*2,w6,linPix),Qt::AlignCenter, vnames[4]);
    //painter.drawLine(x6, oNPix+linPix*3, pW-7, oNPix+linPix*3);

    // ӯ����ʾ����
    painter.setPen(QColor(119, 119, 119));
    bsRect = QRect(x6,oNPix/2+4*linPix+5,w6,linPix-2);
    QColor bsColor = QColor(145, 145, 145);
    linearGradient.setStart(x6,oNPix/2+4*linPix+5);
    linearGradient.setFinalStop(x6,oNPix/2+5*linPix+5);
    painter.setPen(QColor(255,255,255));
    if (yk > 0.0) {
        bsColor = QColor(171, 42, 44);
    } else if (yk < 0.0) {
        bsColor = QColor(87, 167, 56);
    } else {
        bsColor = QColor(145, 145, 145);
    }
    linearGradient.setColorAt(0,bsColor);
    linearGradient.setColorAt(1,bsColor);
    painter.setBrush(QBrush(linearGradient));
    painter.drawRect(bsRect);
    painter.setPen(QColor(240, 240, 240));
    painter.drawText(bsRect,Qt::AlignCenter, vnames[3]);
    painter.setFont(def);

    // �µ�������������
    painter.setPen(QColor(119, 119, 119));
    submitCountsR = QRect(x6,oNPix+3*linPix+1,w6,linPix-2);
    numL->setGeometry(submitCountsR);
    painter.drawRect(submitCountsR);
    painter.fillRect(submitCountsR,QColor(255, 255, 255));
    painter.setPen(QColor(34, 34, 34));
    painter.drawText(submitCountsR,Qt::AlignCenter, numL->text());

    // ��������
    painter.setPen(QColor(119, 119, 119));
    rcount1 = QRect(x6, oNPix+linPix*4+7, w6/2-1, linPix-2);
    rcount2 = QRect(x6+w6/2+1,oNPix+linPix*4+7,w6/2-1,linPix-2);
    rcount3 = QRect(x6,oNPix+linPix*5+7,w6/2-1,linPix-2);
    rcount5 = QRect(x6+w6/2+1,oNPix+linPix*5+7,w6/2-1,linPix-2);
    rcount10 = QRect(x6,oNPix+linPix*6+7,w6/2-1,linPix-2);
    rcount20 = QRect(x6+w6/2+1,oNPix+linPix*6+7,w6/2-1,linPix-2);

    linearGradient.setStart(x6, oNPix+linPix*4+7);						// 1
    linearGradient.setFinalStop(x6, oNPix+linPix*5+7);
    if(rcount1.contains(x,y)) {
        linearGradient.setColorAt(0,QColor(165,165,165));
        linearGradient.setColorAt(1,QColor(165,165,165));
    } else {
        linearGradient.setColorAt(0,QColor(145,145,145));
        linearGradient.setColorAt(1,QColor(145,145,145));
    }
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(rcount1, 1, 1);

    linearGradient.setStart(x6+w6/2+1,oNPix+linPix*4+7);				// 5
    linearGradient.setFinalStop(x6+w6/2+1,oNPix+linPix*5+7);
    if(rcount2.contains(x,y)) {
        linearGradient.setColorAt(0,QColor(165,165,165));
        linearGradient.setColorAt(1,QColor(165,165,165));
    } else {
        linearGradient.setColorAt(0,QColor(145,145,145));
        linearGradient.setColorAt(1,QColor(145,145,145));
    }
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(rcount2, 1, 1);

    linearGradient.setStart(x6,oNPix+linPix*5+7);						// 10
    linearGradient.setFinalStop(x6,oNPix+linPix*6+7);
    if(rcount3.contains(x,y)) {
        linearGradient.setColorAt(0,QColor(165,165,165));
        linearGradient.setColorAt(1,QColor(165,165,165));
    } else {
        linearGradient.setColorAt(0,QColor(145,145,145));
        linearGradient.setColorAt(1,QColor(145,145,145));
    }
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(rcount3, 1, 1);

    linearGradient.setStart(x6+w6/2+1,oNPix+linPix*5+7);				// 50
    linearGradient.setFinalStop(x6+w6/2+1,oNPix+linPix*6+7);
    if(rcount5.contains(x,y)) {
        linearGradient.setColorAt(0,QColor(165,165,165));
        linearGradient.setColorAt(1,QColor(165,165,165));
    } else {
        linearGradient.setColorAt(0,QColor(145,145,145));
        linearGradient.setColorAt(1,QColor(145,145,145));
    }
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(rcount5, 1, 1);

    linearGradient.setStart(x6,oNPix+linPix*6+7);						// 100
    linearGradient.setFinalStop(x6,oNPix+linPix*7+7);
    if(rcount10.contains(x,y)) {
        linearGradient.setColorAt(0,QColor(165,165,165));
        linearGradient.setColorAt(1,QColor(165,165,165));
    } else {
        linearGradient.setColorAt(0,QColor(145,145,145));
        linearGradient.setColorAt(1,QColor(145,145,145));
    }
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(rcount10, 1, 1);

    linearGradient.setStart(x6+w6/2+1,oNPix+linPix*6+7);				// 500
    linearGradient.setFinalStop(x6+w6/2+1,oNPix+linPix*7+7);
    if(rcount20.contains(x,y)) {
        linearGradient.setColorAt(0,QColor(165,165,165));
        linearGradient.setColorAt(1,QColor(165,165,165));
    } else {
        linearGradient.setColorAt(0,QColor(145,145,145));
        linearGradient.setColorAt(1,QColor(145,145,145));
    }
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(rcount20, 1, 1);

    painter.setPen(QColor(240, 240, 240));
    painter.drawText(rcount1,Qt::AlignCenter, "1");
    painter.drawText(rcount2,Qt::AlignCenter, "5");
    painter.drawText(rcount3,Qt::AlignCenter, "10");
    painter.drawText(rcount5,Qt::AlignCenter, "50");
    painter.drawText(rcount10,Qt::AlignCenter, "100");
    painter.drawText(rcount20,Qt::AlignCenter, "500");
    //
    //// ������������
    //painter.setPen(QColor(119, 119, 119));
    //cAllBuy = QRect(x6,oNPix+linPix*8+3,w6,linPix);
    //linearGradient.setStart(x6,oNPix+linPix*8+3);
    //linearGradient.setFinalStop(x6,oNPix+linPix*9+3);
    //if(cAllBuy.contains(x,y)) {
    //	linearGradient.setColorAt(0,QColor(13, 150, 214));
    //	linearGradient.setColorAt(1,QColor(10, 120, 171));
    //} else {
    //	linearGradient.setColorAt(0,QColor(10, 120, 171));
    //	linearGradient.setColorAt(1,QColor(8, 96, 137));
    //}
    //painter.setBrush(QBrush(linearGradient));
    //painter.drawRoundedRect(cAllBuy, 2, 2);
    //painter.setPen(QColor(240, 240, 240));
    //painter.drawText(cAllBuy,Qt::AlignCenter, "����");
    //
    //painter.setPen(QColor(119, 119, 119));
    //cAllSell = QRect(x6,oNPix+linPix*9+7,w6,linPix);
    //linearGradient.setStart(x6,oNPix+linPix*9+7);
    //linearGradient.setFinalStop(x6,oNPix+linPix*10+7);
    //if(cAllSell.contains(x,y)) {
    //	linearGradient.setColorAt(0,QColor(210, 54, 54));
    //	linearGradient.setColorAt(1,QColor(178, 40, 40));
    //} else {
    //	linearGradient.setColorAt(0,QColor(178, 40, 40));
    //	linearGradient.setColorAt(1,QColor(149, 33, 33));
    //}
    //painter.setBrush(QBrush(linearGradient));
    //painter.drawRoundedRect(cAllSell, 2, 2);
    //painter.setPen(QColor(240, 240, 240));
    //painter.drawText(cAllSell,Qt::AlignCenter, "����");
    //
    painter.setPen(QColor(119, 119, 119));
    //	cAll = QRect(x6,oNPix+linPix*10+11,w6,linPix);
    cAll = QRect(x6,oNPix+linPix*8+3,w6,linPix);
    linearGradient.setStart(x6,oNPix+linPix*8+3);
    linearGradient.setFinalStop(x6,oNPix+linPix*9+3);
    if(cAll.contains(x,y)) {
        linearGradient.setColorAt(0,QColor(165,165,165));
        linearGradient.setColorAt(1,QColor(165,165,165));
    } else {
        linearGradient.setColorAt(0,QColor(145,145,145));
        linearGradient.setColorAt(1,QColor(145,145,145));
    }
    painter.setBrush(QBrush(linearGradient));
    painter.drawRoundedRect(cAll, 2, 2);
    painter.setPen(QColor(240, 240, 240));
    painter.drawText(cAll,Qt::AlignCenter, QString::fromLocal8Bit("ȫ��"));
    if(tradeStatus != LIMIT && moveWPixs > 0 && moveHPixs > 0)
    {
        painter.setPen(QColor(0, 0, 0));
        painter.drawLine(moveWPixs,moveHPixs,x,y);
        double srt = qSqrt((moveWPixs-x)*(moveWPixs-x)+(moveHPixs-y)*(moveHPixs-y));
        if(srt < 1) srt = 1;
        int x1 = x+10*((moveWPixs-x)*cos_15-(moveHPixs-y)*sin_15)/srt;
        int y1 = y+10*((moveWPixs-x)*sin_15+(moveHPixs-y)*cos_15)/srt;
        int x2 = x+10*((moveWPixs-x)*cos_15f-(moveHPixs-y)*sin_15f)/srt;
        int y2 = y+10*((moveWPixs-x)*sin_15f+(moveHPixs-y)*cos_15f)/srt;
        QPainterPath jt;
        jt.moveTo(x,y);
        jt.lineTo(x1,y1);
        jt.lineTo(x2,y2);
        jt.lineTo(x,y);
        painter.fillPath(jt,QColor(0, 0, 0));
    }
    // ����׷��ֹ����
 /*   if(pploy && pploy->posi.size() > 0 && pploy->trailPriceu > 0.0001)
    {
        double steps = ( - pploy->trailPriceu)/minMove;
        int oLin  = 0;
        double dis = steps-(int)steps;
        if(dis > 0)
        {
            if(dis < 0.5)
            {
                oLin = (int)steps + 1 +(wheelSteps+linNum/2);
            }
            else
            {
                oLin = (int)steps + 1 + 1 +(wheelSteps+linNum/2);
            }
        }
        else
        {
            if(dis > -0.5)
            {
                oLin = (int)steps + 1 +(wheelSteps+linNum/2);
            }
            else
            {
                oLin = (int)steps -1 + 1 +(wheelSteps+linNum/2);
            }
        }
        if(pploy->posi.size() == 0)
            return;
        int px = 0;
        int px0 = 0;
        int py = 0;
        int py0 = 0;
        double step = pploy->points/minMove;
    /*	if( pploy->posi->BuyQty < pploy->posi->SellQty)
        {
            px = submitBR.x() + orderW*oPix[0]/200;
            py = oLin*linPix+1+linPix/2;
            if(tradeStatus == ZZMODE)
            {
                px0 = x;
                py0 = y;
            }
            else
            {
                px0 = submitBR.x() + orderW*oPix[4]/200;
                py0 = oLin*linPix+linPix-step*linPix;
            }
        }
        else*/
      /*  {
            px = submitSR.x() + orderW*oPix[4]/200;
            py = oLin*linPix+1+linPix/2;
            if(tradeStatus == ZZMODE)
            {
                px0 = x;
                py0 = y;
            }
            else
            {
                px0 = submitSR.x() + orderW*oPix[4]/200;
                py0 = oLin*linPix+step*linPix;
            }
        }
        double srt = qSqrt((px-px0)*(px-px0)+(py-py0)*(py-py0));
        if(srt < 1) srt = 1;
        int px1 = px0+10*((px-px0)*cos_15-(py-py0)*sin_15)/srt;
        int py1 = py0+10*((px-px0)*sin_15+(py-py0)*cos_15)/srt;
        int px2 = px0+10*((px-px0)*cos_15f-(py-py0)*sin_15f)/srt;
        int py2 = py0+10*((px-px0)*sin_15f+(py-py0)*cos_15f)/srt;
        painter.setPen(QColor(0,0,0));
        painter.drawLine(px0,py0,px,py);
        QPainterPath jt;
        jt.moveTo(px0,py0);
        jt.lineTo(px1,py1);
        jt.lineTo(px2,py2);
        jt.lineTo(px0,py0);
        painter.fillPath(jt,QColor(0, 0, 0));
    }*/
}
