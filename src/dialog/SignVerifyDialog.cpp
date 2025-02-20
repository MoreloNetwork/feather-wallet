// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SignVerifyDialog.h"
#include "ui_SignVerifyDialog.h"

#include <QMessageBox>

#include "utils/Utils.h"

SignVerifyDialog::SignVerifyDialog(Wallet *wallet, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::SignVerifyDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    connect(ui->btn_Sign, &QPushButton::clicked, this, &SignVerifyDialog::signMessage);
    connect(ui->btn_Verify, &QPushButton::clicked, this, &SignVerifyDialog::verifyMessage);
    connect(ui->btn_Copy, &QPushButton::clicked, this, &SignVerifyDialog::copyToClipboard);

    connect(ui->message, &QPlainTextEdit::textChanged, [this](){ui->btn_Copy->setVisible(false);});
    connect(ui->address, &QLineEdit::textEdited, [this](){ui->btn_Copy->setVisible(false);});
    connect(ui->signature, &QLineEdit::textEdited, [this](){ui->btn_Copy->setVisible(false);});

    ui->address->setText(m_wallet->address(0, 0));
    ui->address->setCursorPosition(0);

    if (m_wallet->isHwBacked()) {
        // We don't have the secret spend key to sign messages
        ui->btn_Sign->setEnabled(false);
        ui->btn_Sign->setToolTip("Message signing is not supported on this hardware device.");
    }

    ui->btn_Copy->setVisible(false);
}

void SignVerifyDialog::signMessage() {
    QString signature = m_wallet->signMessage(ui->message->toPlainText(), false, ui->address->text());

    if (signature.isEmpty()) {
        QMessageBox::information(this, "Information", m_wallet->errorString());
        return;
    }

    ui->signature->setText(signature);
    ui->btn_Copy->setVisible(true);
}

void SignVerifyDialog::verifyMessage() {
    bool verified = m_wallet->verifySignedMessage(ui->message->toPlainText(), ui->address->text(), ui->signature->text());

    if (verified) {
        Utils::showInfo(this, "Signature is valid");
    } else {
        Utils::showError(this, "Signature is not valid");
    }
}

void SignVerifyDialog::copyToClipboard() {
    QStringList sig;
    sig << ui->message->toPlainText() << ui->address->text() << ui->signature->text();
    Utils::copyToClipboard(sig.join("\n"));
}

SignVerifyDialog::~SignVerifyDialog() = default;