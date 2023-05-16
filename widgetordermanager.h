#ifndef WIDGETORDERMANAGER_H
#define WIDGETORDERMANAGER_H

#include <QWidget>
#include <QStandardItemModel>
#include "ui_widgetordermanager.h"
#include "tradewidget.h"

class COrderManager : public QWidget
{
    Q_OBJECT
public:
    COrderManager(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~COrderManager();

public slots:
//    void    onCancleOrder();
//    void    onAddOrUpdateOrder();
//    void    onChkChangeShowOrder();

private:
    Ui::WidgetOrder ui;
    QStandardItemModel *m_model;
};

#endif // WIDGETORDERMANAGER_H
