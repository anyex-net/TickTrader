#include "orderManage.h"
#include "coolsubmit.h"
#include "tradewidget.h"
#include <QPainterPath>

#define TOPPIX 30
#define LINPIX 20
#define MINWID 800

// ������¼����
extern loginWin * loginW;
extern QString convertBsFlag(TThostFtdcDirectionType flag);
extern QString convertOcFlag(TThostFtdcDirectionType flag);
extern QString convertPriceType(TThostFtdcDirectionType flag);
extern QString convertConditionMethod(TThostFtdcDirectionType flag);
extern QString convertOrderStatus(TThostFtdcDirectionType flag);
extern QString convertExchangeID(TThostFtdcExchangeIDType ID);
extern quint32 CreateNewRequestID();

OrderManage::OrderManage(TradeWidget * tWidget, QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
	tw = tWidget;
	QIcon icon;
    icon.addFile(QString::fromUtf8(":/image/images/OE@32px.png"), QSize(), QIcon::Normal, QIcon::Off);
    this->setWindowIcon(icon);
	setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowTitle(QString::fromLocal8Bit("��������"));
	c_order = NULL;
	udNum = 0;
	orderStatus = false;
	moveWPixs = 9999;
	moveHPixs = 9999;
	wrows = 0;
	v_sc = 0;
	clickLine = 0;
	setMouseTracking(true);
	setMenu = new QMenu(this);
	QAction *spiAction = setMenu->addAction(QString::fromLocal8Bit("ֻ��ʾδ�ɽ�"));
	connect(spiAction, SIGNAL(triggered(bool)), this, SLOT(changeOrderStatus()));
	spiAction->setCheckable(true);
	// �ر�ʱ�Զ��ͷ�
	setAttribute(Qt::WA_DeleteOnClose, false);
}

OrderManage::~OrderManage()
{
}

// ���ڴ�С����
void OrderManage::resizeEvent(QResizeEvent * event)
{
	v_sc = 0;
}


// �Ҽ��˵�
void OrderManage::contextMenuEvent(QContextMenuEvent* e)
{
	setMenu->exec(cursor().pos()); 
}

// �Ҽ� ��ֻ��ʾ��Ծ����
void OrderManage::changeOrderStatus()
{
	orderStatus = !orderStatus;
	update();
}

void OrderManage::paintEvent(QPaintEvent * event)
{
	//int x = QWidget::mapFromGlobal(cursor().pos()).x();
	//int y = QWidget::mapFromGlobal(cursor().pos()).y();
	QPainter painter(this); 
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(QColor(34, 34, 34)); 
	// ���ư�ť����
	paintBox(&painter);
	// ���ƶ������
	paintOrderTable(&painter);
}

// ���ư�ť����
void OrderManage::paintBox(QPainter * p)
{
	// top 70���� �������ڻ��ư�ť
	p->fillRect(QRect(0,0,width(),24),QColor(173,173,173));
	//// ���۰�ť
	//upriceRect = QRect(5,5,2*TOPPIX,2*TOPPIX);
    QLinearGradient linearGradient(5,5,5,2*TOPPIX+5);
	// ������ť
	p->setPen(QColor(210, 210, 210));
	cancelRect = QRect(2,2,55,20);
	linearGradient.setStart(2,2);
	linearGradient.setFinalStop(2,22);
	linearGradient.setColorAt(0,QColor(138,138,138));
	linearGradient.setColorAt(1,QColor(138,138,138));
	p->setBrush(QBrush(linearGradient));
	p->drawRoundedRect(cancelRect,2,2);
	p->setPen(QColor(255,255,255));
    p->drawText(cancelRect,Qt::AlignCenter, QString::fromLocal8Bit("����"));
}

// ���ư�ť����
void OrderManage::paintOrderTable(QPainter * p)
{
	QPen pen(QColor(240,210,6));
	pen.setWidth(2);
	p->setPen(pen);
	int marTop = 24;
	int wid_ = width()>MINWID?width():MINWID;
	int vscl = v_sc*wid_/width(); // ���������ƫ������
	int pvh = height()-16;
	int vbw = width()*width()/wid_;
	QRect tableRect = QRect(0,marTop,width(),height()-marTop);
	p->fillRect(tableRect, QColor(225, 225, 225));
	int linPix = marTop;
	TradeInfo * ti = &(tradeInfoLst[loginW->userName]);
	linPix += LINPIX;
	p->setPen(QColor(203, 203, 203));
	p->drawLine(0,linPix,wid_,linPix);
    int wspan[8] = {11,11,11, 11, 11, 11, 11, 12};
    int wrats[8] = {0,0,0,       0,0,0,     0,};
    for(int i=1;i<8;i++)
	{
		wrats[i]=wrats[i-1]+wspan[i-1];
	}
    QString otitle[8] = {QString::fromLocal8Bit("����ʱ��")
						 ,QString::fromLocal8Bit("��Ʒ"), QString::fromLocal8Bit("����")
						 //,QString::fromLocal8Bit("��ƽ")
						 ,QString::fromLocal8Bit("�۸�")
						 ,QString::fromLocal8Bit("����"),
						 QString::fromLocal8Bit("�ɽ�") // QString::fromLocal8Bit("ʣ��")
                         //,QString::fromLocal8Bit("�ɽ���")
                         ,QString::fromLocal8Bit("״̬")
						 , QString::fromLocal8Bit("��ע")
						};
	int afg = Qt::AlignLeft|Qt::AlignVCenter;
	QFont def = p->font();
	QFont font;
	font.setFamily(QString::fromUtf8("΢���ź�"));
	font.setBold(true);
	p->setFont(font);
    for(int c=0;c<7;c++)
	{
		int c1 = wid_*wrats[c]/100 - vscl;
		int c2 = wid_*wrats[c+1]/100 - vscl;
		p->setPen(QColor(203, 203, 203));
		p->drawLine(c2,linPix-LINPIX,c2,linPix);
		QPen tpen(QColor(34, 34, 34));
		tpen.setWidth(2);
		p->setPen(tpen);
		afg = Qt::AlignLeft|Qt::AlignVCenter;
		if(c>=3 && c<=6)
				afg = Qt::AlignRight|Qt::AlignVCenter;
		p->drawText(QRect(c1+2,linPix-LINPIX,c2-c1-4,LINPIX),afg, otitle[c]);
	}
    p->drawText(QRect(wid_*wrats[7]/100 - vscl+2,linPix-LINPIX,wid_*(100-wrats[7])/100-4,LINPIX),afg, otitle[7]);
	p->setFont(def);
//    qInfo() << "ti.orderLst size: " << ti->orderLst.size();
	bool jo = false; // ��ż����ʾ false�� trueż
	// ������ʱ������
    QMap<QString, CThostFtdcOrderField *> oLst;// ���˻���Ӧ�Ķ�����Ϣ KEY:����ʱ��
    QMapIterator<QString, CThostFtdcOrderField *> s(ti->orderLst);
	int dis = 0; // �������ֶ���ʱ�䲻һ�������
	int showNum = 0;
	while (s.hasNext())
	{
        CThostFtdcOrderField * sOrder = ti->orderLst[s.next().key()];
        if(!sOrder)
            continue;
		if(pQuotApi && tw->insMap_dy[QString::fromLocal8Bit(sOrder->InstrumentID)] == NULL)
		{
			// ����ǰ�ȶ���
            int count = 1;
            char **InstrumentID = new char*[count];
            InstrumentID[0] = sOrder->InstrumentID;
            pQuotApi->SubscribeMarketData(InstrumentID, 1);
			tw->insMap_dy[QString::fromLocal8Bit(sOrder->InstrumentID)] = tw->insMap[QString::fromLocal8Bit(sOrder->InstrumentID)];
            delete[] InstrumentID;
		}
        if(orderStatus && sOrder->OrderStatus != THOST_FTDC_OST_NoTradeQueueing && sOrder->OrderStatus != THOST_FTDC_OST_NotTouched)
			continue;
        QString dt = QString(sOrder->InsertDate).append(QString(sOrder->InsertTime));
		if(!oLst.contains(dt))
		{
			oLst[dt] = sOrder;
		}
		else
		{
			oLst[dt.append(QString::number(dis))] = sOrder;
			dis++;
		}
		showNum++;
	}
	//if(showNum == 0)
	//{
	//	QFont def = p->font();
	//	QFont font;
	//	font.setFamily(QString::fromUtf8("Cambria Math"));
	//	font.setPointSize(30);
	//	p->setFont(font);
	//	p->setPen(QColor(34, 34, 34));
	//	p->drawText(tableRect,Qt::AlignCenter, QString::fromLocal8Bit("���޶���"));
	//	p->setFont(def);
	//	return;
	//}
    QMapIterator<QString, CThostFtdcOrderField *> op(oLst);
	op.toBack();
	int igrows = wrows>0?wrows:0;
	while (op.hasPrevious() && linPix<height())
	{
        CThostFtdcOrderField * sOrder = oLst[op.previous().key()];
        if(!sOrder || sOrder->VolumeTotalOriginal <= 0) continue;
		if(igrows > 0)
		{
			igrows --;
			continue;
		}
		linPix += LINPIX;
        CThostFtdcInstrumentField * sbInstr = tw->insMap[QString::fromLocal8Bit(sOrder->InstrumentID)];
		QColor tPenc = QColor(66, 124, 211);
		if(!sbInstr)
			continue;
		int scale = tw->getScale(sOrder->InstrumentID);
		if(jo)
		{
			p->fillRect(QRect(0,linPix-LINPIX,wid_,LINPIX),QColor(221,223,225));
		}
		if(marTop+clickLine*LINPIX == linPix)
		{
			if(c_order != sOrder)
				udNum = 0;
			c_order = sOrder;
			tPenc = QColor(193, 40, 44);
			p->fillRect(QRect(0,linPix-LINPIX,wid_,LINPIX),QColor(255,255,255));
		}
		jo = !jo;
		p->setPen(QColor(203, 203, 203));
		p->drawLine(0,linPix,wid_,linPix);
        otitle[0] = QString::fromLocal8Bit(sOrder->InsertTime);/* ����ʱ�� */
		otitle[1] = QString::fromLocal8Bit(sbInstr->InstrumentName);/* ��Լ�� */
        otitle[2] = convertBsFlag(sOrder->Direction).append(convertOcFlag(sOrder->CombOffsetFlag[0]));/* ������־ *//* ��ƽ��־ */
        otitle[3] = QString::number(sOrder->LimitPrice,'f',scale);/* �۸� */
        otitle[4] = QString::number(sOrder->VolumeTotalOriginal);/* ���� */
        otitle[5] = QString::number(sOrder->VolumeTotalOriginal - sOrder->VolumeTotal); /* �ɽ� */  /* ʣ������ */
//        otitle[6] = QString::number(sOrder->LimitPrice,'f',scale);   /* �ɽ����� */
        otitle[6] = convertOrderStatus(sOrder->OrderStatus);  /* ״̬ */
//		otitle[8] = QString::fromLocal8Bit(sOrder->OrderID);  /* ������ */
        if( strcmp( sOrder->StatusMsg, "ȫ���ɽ��������ύ" ) == 0 ){
            if(sOrder->ContingentCondition == THOST_FTDC_CC_Immediately) // �������� Ϊ�޵�ʱ�� �޼۵� д�ڱ�עһ��
            {
                otitle[7] = QString::fromLocal8Bit(LIMITORDER);   /* �޼۵� */
            }
            else
            {
                if(sOrder->OrderPriceType == THOST_FTDC_OPT_AnyPrice) // �۸�����Ϊ�м�
                {
    //				switch(sOrder->FilledPoints)
    //				{
    //				case 1:
    //					otitle[8] = QString::fromLocal8Bit(ONETICK);   /* +1tick */
    //					break;
    //				case 2:
    //					otitle[8] = QString::fromLocal8Bit(TWOTICK);   /* +2tick */
    //					break;
    //				case 3:
    //					otitle[8] = QString::fromLocal8Bit(THREETICK);   /* +3tick */
    //					break;
    //				}
                    otitle[7] = QString::fromLocal8Bit(ONETICK);
                }
                else
                {
                    otitle[7] = QString::fromLocal8Bit(STOPLIMITORDER);   /* ֹ���޼۵� */
                }
            }
        }
        else{
            otitle[7] = QString::fromLocal8Bit(sOrder->StatusMsg);
        }

//		otitle[10] = QString::fromLocal8Bit(sOrder->DetailStatus);  /* ��ϸ״̬ */
		int afg = Qt::AlignLeft|Qt::AlignVCenter;
        for(int rc=0;rc<8;rc++)
		{
			int c1 = wid_*wrats[rc]/100 - vscl;
			int c2 = wid_*wrats[rc+1]/100 - vscl;
			p->setPen(QColor(189, 189, 189));
			p->drawLine(c2,linPix-LINPIX,c2,linPix);
			p->setPen(tPenc);
			afg = Qt::AlignLeft|Qt::AlignVCenter;
			if(rc>=3 && rc<=6)
				afg = Qt::AlignRight|Qt::AlignVCenter;
			p->setPen(QColor(0, 0, 0));
			p->drawText(QRect(c1+2,linPix-LINPIX,c2-c1-4,LINPIX), afg, otitle[rc]);
		}
        p->drawText(QRect(wid_*wrats[7]/100 - vscl+2,linPix-LINPIX,width()*(100-wrats[7])/100-4,LINPIX), afg, otitle[7]);
	}
	if(wid_ > width())
	{
		vsBarR = QRect(0,pvh,wid_,16);
		p->fillRect(vsBarR,QColor(119, 119, 119));
		vsBarRt = QRect(v_sc,pvh+1,vbw,14);
		p->fillRect(vsBarRt,QColor(238, 238, 238));
	}
}


// ��¼��갴��ʱ��״̬
void OrderManage::mousePressEvent(QMouseEvent * event)
{
	if (event->button() != Qt::LeftButton) {
		return;
	}
	moveWPixs = event->x();
	moveHPixs = event->y();
	update();
}


// ����ƶ�ʱ���¼�����
void OrderManage::mouseMoveEvent(QMouseEvent * event)
{
	if(moveWPixs < width() && width() < MINWID)
	{
		v_sc += event->pos().x()-moveWPixs;
		moveWPixs = event->pos().x();
		v_sc = v_sc<0?0:v_sc;
		v_sc = v_sc+vsBarRt.width()>width()?width()-vsBarRt.width():v_sc;
	}
	update();
}


// ��¼����ɿ�ʱ��״̬
void OrderManage::mouseReleaseEvent(QMouseEvent * event)
{
	if(event->button() != Qt::LeftButton)
	{
		return;
	}
	int x = event->pos().x();
	int y = event->pos().y();
	moveWPixs = 9999;
	moveHPixs = 9999;
    CThostFtdcInstrumentField * csi = NULL;
	double minMove = 0.01;
	if(y> 44)
		clickLine = (y-24)/LINPIX+1;
	if(c_order)
	{
		csi = tw->insMap[QString(c_order->InstrumentID)];
		if(csi)
            minMove = csi->PriceTick;
	}
	//if(changeRect.contains(event->pos()))
	//{
	//	if(c_order && csi && c_order->BSFlag != BCESConstBSFlagExecute)
	//	{
	//		updateOrder(c_order, c_order->Price+udNum);
	//		udNum = 0;
	//	}
	//}
	//if(upriceRect.contains(event->pos()) && upriceRect.contains(QPoint(x,y+40))) // ��
	//{
	//	udNum += minMove;
	//}
	//if(upriceRect.contains(event->pos()) && upriceRect.contains(QPoint(x,y-40))) // ��
	//{
	//	udNum -= minMove;
	//}
//    if(upOneRect.contains(event->pos()) && c_order) // ����
//	{
//        updateOrder(c_order, c_order->LimitPrice+minMove);
//	}
//    if(dnOneRect.contains(event->pos()) && c_order) // ����
//	{
//        updateOrder(c_order, c_order->LimitPrice-minMove);
//	}
	if(cancelRect.contains(event->pos()))
	{
		dropOrder(c_order);
	}
	//if(checkRect.contains(event->pos()))
	//{
	//	orderStatus = !orderStatus;
	//}
	update();
}

// ����˫���¼�
void OrderManage::mouseDoubleClickEvent(QMouseEvent * event)
{
	if(!c_order) return;
	if(!tw->CSubmit) return;
	tw->CSubmit->selectInstr->curInstr = tw->insMap[QString::fromLocal8Bit(c_order->InstrumentID)];
	tw->CSubmit->initByInstr();
	tw->CSubmit->update();
}

// �����������¼�
void OrderManage::wheelEvent(QWheelEvent * event)
{
	if(event->delta() > 0)
	{
		if(wrows>0)
		{
			wrows--;
			clickLine ++;
		}
	}
	else if(event->delta() < 0)
	{
		clickLine --;
		wrows++;
	}
	update();
}

// �޸Ķ���
void OrderManage::updateOrder(CThostFtdcOrderField * co, double price2)
{
	if(!co)
		return;
    CThostFtdcInstrumentField * ci = tw->insMap[co->InstrumentID];
	if(!ci)
		return;
	TradeInfo * ti = &(tradeInfoLst[loginW->userName]);
    if(co->ContingentCondition == THOST_FTDC_CC_Immediately && (co->OrderStatus == THOST_FTDC_OST_NoTradeQueueing || co->OrderStatus == THOST_FTDC_OST_PartTradedQueueing))
	{
		NEWORDERINF noi;
		::memset(&noi,0,sizeof(NEWORDERINF));
        sprintf(noi.tif, "%s", loginW->userName);
        strncpy(noi.OrderRef,co->OrderRef,sizeof(noi.OrderRef)); /* ������ */
		strncpy(noi.InvestorID,co->InvestorID,sizeof(noi.InvestorID)); /* Ͷ���ߺ� */
		strncpy(noi.InstrumentID,co->InstrumentID,sizeof(noi.InstrumentID)); /* ��Լ�� */
        noi.Direction = co->Direction; /* ������־ */
        noi.CombOffsetFlag[0] = co->CombOffsetFlag[0]; /* ��ƽ��־ */
        noi.PriceType = co->OrderPriceType;	/* �۸����� */
		noi.Price = price2;	/* �۸� */
        noi.ConditionMethod = co->ContingentCondition;	/* �������� */
        noi.Qty = co->VolumeTotal;	/* ʣ������ */
		strncpy(noi.ExchangeID, co->ExchangeID,sizeof(noi.ExchangeID));
		tw->o2upLst.append(noi);
		int nRequestID = CreateNewRequestID();
        CThostFtdcInputOrderActionField pCancelReq;
        ::memset(&pCancelReq,0,sizeof(CThostFtdcInputOrderActionField));
        strncpy(pCancelReq.OrderRef,co->OrderRef,sizeof(pCancelReq.OrderRef)); /* ������ */
		strncpy(pCancelReq.InvestorID,co->InvestorID,sizeof(pCancelReq.InvestorID)); /* Ͷ���ߺ� */
		strncpy(pCancelReq.InstrumentID,co->InstrumentID,sizeof(pCancelReq.InstrumentID)); /* ��Լ�� */
		strncpy(pCancelReq.ExchangeID,ci->ExchangeID,sizeof(pCancelReq.ExchangeID)); /* �������� */
        ti->api->ReqOrderAction(&pCancelReq, nRequestID); // ��������
	}
    if((co->OrderPriceType == THOST_FTDC_OPT_AnyPrice || co->OrderPriceType == THOST_FTDC_OPT_LimitPrice)&& co->OrderStatus == THOST_FTDC_OST_NotTouched)  // ֹ��
	{
		int scale = tw->getScale(co->InstrumentID);
		// �������Ȿ�ض����۸�
        co->LimitPrice = price2;
	}
	tw->CSubmit->update();
}

// ɾ������
void OrderManage::dropOrder(CThostFtdcOrderField * co)
{
	if(!co)
		return;
    CThostFtdcInstrumentField * ci = tw->insMap[co->InstrumentID];
	if(!ci)
		return;
	TradeInfo & ti = tradeInfoLst[loginW->userName];
    if(co->OrderStatus == THOST_FTDC_OST_NoTradeQueueing || co->OrderStatus == THOST_FTDC_OST_PartTradedQueueing) // ��������
	{
		int nRequestID = CreateNewRequestID();
        CThostFtdcInputOrderActionField pCancelReq;
        ::memset(&pCancelReq,0,sizeof(CThostFtdcInputOrderActionField));
        strncpy(pCancelReq.OrderRef,co->OrderRef,sizeof(pCancelReq.OrderRef)); /* ������ */
		strncpy(pCancelReq.InvestorID,co->InvestorID,sizeof(pCancelReq.InvestorID)); /* Ͷ���ߺ� */
		strncpy(pCancelReq.InstrumentID,co->InstrumentID,sizeof(pCancelReq.InstrumentID)); /* ��Լ�� */
		strncpy(pCancelReq.ExchangeID,ci->ExchangeID,sizeof(pCancelReq.ExchangeID)); /* �������� */
        ti.api->ReqOrderAction(&pCancelReq, nRequestID);
        //if(ti.api->ReqOrderAction(&pCancelReq, nRequestID) == 0)
		//{
		//	QMessageBox::about(NULL,QString::fromLocal8Bit("��ʾ"),QString::fromLocal8Bit("�ѷ��Ͷ�����������"));
		//}
		//else
		//{
		//	QMessageBox::about(NULL,QString::fromLocal8Bit("��ʾ"),QString::fromLocal8Bit("������������ʧ�ܣ�"));
		//}
	}
    if(co->OrderStatus == THOST_FTDC_OST_NotTouched) // ����ֹ��
	{
        co->OrderStatus = THOST_FTDC_OST_Canceled;
        tw->checkOcoStatus(ti, co->OrderRef);
	}
}
