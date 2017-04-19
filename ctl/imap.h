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

#ifndef IMAP_H
#define IMAP_H

#include <QSslSocket>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>

class Imap : public QSslSocket
{
    Q_OBJECT
public:
    enum EncryptionType : quint8 {
        Unsecured	= 0,	/**< 0: no encryption, mostly on port 143 */
        StartTLS	= 1,	/**< 1: use <A HREF="https://en.wikipedia.org/wiki/STARTTLS">StartTLS</A>, mostly on port 143 */
        IMAPS		= 2, 	/**< 2: use <A HREF="https://en.wikipedia.org/wiki/IMAPS">IMAPS</A>, mostly on port 993 */
    };

    enum ResponseType : quint8 {
        OK			= 0,
        NO			= 1,
        BAD			= 2,
        Undefined	= 3
    };

    explicit Imap(QObject *parent = nullptr);
    Imap(const QString &user, const QString &password, const QString &host = QStringLiteral("localhost"), quint16 port = 143, NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol, EncryptionType conType = StartTLS, QObject* parent = nullptr);
    ~Imap();

    bool login();
    bool logout();

    QStringList getCapabilities();

    void setUser(const QString &user);
    void setPassword(const QString &password);
    void setHost(const QString &host);
    void setPort(const quint16 &port);
    void setProtocol(NetworkLayerProtocol protocol);
    void setEncryptionType(EncryptionType encType);

    static QString encryptionTypeToString(EncryptionType et);
    static QString encryptionTypeToString(quint8 et);
    static QString networkProtocolToString(QAbstractSocket::NetworkLayerProtocol nlp);
    static QString networkProtocolToString(quint8 nlp);

private:
    bool checkResponse(const QByteArray &resp, const QString &tag = QString());
    QString m_user;
    QString m_password;
    QString m_host;
    quint16 m_port = 143;
    NetworkLayerProtocol m_protocol = QAbstractSocket::AnyIPProtocol;
    EncryptionType m_encType = StartTLS;
    QString m_response;
    QRegularExpression m_responseRegex = QRegularExpression(QStringLiteral("\\s*.* (OK|BAD|WARN) "));
    QRegularExpressionMatch m_responseRegexMatch;
};

#endif // IMAP_H
