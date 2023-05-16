#ifndef COOLSUBMIT_H
#define COOLSUBMIT_H
#include <QWidget>
#include <QtGui/QPainter>
#include <QLineEdit>
#include <QComboBox>
#include <QWheelEvent>
#include <QMouseEvent>
#include "tradewidget.h"
#include "InstrManage.h"

#define LIMITORDER "�޼۵�"
#define STOPMARKETORDER "ֹ���м۵�"
#define ONETICK "+1TICK�޼۵�"
#define TWOTICK "+2TICK�޼۵�"
#define THREETICK "+3TICK�޼۵�"
#define STOPLIMITORDER "ֹ���޼۵�"

typedef struct
{
    TThostFtdcInstrumentIDType InsID;	/* ��Լ�� */
    TThostFtdcInvestorIDType invID; // Ͷ���ߺ�
	double price; // �۸�
	int account;  // ����
	char bs;      // ��������
}StopParam;

class InstrManage;
class CoolSubmit: public QWidget
{
	Q_OBJECT
public:
    CoolSubmit(QString Instr, CThostFtdcDepthMarketDataField * t, TradeWidget * tWidget, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~CoolSubmit();
	// ��������
	void wheelup();
	// ��������
	void wheeldown();
	// ���״���
	TradeWidget * tw;
	// ����۸�
    CThostFtdcDepthMarketDataField * sQuot;
	// ��������
    QMap<QString, CThostFtdcOrderField *> orderMap;
	// ��ǰ�ʽ��˻�
	QString currentAccout;
	// ��¼��ʾ��Ϣ
	QString messageInfo;
	// �����趨����
	QLineEdit * numL;
	// ���ö���ģʽ
	// QComboBox * orderModeC; 
	// ѡ����Ʒ��
    InstrManage * selectInstr;
	// ���������־ 0 δ���� 1������ 2 �Ѻ�Լ
	int continueFlag;
	// �۸񾫶�
	int pScale;
	// ��С�䶯��λ
	double minMove;
	// ����λ����
	QMap<int, int> sellOrder; 
	// ���λ����
	QMap<int, int> buyOrder; 
	// ����Ч����
	int clkborderWidth;
	// ��������
    TThostFtdcDirectionType methodFlag;
	// ����wheelSteps
	void setWheelSteps(int steps){wheelSteps=steps;};
	// ɾ������
	void dropOrder(char bos, double price);
	// �޸Ķ���
	void updateOrder(char bos, double price, double price2);
	// �½�����
	void insertOrder(char bos, double price, int fpoints = 0, int qty = 0);
	// �½�ֹ�𶩵�
	void insertLOrder(char bos, double price, int fpoints = 0, int qty = 0);
	// ɾ�����򶩵�
	void dropAll(char bos);

    void getOrderOffsetFlag(CThostFtdcInputOrderField *pInputOrder, TThostFtdcVolumeType &OpenQty, TThostFtdcVolumeType &CloseQty, TThostFtdcVolumeType &CloseTodayQty);
private:
	// һҳ������
	int linNum;
	// ��¼���ֵ�λ��
	int wheelSteps;
	// ������λ��
	int maxWheelSteps;
	// ��С����λ��
	int minWheelSteps;
	// ��¼����϶��Ŀ�����
	int moveWPixs;
	// ��¼����϶��ĸ߶����
	int moveHPixs;
	// ÿ�����ݵ����ظ߶�
	int linPix;
	// �ּ�����
	QRect curPriceR;
	// ���������
	QRect BidPriceR;
	// ����������
	QRect AskPriceR;

	// Ʒ��ѡ������
	QRect selectInstrR;
	// �ύ��������
	QRect submitCountsR;
	// �ύ��������������
	QRect submitSR;
	// �ύ������������
	QRect submitBR;
	//// ӯ����ʾ����
	//QRect ProfitR;
	// ��������
	QRect scrollR;
	// ������bar
	QRect sBarR;
	// ����1����
	QRect rcount1;
	// ����2����
	QRect rcount2;
	// ����3����
	QRect rcount3;
	// ����5����
	QRect rcount5;
	// ����10����
	QRect rcount10;
	// ����20����
	QRect rcount20;
	// ������������
	QRect cAllBuy;
	// ��������������
	QRect cAllSell;
	// ����������
	QRect cAll;
	// �Ҳ��Ƿ�����
	QRect disperRect;
	// �Ҳ�ֲ�����
	QRect qtyRect;
	// �Ҳ�ӯ������
	QRect bsRect;
	// �Ҽ��˵�
    QMenu * ractions;
	// OCO price
	double price1;
	// OCO price
	double price2;
	// OCO bos
	char bos1;
	// OCO ��������
	char meFlag1;
	// OCO price
	double price3;
	// OCO price
	double price4;
	// OCO bos
	char bos2;
	// OCO ��������
	char meFlag2;
	// �Ҽ�λ��x
	int curposx;
	// �Ҽ�λ��y
	int curposy;
protected:
	// ocoУ��
	void checkOCO(double curp, char bos, double p1, double p2);
	// ����OCO����
	void initOcoOrder();
	// ����׷��ֹ��
    void initZZOrder(TThostFtdcDirectionType bs);
	// �����������¼�
	void wheelEvent(QWheelEvent * event);
	// ������repaint��update�󴥷�
	void paintEvent(QPaintEvent * event);
	// ��¼��갴��ʱ��״̬
	void mousePressEvent(QMouseEvent * event);
	// ����˫���¼�
	void mouseDoubleClickEvent(QMouseEvent * event);
	// ��¼����ɿ�ʱ��״̬
	void mouseReleaseEvent(QMouseEvent * event);
	// ����ƶ�ʱ���¼�����
	void mouseMoveEvent(QMouseEvent * event);
	// �����¼�
	void keyPressEvent ( QKeyEvent * event );
	// �Ҽ��˵�
	void contextMenuEvent(QContextMenuEvent* e);
public slots:
	// ��ʱ����
	void onTimerOut();
	// ������Ӧ
    void rcancelAction();
	// �л���Լ
	void initByInstr();
};

#endif // COOLSUBMIT_H
