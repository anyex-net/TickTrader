#include "tradewidget.h"
#include <QApplication>
#include <QTextCodec>
#include <QtCore/QFile>
#include <QTextStream>
#include "configWidget.h"
#include "changePassword.h"
#include <QDesktopServices>	
#include <QDesktopWidget>
#include "notice.h"
#include "tools/log.h"

TradeWidget * g_tw = nullptr;
// 设置界面
configWidget * cview = nullptr;
// 修改密码界面
changePassword *cpView = nullptr;
// 构建登录窗口
loginWin * loginW = nullptr;

#define DW QApplication::desktop()->screenGeometry().width()
#define DH QApplication::desktop()->screenGeometry().height()

void loadStyleSheet(QString qssName)//加入下面函数
{
	QFile data(qssName);
	QString qssFile;
	if(data.open(QFile::ReadOnly))
	{
		QTextStream styleIn(&data);
		qssFile = styleIn.readAll();
		data.close();
		qApp->setStyleSheet(qssFile);
	}
}

int main(int argc, char *argv[])
{
	QApplication::addLibraryPath("./plugins");

    QApplication a(argc, argv);

    int iLogIndex = 0;
    QFile file("./logIndex.txt");
    if(file.open(QIODevice::ReadWrite)){
        QByteArray byte = file.readAll();
        if(!byte.isEmpty())
            iLogIndex = byte.toInt();
        else
            file.write(QString("1").toLocal8Bit());
        file.close();
    }
#ifndef QT_DEBUG
    QStringList list;
    list << "QSslSocket";
    Log::instance().start(iLogIndex, list);
#endif

	QFont font;
//	font.setFamily(QString::fromLocal8Bit("微软雅黑"));
	font.setPointSize(9);
    qApp->setFont(font);
	cview = new configWidget(NULL);
	cpView = new changePassword(NULL);
	loginW = new loginWin();
    g_tw = new TradeWidget();
	loginW->setGeometry(DW/2-200,DH/2-200,400,400);
    loginW->show();

	return a.exec();
}
