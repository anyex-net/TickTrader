#ifndef LOG_H
#define LOG_H

#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QFile>

#define logInstance Log::instance()

class Log : public QThread
{
    Q_OBJECT
public:
    static Log &instance()
    {
        static Log m_Instance;
        return m_Instance;
    }

    struct ESLogMessage
    {
        QDateTime dateTime;
        QtMsgType type;
        QString strLog;
    };

    void setFilter(const QStringList &filters);
    void addFilter(const QStringList &filters);
    void addFilter(const QString &filter);
    QStringList getFilters();

    void start(int index,const QStringList &filters = QStringList());
    void stop();
    int fileNameIndex();
    QString filePath();
public slots:
    void appendLog(ESLogMessage log);

private:
    Log();
    ~Log();

    QFile logFile;
    int m_logIndex = 1;
    QString fileName;
    QStringList s_filter;
    const int FILE_LEN_LIMIT = 10 * 1024 * 1024 - 1024;

    static void outputLogMessage(QtMsgType type,const QMessageLogContext &context, const QString &msg);
};

#endif // LOG_H
