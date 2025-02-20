// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_CALCWIDGET_H
#define FEATHER_CALCWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QTimer>

namespace Ui {
    class CalcWidget;
}

class CalcWidget : public QWidget
{
Q_OBJECT

public:
    explicit CalcWidget(QWidget *parent = nullptr);
    ~CalcWidget() override;

public slots:
    void skinChanged();

private slots:
    void initComboBox();
    void showCalcConfigureDialog();
    void onPricesReceived();

private:
    void convert(bool reverse);
    void setupComboBox(QComboBox *comboBox, const QStringList &crypto, const QStringList &fiat);
    void updateStatus();

    QScopedPointer<Ui::CalcWidget> ui;
    bool m_comboBoxInit = false;
    QTimer m_statusTimer;
};

#endif // FEATHER_CALCWIDGET_H
