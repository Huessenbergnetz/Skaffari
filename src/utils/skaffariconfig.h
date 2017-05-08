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

#ifndef SKAFFARICONFIG_H
#define SKAFFARICONFIG_H

#include <QCryptographicHash>
#include <QAbstractSocket>
#include "../../common/password.h"
#include "../imap/skaffariimap.h"
#include "../objects/account.h"

class SkaffariConfig
{
public:
    SkaffariConfig();

    static void load(const QVariantMap &accounts, const QVariantMap &admins, const QVariantMap &defaults, const QVariantMap &imap);

    static bool isInitialized();

    static Password::Method accPwMethod();
    static Password::Algorithm accPwAlgorithm();
    static quint32 accPwRounds();
    static quint8 accPwMinlength();

    static QCryptographicHash::Algorithm admPwAlgorithm();
    static quint32 admPwRounds();
    static quint8 admPwMinlength();

    static quint32 defDomainquota();
    static quint32 defQuota();
    static quint32 defMaxaccounts();
    static QString defLanguage();
    static QString defTimezone();
    static quint8 defMaxdisplay();
    static quint8 defWarnlevel();

    static QString imapHost();
    static quint16 imapPort();
    static QString imapUser();
    static QString imapPassword();
    static QString imapPeername();
    static QAbstractSocket::NetworkLayerProtocol imapProtocol();
    static SkaffariIMAP::EncryptionType imapEncryption();
    static Account::CreateMailbox imapCreatemailbox();
    static bool imapDomainasprefix();
    static bool imapFqun();

private:
    static Password::Method m_accPwMethod;
    static Password::Algorithm m_accPwAlgorithm;
    static quint32 m_accPwRounds;
    static quint8 m_accPwMinlength;

    static QCryptographicHash::Algorithm m_admPwAlgorithm;
    static quint32 m_admPwRounds;
    static quint8 m_admPwMinlength;

    static quint32 m_defDomainquota;
    static quint32 m_defQuota;
    static quint32 m_defMaxaccounts;
    static QString m_defLanguage;
    static QString m_defTimezone;
    static quint8 m_defMaxdisplay;
    static quint8 m_defWarnlevel;

    static QString m_imapHost;
    static quint16 m_imapPort;
    static QString m_imapUser;
    static QString m_imapPassword;
    static QString m_imapPeername;
    static QAbstractSocket::NetworkLayerProtocol m_imapProtocol;
    static SkaffariIMAP::EncryptionType m_imapEncryption;
    static Account::CreateMailbox m_imapCreatemailbox;
    static bool m_imapDomainasprefix;
    static bool m_imapFqun;
};

#endif // SKAFFARICONFIG_H
