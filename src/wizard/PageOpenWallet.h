// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_OPENWALLET_H
#define FEATHER_OPENWALLET_H

#include <QLabel>
#include <QStandardItemModel>
#include <QWizardPage>

class WalletKeysFilesModel;
class WalletKeysFilesProxyModel;

namespace Ui {
    class PageOpenWallet;
}

class PageOpenWallet : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOpenWallet(WalletKeysFilesModel *wallets, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

signals:
    void openWallet(QString path);

private slots:
    void nextPage();

private:
    void updatePath();

    Ui::PageOpenWallet *ui;
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WalletKeysFilesProxyModel *m_keysProxy;
    QStandardItemModel *m_model;
    QString m_walletFile;
};

#endif //FEATHER_OPENWALLET_H
