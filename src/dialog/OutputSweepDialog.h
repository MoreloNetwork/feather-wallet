// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_OUTPUTSWEEPDIALOG_H
#define FEATHER_OUTPUTSWEEPDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/rows/CoinsInfo.h"

namespace Ui {
    class OutputSweepDialog;
}

class OutputSweepDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit OutputSweepDialog(QWidget *parent, quint64 amount);
    ~OutputSweepDialog() override;

    QString address();
    bool churn() const;
    int outputs() const;
    int feeLevel() const;

private:
    QScopedPointer<Ui::OutputSweepDialog> ui;

    uint64_t m_amount;

    QString m_address;
    bool m_churn = false;
    int m_outputs = 1;
    int m_feeLevel = 0;
};


#endif //FEATHER_OUTPUTSWEEPDIALOG_H
