#include "dealView.h"
#include "coolsubmit.h"
#include "tradewidget.h"
#include "notice.h"

#define LINPIX 30

extern loginWin * loginW;
extern QString convertBsFlag(TThostFtdcDirectionType flag);
extern QString convertOcFlag(TThostFtdcDirectionType flag);
extern QString convertPriceType(TThostFtdcDirectionType flag);
extern QString convertConditionMethod(TThostFtdcDirectionType flag);
extern QString convertOrderStatus(TThostFtdcDirectionType flag);
extern QString convertExchangeID(TThostFtdcExchangeIDType ID);
extern quint32 CreateNewRequestID();

dealView::dealView(TradeWidget * tWidget, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
	tw = tWidget;
	QIcon icon;
    icon.addFile(QString::fromUtf8(":/image/images/deal@32px.png"), QSize(), QIcon::Normal, QIcon::Off);
    this->setWindowIcon(icon);
	setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowTitle(QString::fromLocal8Bit("�ɽ���"));
	c_trader = NULL;
	wrows = 0;
	clickLine = 0;
	setMouseTracking(true);
	// �ر�ʱ�Զ��ͷ�
	setAttribute(Qt::WA_DeleteOnClose, false);
}

dealView::~dealView()
{

}

void dealView::paintEvent(QPaintEvent * event)
{
	QPainter painter(this); 
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(QColor(34, 34, 34)); 
	// ���ƶ������
	paintOrderTable(&painter);
}

// ���Ʊ������
void dealView::paintOrderTable(QPainter *p)
{
	int linPix = 0;
	QPen pen(QColor(240,210,6));
	pen.setWidth(2);
	p->setPen(pen);
	QRect tableRect = QRect(0, 0,width(),height());
	p->fillRect(tableRect, QColor(225, 225, 225));
	TradeInfo * ti = &(tradeInfoLst[loginW->userName]);
	if(ti && ti->orderLst.count() == 0)
	{
		QFont def = p->font();
		QFont font;
		font.setFamily(QString::fromUtf8("΢���ź�"));
		font.setPointSize(30);
		p->setFont(font);
		p->setPen(QColor(34, 34, 34));
		p->drawText(tableRect,Qt::AlignCenter, QString::fromLocal8Bit("���޳ɽ�"));
		p->setFont(def);
		return;
	}
	linPix += LINPIX;
	p->setPen(QColor(189,189,189));
	p->drawLine(0,LINPIX,width(),LINPIX);
	int wspan[5] = {20,20,20,20,20};
	int wrats[5] = {0,0,0,0,0};
	for(int i=1;i<5;i++)
	{
		wrats[i]=wrats[i-1]+wspan[i-1];
	}
	QString otitle[5] = {QString::fromLocal8Bit("�ɽ�ʱ��") ,QString::fromLocal8Bit("��Ʒ"),
						QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�۸�")
						,QString::fromLocal8Bit("����")};//, QString::fromLocal8Bit("�ɽ���")};
	int afg = Qt::AlignLeft|Qt::AlignVCenter;
	QFont def = p->font();
	QFont font;
	font.setFamily(QString::fromUtf8("΢���ź�"));
	font.setBold(true);
	p->setFont(font);
	for(int c=0;c<4;c++)
	{
		int c1 = width()*wrats[c]/100;
		int c2 = width()*wrats[c+1]/100;
		p->setPen(QColor(189,189,189));
		p->drawLine(c2,linPix-LINPIX,c2,linPix);
		QPen tpen(QColor(34, 34, 34));
		tpen.setWidth(2);
		p->setPen(tpen);
		afg = Qt::AlignLeft|Qt::AlignVCenter;
		if(c>=3 && c<=4)
				afg = Qt::AlignRight|Qt::AlignVCenter;
		p->drawText(QRect(c1+2,linPix-LINPIX,c2-c1-4,LINPIX),afg, otitle[c]);
	}
	p->drawText(QRect(width()*wrats[4]/100+2,linPix-LINPIX,width()*(100-wrats[4])/100-4,LINPIX), afg, otitle[4]);
	p->setFont(def);
	bool jo = false; // ��ż����ʾ false�� trueż
	// ������ʱ������
    QMap<QString, CThostFtdcTradeField *> tLst;// ���˻���Ӧ�Ķ�����Ϣ KEY:����ʱ��
    QMapIterator<QString, CThostFtdcTradeField *> s(ti->tradeLst);
	int dis = 0; // �������ֶ���ʱ�䲻һ�������
	while (s.hasNext())
	{
        CThostFtdcTradeField * st = ti->tradeLst[s.next().key()];
		if(!st)
			continue;
        QString dt = QString(st->TradeTime).append(QString(st->TradeDate));
		if(!tLst.contains(dt))
		{
			tLst[dt] = st;
		}
		else
		{
			tLst[dt.append(QString::number(dis))] = st;
			dis++;
		}
	}
    QMapIterator<QString, CThostFtdcTradeField *> i(tLst);
	i.toBack();
	int igrows = wrows>0?wrows:0;
	while (i.hasPrevious() && linPix<height())
	{
        CThostFtdcTradeField * ste = tLst[i.previous().key()];
		if(!ste) continue;
		if(igrows > 0)
		{
			igrows --;
			continue;
		}
		linPix += LINPIX;
        CThostFtdcInstrumentField * sbInstr = tw->insMap[QString::fromLocal8Bit(ste->InstrumentID)];
		if(!sbInstr)
			continue;
		int scale = tw->getScale(ste->InstrumentID);
		if(jo)
		{
			p->fillRect(QRect(0,linPix-LINPIX,width(),LINPIX),QColor(220,220,220));
		}
		if(LINPIX+clickLine*LINPIX == linPix)
		{
			c_trader = ste;
			p->fillRect(QRect(0,linPix-LINPIX,width(),LINPIX),QColor(255,255,255));
		}
		jo = !jo;
		p->setPen(QColor(189, 189, 189));
		p->drawLine(0,linPix,width(),linPix);
        otitle[0] = QString::fromLocal8Bit(ste->TradeTime);  /* �ɽ�ʱ�� */
		otitle[1] = QString::fromLocal8Bit(sbInstr->InstrumentName);/* ��Լ���� */
        otitle[2] = convertBsFlag(ste->Direction).append(convertOcFlag(ste->OffsetFlag));/* ������־ *//* ��ƽ��־ */
		otitle[3] = QString::number(ste->Price,'f',scale);/* �۸� */ 
        otitle[4] = QString::number(ste->Volume);/* ���� */
//		otitle[5] = QString::fromLocal8Bit(ste->OrderID);  /* ������ */
		//otitle[6] = QString::fromLocal8Bit(ste->MatchID);  /* �ɽ��� */
		int afg = Qt::AlignLeft|Qt::AlignVCenter;
		for(int rc=0;rc<4;rc++)
		{
			int c1 = width()*wrats[rc]/100;
			int c2 = width()*wrats[rc+1]/100;
			p->setPen(QColor(203, 203, 203));
			p->drawLine(c2,linPix-LINPIX,c2,linPix);
			p->setPen(QColor(47,135,214));
			afg = Qt::AlignLeft|Qt::AlignVCenter;
			if(rc>=3 && rc<=4)
				afg = Qt::AlignRight|Qt::AlignVCenter;
			p->setPen(QColor(0, 0, 0));
			p->drawText(QRect(c1+2,linPix-LINPIX,c2-c1-4,LINPIX), afg, otitle[rc]);
		}
		p->drawText(QRect(width()*wrats[4]/100+2,linPix-LINPIX,width()*(100-wrats[4])/100-4,LINPIX), afg, otitle[4]);
	}
}

// ��¼����ɿ�ʱ��״̬
void dealView::mouseReleaseEvent(QMouseEvent * event)
{
	if(event->button() != Qt::LeftButton)
	{
		return;
	}
	int x = event->pos().x();
	int y = event->pos().y();
//	CThostFtdcInstrumentField * csi = NULL;
	if(y > LINPIX)
		clickLine = (y+LINPIX)/LINPIX-1;
	//if(c_trader)
	//{
	//	csi = tw->insMap[QString(c_trader->InstrumentID)];
	//}
	update();
}

// �����������¼�
void dealView::wheelEvent(QWheelEvent * event)
{
	if(event->delta() > 0)
	{
		if(wrows > 0)
		{
			clickLine++;
			wrows--;
		}
	}
	else if(event->delta() < 0)
	{
		clickLine--;
		wrows++;
	}
	update();
}
