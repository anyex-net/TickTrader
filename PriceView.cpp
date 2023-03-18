#include "PriceView.h"
#include "coolsubmit.h"
#include "tradewidget.h"
#include <time.h>
#include <cfloat>
#include "configWidget.h"

#define LINPIX 20

// ���ý���
extern configWidget * cview;
extern loginWin * loginW;
//extern void writePrivates(PrivateIns & pi);
// �߳���
//extern QMutex mutex;
extern QString convertBsFlag(TThostFtdcDirectionType flag);
extern QString convertOcFlag(TThostFtdcDirectionType flag);
extern QString convertPriceType(TThostFtdcDirectionType flag);
extern QString convertConditionMethod(TThostFtdcDirectionType flag);
extern QString convertOrderStatus(TThostFtdcDirectionType flag);
extern QString convertExchangeID(TThostFtdcExchangeIDType ID);

PriceView::PriceView(TradeWidget * tWidget, QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
	tw = tWidget;
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/image/images/price@32px.png"), QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(icon);
	setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowTitle(QString::fromLocal8Bit("���۰�"));
	connect(tw, SIGNAL(floating()), this, SLOT(update()));
	setMouseTracking(true);
	wrows = 0;
	clickLine = 0;
	setMenu = new QMenu(this);
	//QAction *orderAction = setMenu->addAction(QString::fromLocal8Bit("�µ�"));
	//connect(orderAction, SIGNAL(triggered(bool)), this, SLOT(showTradeW())); // todo
	QAction *spiAction = setMenu->addAction(QString::fromLocal8Bit("�����ѡ"));
	connect(spiAction, SIGNAL(triggered(bool)), this, SLOT(addToSelfMap())); // todo
	QAction *delAction = setMenu->addAction(QString::fromLocal8Bit("ɾ����ѡ"));
	connect(delAction, SIGNAL(triggered(bool)), this, SLOT(delFrSelfMap())); // todo
//	QAction *kLinAction = setMenu->addAction(QString::fromLocal8Bit("K��"));
//	connect(kLinAction, SIGNAL(triggered(bool)), this, SLOT(showCurve())); // todo
	////////////////////////////////////////////////
	priRect = new PrivateInstrs;
	priRect->name = QString::fromLocal8Bit("��ѡ��Լ");
	/////////////////////////////////////////////////
	zlsRect = new PrivateInstrs;;
	zlsRect->name = QString::fromLocal8Bit("������Լ");
	curPi = zlsRect;
	moveWPixs = 9999;
	moveHPixs = 9999;
	v_sc = 0;
	h_sc = 0;
	selectInstr = new InstrManage(NULL, this);
	mapTemp = &tw->insMap_zl;
	selectInstr->minMap = mapTemp;
	connect(selectInstr, SIGNAL(changed()), this, SLOT(getInstr()));
	selectInstr->hide();
	// �ر�ʱ�Զ��ͷ�
	setAttribute(Qt::WA_DeleteOnClose, false);
}

PriceView::~PriceView()
{

}

void PriceView::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	// ������ѡ��
	paintPriBoard(&painter);
	// ���Ʊ��۰�
	paintPriceBoard(&painter);
	selectInstr->setGeometry(width()-200,height()-5*LINPIX,200,5*LINPIX);
}

// �����¼�
void PriceView::keyPressEvent( QKeyEvent * event )
{
	selectInstr->setGeometry(width()-200,height()-5*LINPIX,200,5*LINPIX);
	if(event->key() >= 0x30 && event->key() <= 0x39) // 0--9
	{
		selectInstr->init(NULL);
		QString text = selectInstr->searchBox->text();
		selectInstr->searchBox->setText(text.append(event->key()));
	}
	else if(event->key() >= 0x41 && event->key() <= 0x5A) //a--z
	{
		selectInstr->init(NULL);
		QString text = selectInstr->searchBox->text();
		selectInstr->searchBox->setText(text.append(event->key()));
	}
	update();
}

void PriceView::contextMenuEvent(QContextMenuEvent* e)  
{  
	int x = QWidget::mapFromGlobal(cursor().pos()).x();
	int y = QWidget::mapFromGlobal(cursor().pos()).y();
	clickLine = y / LINPIX;
	update();
	if(y < setsRect.y())
	{
		setMenu->exec(cursor().pos()); 
	}
}

// ���Ʒ���
void PriceView::paintPriBoard(QPainter *p)
{
	priRect->privateSet = QRect(2,height()-23,60,22);
	zlsRect->privateSet = QRect(64,height()-23,60,22);
	setsRect = QRect(0,height()-24,width(),24);
	p->fillRect(setsRect, QColor(135,135,135));
	QLinearGradient linearGradient(2,height()-23,64,height()-23);
	QColor curP = QColor(200,200,200);
	QColor curZ = QColor(200,200,200);
	if(curPi == priRect)
	{
		curP = QColor(255,255,255);
	}
	if(curPi == zlsRect)
	{
		curZ = QColor(255,255,255);
	}
	linearGradient.setColorAt(0,curP);
	linearGradient.setColorAt(1,curP);
	p->setBrush(QBrush(linearGradient));
	p->drawRoundedRect(priRect->privateSet, 1, 1);
	linearGradient.setColorAt(0,curZ);
	linearGradient.setColorAt(1,curZ);
	p->setBrush(QBrush(linearGradient));
	p->drawRoundedRect(zlsRect->privateSet, 1, 1);
	//linearGradient.setStart(curPi->privateSet.x(),curPi->privateSet.y());				// 5
	//linearGradient.setFinalStop(curPi->privateSet.x(),curPi->privateSet.y()+curPi->privateSet.height());
	//linearGradient.setColorAt(0,QColor(221,223,225));
	//linearGradient.setColorAt(1,QColor(221,223,225));
	//p->setBrush(QBrush(linearGradient));
	//p->drawRoundedRect(curPi->privateSet, 2, 2);
	p->setPen(QColor(54,54,54));
	p->drawText(priRect->privateSet, Qt::AlignCenter, priRect->name);
	p->drawText(zlsRect->privateSet, Qt::AlignCenter, zlsRect->name);
}

// ȡ����������
void PriceView::cancelQuotReq()
{
    QMap<QString, CThostFtdcInstrumentField *> temp_dy;
    QMapIterator<QString, CThostFtdcInstrumentField *> s(tw->insMap_dy);
	while (s.hasNext())
	{
		QString key = s.next().key();
        CThostFtdcInstrumentField * si = tw->insMap_dy[key];
		if(!si) continue;
		if(checkInstr(si))
		{
			temp_dy[key] = si;
			continue;
		}
		// ȡ����������
        int count = 1;
        char **InstrumentID = new char*[count];
        InstrumentID[0] = si->InstrumentID;
		tw->insMap_dy[QString::fromLocal8Bit(si->InstrumentID)] = NULL;
        pQuotApi->UnSubscribeMarketData(InstrumentID, 1); // ȡ������
        delete[] InstrumentID;
	}
	tw->insMap_dy.clear();
	tw->insMap_dy = temp_dy;
}

// У�鵱ǰ��Լ�Ƿ���������б��� true �� false ����
bool PriceView::checkInstr(CThostFtdcInstrumentField * is)
{
	// ��Լ���Ƿ�Ϊ��
	if(!is || ::strcmp(is->InstrumentID,"") == 0)
		return true;
	// �������Ƿ��Ǵ˺�Լ
	if(tw->CSubmit && tw->CSubmit->selectInstr->curInstr == is)
		return true;
	// ��ѡ���Ƿ��д˺�Լ
    QMap<QString, CThostFtdcInstrumentField *>::const_iterator iter = tw->insMap_zx.find(QString::fromLocal8Bit(is->InstrumentID));
    CThostFtdcInstrumentField * zxins = iter != tw->insMap_zx.end() ? iter.value():NULL;
	if(zxins)
		return true;
	// ��ʾ�ĺ�Լ
    QMap<QString, CThostFtdcInstrumentField *>::const_iterator zliter = viewTemp.find(QString::fromLocal8Bit(is->InstrumentID));
    CThostFtdcInstrumentField * zlins = zliter != viewTemp.end() ? zliter.value():NULL;
	if(zlins)
		return true;
	// K��ͼ�Ƿ��д˺�Լ
//	for(int s=0;s<CUnitList.length();s++)
//	{
//		CurveUnit * cU = CUnitList[s];
//		if(cU && cU->InstrumentID == QString(is->InstrumentID))
//			return true;
//	}
	QMapIterator<QString, TradeInfo> ti(tradeInfoLst);
	while (ti.hasNext())
	{
		QString key = ti.next().key();
		TradeInfo & tti = tradeInfoLst[key];
		QMap<QString,PosiPloy *>::const_iterator iter = tti.posiLst.find(QString::fromLocal8Bit(is->InstrumentID));
		PosiPloy * pPloy = iter != tti.posiLst.end() ? iter.value():NULL;
		if(pPloy)
		{
			return true;
		}
        QMapIterator<QString, CThostFtdcOrderField *> ort(tti.orderLst);
		while(ort.hasNext())
		{
			QString keyP = ort.next().key();
            CThostFtdcOrderField * so = tti.orderLst[keyP];
			if(so && ::strcmp(so->InstrumentID, is->InstrumentID) == 0)
			{
				return true;
			}
		}
	}
	return false;
}

// ���ڴ�С����
void PriceView::resizeEvent(QResizeEvent * event)
{
	v_sc = 0;
	h_sc = 0;
}

// ���Ʊ��۰�
void PriceView::paintPriceBoard(QPainter *p) {
	if(!mapTemp)
		mapTemp = &tw->insMap_zx;
	int x = QWidget::mapFromGlobal(cursor().pos()).x();
	int y = QWidget::mapFromGlobal(cursor().pos()).y();
	int marTop = 0;
	int marRight = 20;
	int marButtom = 30;
	int width_ = width();
	int pvw = width();
	pvw = pvw<DEFAULT_W?DEFAULT_W:pvw; // ��С���1000
	int pvh = height()-setsRect.height();
	pvh = pvw > width()?pvh-16:pvh; // �к����������ʱ��ͼ��߶ȼ�16
	int vbw = width()*width()/pvw; // ����������
	int vscl = v_sc*pvw/width(); // ���������ƫ������
	int lines = (pvh-marTop)/LINPIX; // һҳ�������
	if(mapTemp->count() > lines)
	{
		width_ -= 16;
		marButtom = 16;
	}
	QRect boardRect = QRect(0, marTop, width(), pvh - marTop);
	p->fillRect(boardRect, QColor(225, 225, 225));
	int linPix = marTop;
	p->setPen(QColor(203, 203, 203));
	p->drawLine(0, linPix + LINPIX, width_, linPix + LINPIX);
	int wspan[21] = {4,5,5,5,4,   5,5,4,5,5,   5,5,5,5,5,5,4,   4,5,5,5};
	int wrats[21] = {0,0,0,0,0,   0,0,0,0,0,   0,0,0,0,0,0,0,   0,0,0,0};
	for(int i=1;i<21;i++)
	{
		wrats[i]=wrats[i-1]+wspan[i-1];
	}
	QString quotTitle[21] = {
		QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����"), 
		QString::fromLocal8Bit("�ּ�"),
		QString::fromLocal8Bit("�Ƿ�"), 
		//QString::fromLocal8Bit("����"),
		QString::fromLocal8Bit("����"),
		QString::fromLocal8Bit("���"), QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����"),
		QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���"), QString::fromLocal8Bit("���"),
		QString::fromLocal8Bit("�ɽ���"),
		QString::fromLocal8Bit("�ֲ���"), QString::fromLocal8Bit("�ɽ���(��)"), 
		QString::fromLocal8Bit("��ͣ���"), QString::fromLocal8Bit("��ͣ���"), 
		QString::fromLocal8Bit("���"),
		//QString::fromLocal8Bit("��ֲ���"), 
		QString::fromLocal8Bit("���"), QString::fromLocal8Bit("����"),
		QString::fromLocal8Bit("ʱ��"), QString::fromLocal8Bit("������")
	};
	QFont def = p->font();
	QFont font;
	font.setFamily(QString::fromUtf8("΢���ź�"));
	font.setBold(true);
	p->setFont(font);
	int afg = Qt::AlignLeft|Qt::AlignVCenter;
	for (int i = 0; i < 20; ++i) {
		int c1 = pvw * wrats[i] / 100 - vscl;
		int c2 = pvw * wrats[i+1] / 100 - vscl;
		if(c2 < 0 || c1 > width())
			continue;
		p->setPen(QColor(203, 203, 203));
		p->drawLine(c2, linPix, c2, linPix + LINPIX);
		QPen tpen(QColor(34, 34, 34));
		tpen.setWidth(2);
		p->setPen(tpen);
		afg = Qt::AlignLeft|Qt::AlignVCenter;
		if(i>1 && i<19)
				afg = Qt::AlignRight|Qt::AlignVCenter;
		p->drawText(QRect(c1, linPix, c2 - c1, LINPIX), afg, quotTitle[i]); 
	}
	p->drawText(QRect(pvw * wrats[20] / 100 - vscl, linPix, pvw * (100 - wrats[18]) / 100, LINPIX), afg, quotTitle[20]);
	p->setFont(def);
	bool jo = false;
	bool check = false;
    QMapIterator<QString, CThostFtdcInstrumentField *> i(*mapTemp);
	int index = 0;
	int igrows = wrows > 0 ? wrows : 0;
	// �������
	cancelQuotReq();
	viewTemp.clear();
	int hscl = 100;
	while (i.hasNext() && linPix < pvh - LINPIX) {
		QString key = i.next().key();
		if(key == "")
			continue;
		if(igrows > 0)
		{
			igrows --;
			continue;
		}
        CThostFtdcInstrumentField *sbInstr = mapTemp->value(key);
		if(!sbInstr)
		{
			continue;
		}
		viewTemp[sbInstr->InstrumentID] = sbInstr;
        CThostFtdcDepthMarketDataField * pp = tw->quotMap[QString::fromLocal8Bit(sbInstr->InstrumentID)];
		if(!pp)
		{
            pp = new CThostFtdcDepthMarketDataField;
            ::memset(pp,0,sizeof(CThostFtdcDepthMarketDataField));
		}
		if(pQuotApi && tw->insMap_dy[QString::fromLocal8Bit(sbInstr->InstrumentID)] == NULL)
		{
            int count = 1;
            char **InstrumentID = new char*[count];
            InstrumentID[0] = sbInstr->InstrumentID;
            pQuotApi->SubscribeMarketData(InstrumentID, 1); // ��������
			tw->insMap_dy[QString::fromLocal8Bit(sbInstr->InstrumentID)] = sbInstr;
            delete[] InstrumentID;
		}
		linPix += LINPIX;
		int scale = tw->getScale(pp->InstrumentID);
		if(jo)
		{
			p->fillRect(QRect(0,linPix+1,width_,LINPIX-1),QColor(220,220,220));
		}
		if(marTop+clickLine*LINPIX == linPix)
		{
			selectInstr->curInstr = sbInstr;
			p->fillRect(QRect(0,linPix+1,width_,LINPIX-1),QColor(255,255,255));
		}
		jo = !jo;
		double zf = 0; // �Ƿ�
        if(pp->LastPrice < 0.0001)
		{
        }else if(pp->PreSettlementPrice > 0.0001) // ������
		{
            zf = 100*(pp->LastPrice-pp->PreSettlementPrice)/pp->PreSettlementPrice;
		}
        else if(pp->PreClosePrice > 0.0001) //�����̼�
		{
            zf = 100*(pp->LastPrice-pp->PreClosePrice)/pp->PreClosePrice;
		}
		p->setPen(QColor(203, 203, 203));
		p->drawLine(0, linPix + LINPIX, pvw, linPix + LINPIX);
		quotTitle[0] = QString::fromLocal8Bit(sbInstr->InstrumentID);			/* ��Ʒ���� */
		quotTitle[1] = QString::fromLocal8Bit(sbInstr->InstrumentName);  		/* ��Ʒ���� */
        if(pp->LastPrice < DBL_MAX/2)
            quotTitle[2] = QString::number(pp->LastPrice,'f',scale);				/* �ּ� */
		else
			quotTitle[2] = "-";				/* �ּ� */
		quotTitle[3] = QString::number(zf,'f',1).append("%");		/* �Ƿ� */
        quotTitle[4] = QString::number(pp->BidVolume1);						/* ������ */
        quotTitle[5] = QString::number(pp->BidPrice1,'f',scale);			/* ����� */
		//quotTitle[3] = QString::number(pp->Qty);					  		/* ���� */
        quotTitle[6] = QString::number(pp->AskPrice1,'f',scale);			/* ������ */
        quotTitle[7] = QString::number(pp->AskVolume1);						/* ������ */
		quotTitle[8] = QString::number(pp->OpenPrice,'f',scale);			/* ���̼� */ 
		quotTitle[9] = QString::number(pp->HighestPrice,'f',scale);		/* ��߼� */ 
		quotTitle[10] = QString::number(pp->LowestPrice,'f',scale);		/* ��ͼ� */ 
        quotTitle[11] = QString::number(pp->Volume);					/* �ɽ��� */
        quotTitle[12] = QString::number(pp->OpenInterest);							/* �ֲ��� */
        quotTitle[13] = QString::number(pp->Turnover/10000,'f',1);						/* �ɽ��� */
        quotTitle[14] = QString::number(pp->UpperLimitPrice,'f',scale);		/* ��ͣ��� */
        quotTitle[15] = QString::number(pp->LowerLimitPrice,'f',scale);			/* ��ͣ��� */
        quotTitle[16] = QString::number(pp->LastPrice,'f',scale);		/* ����� */
        quotTitle[17] = QString::number(pp->PreSettlementPrice,'f',scale);		/* ������ */
        quotTitle[18] = QString::number(pp->PreClosePrice,'f',scale);			/* �����̼� */
		//quotTitle[18] = QString::number(pp->PreOI);						/* ��ֲ��� */ 
        quotTitle[19] = QString::fromLocal8Bit(pp->UpdateTime);			/* ����ʱ�� */
		quotTitle[20] = convertExchangeID(sbInstr->ExchangeID);			/* �������� */ 
		int afg = Qt::AlignLeft|Qt::AlignVCenter;
		for(int i = 0; i < 20; ++i) {
			int c1 = pvw * wrats[i]/100 - vscl;
			int c2 = pvw * wrats[i+1]/100 - vscl;
			if(c2 < 0 || c1 > width())
				continue;
			p->setPen(QColor(203, 203, 203));
			p->drawLine(c2, linPix, c2, linPix + LINPIX);
			//p->setPen(QColor(47,135,214));
			p->setPen(QColor(0, 0, 0));
			afg = Qt::AlignLeft|Qt::AlignVCenter;
			if(i>1 && i<19)
				afg = Qt::AlignRight|Qt::AlignVCenter;
            QRect vR = QRect(c1 + 2, linPix + 1, c2 - c1 - 4, LINPIX - 2);
			if(i==5)
			{
				/*if(vR.contains(QPoint(moveWPixs,moveHPixs)))
				{
					tw->showTradeW(sbInstr);
				}*/
				p->fillRect(vR, QColor(13,150,214));
				p->setPen(QColor(255, 255, 255));
			}
			if(i==6)
			{
				/*if(vR.contains(QPoint(moveWPixs,moveHPixs)))
				{
					tw->showTradeW(sbInstr);
				}*/
				p->fillRect(vR, QColor(210,54,54));
				p->setPen(QColor(255, 255, 255));
			}
			if(i==4)
			{
				/*if(vR.contains(QPoint(moveWPixs,moveHPixs)))
				{
					tw->showTradeW(sbInstr);
				}*/
				p->setPen(QColor(13, 150, 214));
			}
			if(i==7)
			{
				/*if(vR.contains(QPoint(moveWPixs,moveHPixs)))
				{
					tw->showTradeW(sbInstr);
				}*/
				p->setPen(QColor(210, 54, 54));
			}
			if ( i == 3 ) {
				if ( zf > 0.00 ) {
					p->setPen(QColor(210, 54, 54));
				} else if ( zf < 0.00 ) {
					p->setPen(QColor(87, 167, 56));
				}
			}
			//if(vR.contains(QPoint(x,y)))
			//{
			//	p->fillRect(vR, QColor(237,240,111));
			//	p->setPen(QColor(0, 0, 0));
			//}
			p->drawText(vR, afg, quotTitle[i]);
		}
		p->drawText(QRect(pvw * wrats[20] / 100 - vscl+2, linPix, pvw * (100 - wrats[20]) / 100-4, LINPIX), afg, quotTitle[20]);
	}
	if(pvh < height()-setsRect.height())
	{
		int pscl = height()-setsRect.height()-pvh;
		vsBarR = QRect(0,pvh,width(),pscl);
		p->fillRect(vsBarR,QColor(119, 119, 119));
		vsBarRt = QRect(v_sc,pvh+1,vbw,pscl-2);
		p->fillRect(vsBarRt,QColor(238, 238, 238));
	}
	if(mapTemp->count() > lines)
	{
		int rHight = pvh-LINPIX;
		hsBarR = QRect(width_,marTop+LINPIX,16,rHight);
		p->fillRect(hsBarR,QColor(119, 119, 119));
		int hrtHt = marTop + rHight*wrows/mapTemp->count();
		int hrtH = rHight*lines/mapTemp->count();
		hrtH = hrtH <LINPIX?LINPIX:hrtH;
		int hy = marTop+LINPIX+h_sc;
		if(hy+hrtH > pvh)
			hy = pvh-hrtH;
		hsBarRt = QRect(width_+1,hy,14,hrtH);
		p->fillRect(hsBarRt,QColor(238, 238, 238));
	}
}

// ����ƶ�ʱ���¼�����
void PriceView::mouseMoveEvent(QMouseEvent *event) {
	int marTop = setsRect.height();
	int pvw = width();
	pvw = pvw<DEFAULT_W?DEFAULT_W:pvw; // ��С���1000
	int pvh = height();
	pvh = pvw > width()?pvh-16:pvh; // �к����������ʱ��ͼ��߶ȼ�16
	int lines = (pvh-marTop)/LINPIX; // һҳ�������
	if(setsRect.contains(event->pos())) // TODO �ĳɸ�����ѡ��Լ
	{
		setCursor(QCursor(Qt::PointingHandCursor)); 
	}
	else
	{
		setCursor(QCursor(Qt::ArrowCursor)); 
	}
	if(moveWPixs < width() && width() < DEFAULT_W)
	{
		v_sc += event->pos().x()-moveWPixs;
		moveWPixs = event->pos().x();
		v_sc = v_sc<0?0:v_sc;
		v_sc = v_sc+vsBarRt.width()>width()?width()-vsBarRt.width():v_sc;
	}
	if(moveHPixs < height())
	{
		h_sc += event->pos().y()-moveHPixs;
		moveHPixs = event->pos().y();
		int dataHeight = height()-marTop-setsRect.height()-LINPIX;
		h_sc = h_sc>dataHeight?dataHeight:h_sc;
		h_sc = h_sc<0?0:h_sc;
		wrows = (mapTemp->count()-lines)*h_sc/(pvh-marTop-hsBarRt.height());
	}
	update();
}

// ��¼��갴��ʱ��״̬
void PriceView::mousePressEvent(QMouseEvent * event)
{
	if (event->button() != Qt::LeftButton) {
		return;
	}
	if(vsBarRt.contains(event->pos()))
		moveWPixs = event->x();
	if(hsBarRt.contains(event->pos()))
		moveHPixs = event->y();
	update();
}

// ��¼����ɿ�ʱ��״̬
void PriceView::mouseReleaseEvent(QMouseEvent *event) {
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
	//dis_y = (dis_y/70)*70;
	//dis_y = dis_y<100?dis_y:70;
	int x = event->pos().x();
	int y = event->pos().y();
	clickLine = y / LINPIX;
	if (event->button() != Qt::LeftButton) {
		return;
	}
	moveWPixs = 9999;
	moveHPixs = 9999;
	//if(y < dis_y)
	//{
	//	curPi = priInstrMap[QString::fromLocal8Bit("��ѡ��Լ")];
	//	mapTemp =  &tw->insMap_zx;
	//	selectInstr->minMap = mapTemp;
	//	selectInstr->curInstr = NULL;
	//	wrows = 0;
	//}
	if(priRect->privateSet.contains(x,y))
	{
		curPi = priRect;
		loadDBinfo();
		mapTemp = &tw->insMap_zx;
		selectInstr->minMap =  &tw->insMap;
	}
	else if(zlsRect->privateSet.contains(x,y))
	{
		curPi = zlsRect;
		mapTemp = &tw->insMap_zl;
		selectInstr->minMap = mapTemp;
	}
	update();
}

// ��������			
void PriceView::loadDBinfo()
{
	for(int i=0;i<1000;i++)
	{
		if(cview->pi.InstrumentID[i][0] == 0)
			continue;
		if(tw->insMap_zx[QString::fromLocal8Bit(cview->pi.InstrumentID[i])])
			continue;
		tw->insMap_zx[QString::fromLocal8Bit(cview->pi.InstrumentID[i])] = tw->insMap[QString::fromLocal8Bit(cview->pi.InstrumentID[i])];
	}
}

void PriceView::getInstr()
{
	this->setFocus(Qt::OtherFocusReason);
	int rows = (height()-24)/LINPIX-1; // һҳ������
	int index = 0;
	// ��¼����϶��Ŀ�����
	moveWPixs = 9999;
	// ��¼����϶��ĸ߶����
	moveHPixs = 9999;
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
	if(mapTemp)
	{
        QMapIterator<QString, CThostFtdcInstrumentField *> i(*mapTemp);
		while (i.hasNext())
		{
            CThostFtdcInstrumentField *sbInstr = mapTemp->value(i.next().key());
			if(!sbInstr) continue;
			if(ci != sbInstr)
				index++;
			else
				break;
		}
	}
	if(index > rows/2)
	{
		wrows = index-rows/2;
	}
	else
	{
		wrows = 0;
	}
	if(curPi && curPi->name == QString::fromLocal8Bit("��ѡ��Լ"))
	{
		addToSelfMap();
	}
	update();
}

// �����������¼�
void PriceView::wheelEvent(QWheelEvent *event) {
	int marTop = setsRect.height();
	int pvw = width();
	pvw = pvw<DEFAULT_W?DEFAULT_W:pvw; // ��С���1000
	int pvh = height();
	pvh = pvw > width()?pvh-16:pvh; // �к����������ʱ��ͼ��߶ȼ�16
	int lines = (pvh-marTop)/LINPIX; // һҳ�������
	if (event->delta() > 0) {
		if(wrows > 0)
		{
			wrows --;
			clickLine ++;
		}
	} else if (event->delta() < 0) {
		if(wrows < mapTemp->count()-lines)
		{
			clickLine --;
			wrows++;
		}
	}
	if(mapTemp->count() > lines)
		h_sc = (height()-setsRect.height()-hsBarRt.height())*wrows/(mapTemp->count()-lines);
	update();
}

// ����˫���¼�
void PriceView::mouseDoubleClickEvent(QMouseEvent *event) {
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
	if(ci && tw->CSubmit)
	{
		tw->CSubmit->selectInstr->curInstr = ci;
		tw->CSubmit->initByInstr();
		tw->CSubmit->update();
	}
}

// ���µ�����
//void PriceView::showTradeW()
//{
//	if(selectInstr->curInstr)
//		tw->showTradeW(selectInstr->curInstr);
//}

// ��ӵ���ѡ��Լmap
void PriceView::addToSelfMap()
{
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
	if(!ci) return;
	tw->insMap_zx[QString::fromLocal8Bit(ci->InstrumentID)] = ci;
	if(pQuotApi && tw->insMap_dy[QString::fromLocal8Bit(ci->InstrumentID)] == NULL)
	{
        int count = 1;
        char **InstrumentID = new char*[count];
        InstrumentID[0] = ci->InstrumentID;
        pQuotApi->SubscribeMarketData(InstrumentID, count); // ��������
		tw->insMap_dy[QString::fromLocal8Bit(ci->InstrumentID)] = ci;
        delete[] InstrumentID;
	}
	for(int i=0;i<1000;i++)
	{
		if(::strcmp(cview->pi.InstrumentID[i],"") == 0)
		{
            strncpy(cview->pi.InstrumentID[i],ci->InstrumentID, sizeof(TThostFtdcInstrumentIDType));
			break;
		}
		else if(::strcmp(cview->pi.InstrumentID[i],ci->InstrumentID) == 0)
		{
			break;
		}
	}
//	writePrivates(cview->pi);
	update();
}

// ɾ����ѡ
void PriceView::delFrSelfMap()
{
    CThostFtdcInstrumentField * ci = selectInstr->curInstr;
	if(ci)
	{
		tw->insMap_zx.remove(QString::fromLocal8Bit(ci->InstrumentID));
		// ȡ����������
        int count = 1;
        char **InstrumentID = new char*[count];
        InstrumentID[0] = ci->InstrumentID;
        pQuotApi->UnSubscribeMarketData(InstrumentID, 1); // ȡ������
		tw->insMap_dy[QString::fromLocal8Bit(ci->InstrumentID)] = NULL;
		tw->insMap_dy.remove(QString::fromLocal8Bit(ci->InstrumentID));
        delete[] InstrumentID;
		for(int i=0;i<1000;i++)
		{
			if(::strcmp(cview->pi.InstrumentID[i],ci->InstrumentID) == 0)
			{
                ::memset(cview->pi.InstrumentID[i],0,sizeof(TThostFtdcInstrumentIDType));
//				writePrivates(cview->pi);
				break;
			}
		}
	}
	update();
}


// K��
void PriceView::showCurve()
{
//	if(selectInstr->curInstr)
//	{
//		CurveUnit * CUnit = new CurveUnit(selectInstr->curInstr);
//		CUnit->setGeometry(DW/2-215,DH/2-125,430,250);
//		CUnit->setActionm5();
//		CUnitList.append(CUnit);
//		CUnit->show();
//	}
}
