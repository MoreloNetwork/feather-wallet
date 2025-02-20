// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SubaddressAccountModel.h"
#include "SubaddressAccount.h"

#include <QFont>
#include "utils/Utils.h"

#include "libwalletqt/WalletManager.h"
#include "rows/AccountRow.h"

SubaddressAccountModel::SubaddressAccountModel(QObject *parent, SubaddressAccount *subaddressAccount)
    : QAbstractTableModel(parent)
    , m_subaddressAccount(subaddressAccount)
{
    connect(m_subaddressAccount, &SubaddressAccount::refreshStarted, this, &SubaddressAccountModel::startReset);
    connect(m_subaddressAccount, &SubaddressAccount::refreshFinished, this, &SubaddressAccountModel::endReset);
}

void SubaddressAccountModel::startReset(){
    beginResetModel();
}

void SubaddressAccountModel::endReset(){
    endResetModel();
}

int SubaddressAccountModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_subaddressAccount->count();
    }
}

int SubaddressAccountModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return Column::COUNT;
}

QVariant SubaddressAccountModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || static_cast<quint64>(index.row()) >= m_subaddressAccount->count())
        return {};

    QVariant result;

    bool found = m_subaddressAccount->getRow(index.row(), [this, &index, &result, &role](const AccountRow &row) {
        if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
            result = parseSubaddressAccountRow(row, index, role);
        }
        else if (role == Qt::FontRole) {
            if (index.column() == Column::Balance || index.column() == Column::UnlockedBalance) {
                result = Utils::getMonospaceFont();
            }
        }
        else if (role == Qt::TextAlignmentRole) {
            if (index.column() == Column::Balance || index.column() == Column::UnlockedBalance) {
                result = Qt::AlignRight;
            }
        }
    });

    if (!found) {
        qCritical("%s: internal error: invalid index %d", __FUNCTION__, index.row());
    }

    return result;
}

QVariant SubaddressAccountModel::parseSubaddressAccountRow(const AccountRow &row,
                                                           const QModelIndex &index, int role) const
{
    switch (index.column()) {
        case Number:
            if (role == Qt::UserRole) {
                return index.row();
            }
            return QString("#%1").arg(QString::number(index.row()));
        case Address:
            return row.getAddress();
        case Label:
            return row.getLabel();
        case Balance:
            if (role == Qt::UserRole) {
                return WalletManager::amountFromString(row.getBalance());
            }
            return row.getBalance();
        case UnlockedBalance:
            if (role == Qt::UserRole) {
                return WalletManager::amountFromString(row.getUnlockedBalance());
            }
            return row.getUnlockedBalance();
        default:
            return QVariant();
    }
}

QVariant SubaddressAccountModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch (section) {
            case Address:
                return QString("Address");
            case Label:
                return QString("Label");
            case Balance:
                return QString("Balance");
            case UnlockedBalance:
                return QString("Spendable balance");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool SubaddressAccountModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        const int row = index.row();

        switch (index.column()) {
            case Label:
                m_subaddressAccount->setLabel(row, value.toString());
                break;
            default:
                return false;
        }
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        return true;
    }
    return false;
}


Qt::ItemFlags SubaddressAccountModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() == Label)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractTableModel::flags(index);
}

AccountRow* SubaddressAccountModel::entryFromIndex(const QModelIndex &index) const {
    return m_subaddressAccount->row(index.row());
}

SubaddressAccountProxyModel::SubaddressAccountProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(Qt::UserRole);
}