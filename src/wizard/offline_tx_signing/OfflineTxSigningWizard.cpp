// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "OfflineTxSigningWizard.h"

#include "PageOTS_ExportOutputs.h"
#include "PageOTS_ImportKeyImages.h"
#include "PageOTS_ExportUnsignedTx.h"
#include "PageOTS_ExportSignedTx.h"

#include "PageOTS_ImportOffline.h"
#include "PageOTS_ExportKeyImages.h"
#include "PageOTS_ImportUnsignedTx.h"
#include "PageOTS_SignTx.h"
#include "PageOTS_ImportSignedTx.h"

#include <QApplication>
#include <QScreen>
#include <QPushButton>

#include "utils/config.h"

OfflineTxSigningWizard::OfflineTxSigningWizard(QWidget *parent, Wallet *wallet, PendingTransaction *tx)
    : QWizard(parent)
    , m_wallet(wallet)
{
    m_wizardFields.scanWidget = new QrCodeScanWidget(nullptr);

    // View-only
    setPage(Page_ExportOutputs, new PageOTS_ExportOutputs(this, m_wallet));
    setPage(Page_ImportKeyImages, new PageOTS_ImportKeyImages(this, m_wallet, &m_wizardFields));
    setPage(Page_ExportUnsignedTx, new PageOTS_ExportUnsignedTx(this, m_wallet, tx));
    setPage(Page_ImportSignedTx, new PageOTS_ImportSignedTx(this, m_wallet, &m_wizardFields));

    // Offline
    setPage(Page_ImportOffline, new PageOTS_ImportOffline(this, m_wallet, &m_wizardFields));
    setPage(Page_ExportKeyImages, new PageOTS_ExportKeyImages(this, m_wallet, &m_wizardFields));
    setPage(Page_ImportUnsignedTx, new PageOTS_ImportUnsignedTx(this, m_wallet, &m_wizardFields));
    setPage(Page_SignTx, new PageOTS_SignTx(this));
    setPage(Page_ExportSignedTx, new PageOTS_ExportSignedTx(this, m_wallet, &m_wizardFields));
    
    if (tx) {
        setStartId(Page_ExportUnsignedTx);
    } else {
        setStartId(m_wallet->viewOnly() ? Page_ExportOutputs : Page_ImportOffline);
    }
    
    this->setWindowTitle("Offline transaction signing");

    QList<QWizard::WizardButton> layout;
    layout << QWizard::CancelButton;
    layout << QWizard::Stretch;
    layout << QWizard::BackButton;
    layout << QWizard::NextButton;
    layout << QWizard::FinishButton;
    layout << QWizard::CommitButton;
    this->setButtonLayout(layout);

    // setOption(QWizard::HaveCustomButton1, true);
    setOption(QWizard::NoBackButtonOnStartPage);
    setWizardStyle(WizardStyle::ModernStyle);
    
    bool geo = this->restoreGeometry(QByteArray::fromBase64(conf()->get(Config::geometryOTSWizard).toByteArray()));
    if (!geo) {
        QScreen *currentScreen = QApplication::screenAt(this->geometry().center());
        if (!currentScreen) {
            currentScreen = QApplication::primaryScreen();
        }
        int availableHeight = currentScreen->availableGeometry().height() - 100;
        
        this->resize(availableHeight, availableHeight);
    }

    // Anti-glare
    // QFile f(":qdarkstyle/style.qss");
    // if (!f.exists()) {
    //     printf("Unable to set stylesheet, file not found\n");
    //     f.close();
    // } else {
    //     f.open(QFile::ReadOnly | QFile::Text);
    //     QTextStream ts(&f);
    //     QString qdarkstyle = ts.readAll();
    //     this->setStyleSheet(qdarkstyle);
    // }
}

bool OfflineTxSigningWizard::readyToCommit() {
    return m_wizardFields.readyToCommit;
}

bool OfflineTxSigningWizard::readyToSign() {
    return m_wizardFields.readyToSign;
}

UnsignedTransaction* OfflineTxSigningWizard::unsignedTransaction() {
    return m_wizardFields.utx;
}

PendingTransaction* OfflineTxSigningWizard::signedTx() {
    return m_wizardFields.tx;
}

OfflineTxSigningWizard::~OfflineTxSigningWizard() {
    conf()->set(Config::geometryOTSWizard, QString(saveGeometry().toBase64()));
}