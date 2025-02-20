// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_PAGESETSEEDPASSPHRASE_H
#define FEATHER_PAGESETSEEDPASSPHRASE_H

#include <QWizardPage>

class WizardFields;

namespace Ui {
    class PageSetSeedPassphrase;
}

class PageSetSeedPassphrase : public QWizardPage
{
Q_OBJECT

public:
    explicit  PageSetSeedPassphrase(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    Ui::PageSetSeedPassphrase *ui;

    WizardFields *m_fields;
};


#endif //FEATHER_PAGESETSEEDPASSPHRASE_H
