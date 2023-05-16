#ifndef WIDGETTRADEMANAGER_H
#define WIDGETTRADEMANAGER_H

#include <QWidget>
#include <QStandardItemModel>
#include "ui_widgettrademanager.h"
#include "tradewidget.h"

class CTradeManager : public QWidget
{
    Q_OBJECT
public:
    CTradeManager(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~CTradeManager();

signals:
    void    addTrade(CThostFtdcTradeField *);

public slots:
    void    onAddTrade(CThostFtdcTradeField *info);

private:
    Ui::WidgetTrade ui;
    QStandardItemModel *m_model;
};
#endif // WIDGETTRADEMANAGER_H
