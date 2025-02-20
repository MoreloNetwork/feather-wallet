// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_QRCODESCANDIALOG_H
#define FEATHER_QRCODESCANDIALOG_H

#include <QDialog>
#include <QScopedPointer>

namespace Ui {
    class QrCodeScanDialog;
}

class QrCodeScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QrCodeScanDialog(QWidget *parent, bool scan_ur = false);
    ~QrCodeScanDialog() override;

    QString decodedString();

private:
    QScopedPointer<Ui::QrCodeScanDialog> ui;
};


#endif //FEATHER_QRCODESCANDIALOG_H
