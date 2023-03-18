#include "changePassword.h"
#include "coolsubmit.h"
#include "tradewidget.h"

// �޸��������
extern changePassword *cpView;
extern TradeWidget *g_tw;
// ������¼����
extern loginWin * loginW;
extern quint32 CreateNewRequestID();

changePassword::changePassword(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
	ui.setupUi(this);
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/image/images/pass.png"), QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(icon);
	// �ر�ʱ�Զ��ͷ�
	setAttribute(Qt::WA_DeleteOnClose, true);
	connect(ui.finishButton, SIGNAL(clicked()),this, SLOT(changePasswordAction()));
	connect(ui.cancelButton, SIGNAL(clicked()),this, SLOT(cancelAction()));
}

changePassword::~changePassword()
{
	cpView = NULL;
}

void changePassword::changePasswordAction() {
	// ��������
	strncpy(oldPassword, ui.oldPasswordEdit->text().toLatin1().data(),sizeof(oldPassword));
	strncpy(newPassword, ui.newPasswordEdit->text().toLatin1().data(),sizeof(newPassword));
	strncpy(verifyPassword, ui.verifyPasswordEdit->text().toLatin1().data(),sizeof(verifyPassword));
	if (ui.oldPasswordEdit->text().isEmpty() || ui.newPasswordEdit->text().isEmpty() || ui.verifyPasswordEdit->text().isEmpty()) {
		//QMessageBox::about(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����������Ϊ�գ�"));
		Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("KDJ����������Ϊ�գ�"), false, QString::fromLocal8Bit("��ʾ"), NULL, 0);
		nt.exec();
		return;
	}

	if (!strcmp(oldPassword, newPassword)) {
		//QMessageBox::about(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("������ԭ������ͬ��"));
		Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("������ԭ������ͬ��"), false, QString::fromLocal8Bit("��ʾ"), NULL, 0);
		nt.exec();
		return;
	} else if (strcmp(newPassword, verifyPassword)) {
		//QMessageBox::about(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("���μ���������벻һ�£�"));
		Notice nt(Notice::NOTICE_TYPE_NOTIFICATION, QString::fromLocal8Bit("���μ���������벻һ�£�"), false, QString::fromLocal8Bit("��ʾ"), NULL, 0);
		nt.exec();
		return;
	} else {
		changePasswordEvent();
		this->close();
	}
}

void changePassword::cancelAction() {
	// ȡ����ť �رմ���
	this->close();
}

// �������������¼�
void changePassword::changePasswordEvent() {
	TradeInfo & ti = tradeInfoLst[loginW->userName];
    CThostFtdcUserPasswordUpdateField pChangeOrder;
    ::memset(&pChangeOrder, 0,sizeof(CThostFtdcUserPasswordUpdateField));
	int nRequestID = CreateNewRequestID();
    strncpy(pChangeOrder.BrokerID,loginW->m_users.BrokerID, sizeof(pChangeOrder.BrokerID));
    strncpy(pChangeOrder.UserID,loginW->userName, sizeof(pChangeOrder.UserID));
    strncpy(pChangeOrder.OldPassword, oldPassword, sizeof(pChangeOrder.OldPassword));
    strncpy(pChangeOrder.NewPassword, newPassword,sizeof(pChangeOrder.NewPassword));
    ti.api->ReqUserPasswordUpdate(&pChangeOrder, nRequestID);
	//if(ti.api->ChangePasswordReq(&pChangeOrder, nRequestID) == 0)
	//{
	//	QMessageBox::about(NULL,QString::fromLocal8Bit("��ʾ"),QString::fromLocal8Bit("�ѷ��������޸�����"));
	//}
	//else
	//{
	//	QMessageBox::about(NULL,QString::fromLocal8Bit("��ʾ"),QString::fromLocal8Bit("�����޸�����ʧ�ܣ�"));
	//}
}
