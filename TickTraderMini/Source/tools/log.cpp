#include "log.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>

void Log::outputLogMessage(QtMsgType type,const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    ESLogMessage log{QDateTime::currentDateTime(), type, msg};
    QMetaObject::invokeMethod(&logInstance, "appendLog", Q_ARG(ESLogMessage, log));
}


Log::Log()
{
    this->moveToThread(this);
    qRegisterMetaType<ESLogMessage>("ESLogMessage");
}

Log::~Log()
{
    stop();
}

void Log::setFilter(const QStringList &filters)
{
    s_filter = filters;
}

void Log::addFilter(const QStringList &filters)
{
    s_filter.append(filters);
}

void Log::addFilter(const QString &filter)
{
    s_filter.append(filter);
}

QStringList Log::getFilters()
{
    return s_filter;
}

void Log::start(int index, const QStringList &filters)
{
    if(index < 1 || index > 10)
    {
        m_logIndex = 1;
    }
    else
    {
        m_logIndex = index;
    }
    // 改变缺省消息处理程序的输出
    qSetMessagePattern("%{time [yyyy-MM-dd hh:mm:ss.zzz ddd]} %{type} %{message}");
//                   #ifdef QT_DEBUG
//                       " File:%{file} Line:%{line} Function:%{function}"
//                   #endif

    QString path("./log/");
    if(!QDir().exists(path))
    {
        QDir().mkdir(path);
    }

    fileName = (path + "TickTraderMini" +  "_log_%1.log");
    logFile.setFileName(fileName.arg(m_logIndex));
    if(logFile.exists() && logFile.size() >= FILE_LEN_LIMIT)
    {
        m_logIndex++;
        if(m_logIndex > 10)
        {
            m_logIndex %= 10;
            if(m_logIndex == 0)
            {
                m_logIndex = 1;
            }
        }
        logFile.setFileName(fileName.arg(m_logIndex));
        logFile.resize(0);
    }
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    qInstallMsgHandler(outputLogMessage);
#else
    qInstallMessageHandler(&Log::outputLogMessage);
#endif
    s_filter = filters;
    logFile.open(QIODevice::WriteOnly | QIODevice::Text| QIODevice::Append);
    QThread::start();
}

void Log::stop()
{
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    qInstallMsgHandler(0);
#else
    qInstallMessageHandler(nullptr);
#endif
    quit();
    wait();
    logFile.close();
    s_filter.clear();
}

int Log::fileNameIndex()
{
    return m_logIndex;
}

QString Log::filePath()
{
    return logFile.fileName();
}

void Log::appendLog(Log::ESLogMessage log)
{
    QString text;
    switch (log.type) {
    case QtDebugMsg:
#ifndef QT_DEBUG
        return;
#endif
        text = "Debug";
        break;
    case QtWarningMsg:
        text = "Warning";
        break;
    case QtCriticalMsg:
        text = "Critical";
        break;
    case QtFatalMsg:
        text = "Fatal";
        break;
    case QtInfoMsg:
        text = "Info";
        break;
    }

    if(log.type == QtInfoMsg || log.type == QtWarningMsg){
        for(QString s:s_filter) {
            if(log.strLog.startsWith(s,Qt::CaseInsensitive)) {
                return;
            }
        }
    }
    static QtMsgType preType;
    static QString preMsg;
    if(preType == log.type && preMsg == log.strLog){
        return;
    }
    preType = log.type;
    preMsg = log.strLog;

    text = QString("[%1] %2 %3").arg(log.dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz ddd")).arg(text).arg(log.strLog);

    if(logFile.size()+text.size() >= FILE_LEN_LIMIT)
    {
        m_logIndex++;
        if(m_logIndex > 10)
        {
            m_logIndex %= 10;
            if(m_logIndex == 0)
            {
                m_logIndex = 1;
            }
        }
        logFile.close();
        logFile.setFileName(fileName.arg(m_logIndex));
        logFile.resize(0);
    }
    if(logFile.isOpen() || logFile.open(QIODevice::WriteOnly | QIODevice::Text| QIODevice::Append))
    {
        QTextStream textStream(&logFile);
        textStream.setCodec(QTextCodec::codecForName("utf-8"));
        textStream<<text<<endl;
        logFile.flush();
    }
}
