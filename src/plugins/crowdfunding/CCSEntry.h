// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_CCSENTRY_H
#define FEATHER_CCSENTRY_H

#include <QString>

struct CCSEntry {
    CCSEntry()= default;;

    QString title;
    QString date;
    QString address;
    QString author;
    QString state;
    QString url;
    QString organizer;
    QString currency;
    double target_amount = 0;
    double raised_amount = 0;
    double percentage_funded = 0;
    int contributions = 0;
};

#endif //FEATHER_CCSENTRY_H
