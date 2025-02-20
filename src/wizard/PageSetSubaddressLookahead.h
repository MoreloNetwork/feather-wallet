// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_PAGESETSUBADDRESSLOOKAHEAD_H
#define FEATHER_PAGESETSUBADDRESSLOOKAHEAD_H

#include <QWizardPage>

class WizardFields;

namespace Ui {
    class PageSetSubaddressLookahead;
}

class PageSetSubaddressLookahead : public QWizardPage
{
Q_OBJECT

public:
    explicit  PageSetSubaddressLookahead(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private:
    Ui::PageSetSubaddressLookahead *ui;

    WizardFields *m_fields;
};


#endif //FEATHER_PAGESETSUBADDRESSLOOKAHEAD_H
