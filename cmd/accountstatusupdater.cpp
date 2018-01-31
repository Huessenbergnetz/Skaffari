/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "accountstatusupdater.h"
#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
#include <QDateTime>

#define PAM_ACCT_EXPIRED 1
#define PAM_NEW_AUTHTOK_REQD 2

AccountStatusUpdater::AccountStatusUpdater(const QString &confFile, bool quiet) :
    ConfigFile(confFile, false, false, quiet)
{

}


int AccountStatusUpdater::exec() const
{
    printMessage(tr("Start status check for all user accounts."));

    int retVal = checkConfigFile();
    if (retVal > 0) {
        return retVal;
    }

    QSettings s(configFileName(), QSettings::IniFormat);
    s.beginGroup(QStringLiteral("Database"));
    const QString dbhost = s.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    const QString dbname = s.value(QStringLiteral("name")).toString();
    const QString dbpass = s.value(QStringLiteral("password")).toString();
    const QString dbtype = s.value(QStringLiteral("type"), QStringLiteral("QMYSQL")).toString();
    const QString dbuser = s.value(QStringLiteral("user")).toString();
    const quint16 dbport = s.value(QStringLiteral("port"), 3306).value<quint16>();
    s.endGroup();

    Database db(dbtype, dbhost, dbport, dbname, dbuser, dbpass);
    printStatus(tr("Establishing database connection"));
    if (!db.open()) {
        printFailed();
        return dbError(db.lastDbError());
    } else {
        printDone();
    }


    printStatus(tr("Fetching accounts"));
    QSqlQuery q(db.getDb());
    if (!q.exec(QStringLiteral("SELECT id, valid_until, pwd_expire, status FROM accountuser WHERE domain_id > 0"))) {
        printFailed();
        return dbError(q.lastError());
    } else {
        //: %1 will be the number of found accounts
        printDone(tr("Found %1").arg(q.size()));
    }

    while (q.next()) {
        const quint32 id = q.value(0).value<quint32>();
        printStatus(tr("Checking account ID %1").arg(id));
        const QDateTime now = QDateTime::currentDateTimeUtc();
        int newStatus = 0;
        QDateTime validUntil = q.value(1).toDateTime();
        validUntil.setTimeSpec(Qt::UTC);
        QDateTime pwdExpire = q.value(2).toDateTime();
        pwdExpire.setTimeSpec(Qt::UTC);
        const int oldStatus = q.value(3).toInt();

        if (validUntil < now) {
            newStatus |= PAM_ACCT_EXPIRED;
        }

        if (pwdExpire < now) {
            newStatus |= PAM_NEW_AUTHTOK_REQD;
        }

        if (oldStatus != newStatus) {
            QSqlQuery updateQuery(db.getDb());
            updateQuery.prepare(QStringLiteral("UPDATE accountuser SET status = :status WHERE id = :id"));
            updateQuery.bindValue(QStringLiteral(":status"), newStatus);
            updateQuery.bindValue(QStringLiteral(":id"), id);

            if (!updateQuery.exec()) {
                printFailed();
                return dbError(updateQuery.lastError());
            }
        }

        printDone();

        if ((newStatus & PAM_ACCT_EXPIRED) == PAM_ACCT_EXPIRED) {
            printMessage(tr("Account ID %1 has been expired at %2.").arg(QString::number(id), validUntil.toLocalTime().toString()));
        }

        if ((newStatus & PAM_NEW_AUTHTOK_REQD) == PAM_NEW_AUTHTOK_REQD) {
            printMessage(tr("Password for account ID %1 has been expired at %2.").arg(QString::number(id), pwdExpire.toLocalTime().toString()));
        }
    }

    printSuccess(tr("Finished updating status for %n account(s).", "", q.size()));

    return 0;
}
