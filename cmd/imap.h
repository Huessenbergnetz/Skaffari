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
#include <QStringList>

#include "../common/global.h"

/*!
 * \ingroup skaffaricmd
 * \brief Provides method to connect to an IMAP server.
 *
 * This class only provides limited functionality. So nothing more than
 * is needed for the skaffaricmd command line utility.
 */
class Imap : public QSslSocket
{
    Q_OBJECT
public:
    /*!
     * \brief Availbale methods of connection encryptions.
     */
    enum EncryptionType : quint8 {
        Unsecured	= 0,	/**< 0: no encryption, mostly on port 143 */
        StartTLS	= 1,	/**< 1: use <A HREF="https://en.wikipedia.org/wiki/STARTTLS">StartTLS</A>, mostly on port 143 */
        IMAPS		= 2, 	/**< 2: use <A HREF="https://en.wikipedia.org/wiki/IMAPS">IMAPS</A>, mostly on port 993 */
    };

    /*!
     * \brief Types of IMAP4rev1 responses.
     */
    enum ResponseType : quint8 {
        OK			= 0,    /**< the request succeeded */
        NO			= 1,    /**< the request failed */
        BAD			= 2,    /**< indicates a protocol error such as unrecognized command or command syntax error */
        Undefined	= 3     /**< the response can not be parsed */
    };

    /*!
     * \brief Constructs a new %Imap object with the given \a parent.
     */
    explicit Imap(QObject *parent = nullptr);

    /*!
     * \brief Constructs a new %Imap object with the given parameters.
     * \param user          user name
     * \param password      user password
     * \param host          IMAP server host
     * \param port          IMAP server port
     * \param protocol      network protocol to use
     * \param conType       connection type
     * \param hierarchysep  the IMAP hierarchy separator
     * \param peerName      TLS/SSL peer name
     * \param parent        parent object
     */
    Imap(const QString &user, const QString &password, const QString &host = QStringLiteral("localhost"), quint16 port = 143, NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol, EncryptionType conType = StartTLS, QChar hierarchysep = QLatin1Char('.'), const QString peerName = QString(), QObject* parent = nullptr);
    ~Imap();

    /*!
     * \brief Performs a login to the configured IMAP server and returns \c true on success.
     */
    bool login();
    /*!
     * \brief Performs a logout from the configured IMAP server and returns \c true on success.
     */
    bool logout();

    /*!
     * \brief Returns the list of capabilities supported by the configured IMAP server.
     * \param forceReload   set to \c true if cache should be omitted
     */
    QStringList getCapabilities(bool forceReload = false);

    /*!
     * \brief Returns the storage quota for the given \a user.
     * \param user  IMAP user name
     */
    quota_pair getQuota(const QString &user);

    /*!
     * \brief Sets the \a user that should login to the IMAP server.
     */
    void setUser(const QString &user);
    /*!
     * \brief Sets the \a password of the user that sould login to the IMAP server.
     */
    void setPassword(const QString &password);
    /*!
     * \brief Sets the \a host the IMAP server is running on.
     */
    void setHost(const QString &host);
    /*!
     * \brief Sets the \a port the IMAP server is listening on.
     */
    void setPort(const quint16 &port);
    /*!
     * \brief Sets the network layer \a protocol that should be used for the connection.
     */
    void setProtocol(NetworkLayerProtocol protocol);
    /*!
     * \brief Sets the encryption \a type that should be used when establishing the connection.
     */
    void setEncryptionType(EncryptionType type);
    /*!
     * \brief Sets the IMAP connection \a parameters.
     * \param params    as returned by ConfigInput::askImapConfig()
     */
    void setParams(const QVariantHash &parameters);
    /*!
     * \brief Sets the IMAP hierarchy \a separator used by the server.
     */
    void setHierarchySeparator(QChar separator);

    /*!
     * \brief Returns the human readable name of the encryption \a type.
     */
    static QString encryptionTypeToString(EncryptionType type);
    /*!
     * \brief Returns the human readable name of the encryption \a type.
     */
    static QString encryptionTypeToString(quint8 type);
    /*!
     * \brief Returns the human readable name of the network layer \a protocol.
     */
    static QString networkProtocolToString(QAbstractSocket::NetworkLayerProtocol protocol);
    /*!
     * \brief Returns the human readable name of the network layer \a protocol.
     */
    static QString networkProtocolToString(quint8 protocol);

    /*!
     * \brief Returns the last occurred error.
     */
    QString lastError() const;

private:
    bool checkResponse(const QByteArray &data, const QString &tag = QLatin1String("."), QList<QByteArray> *response = nullptr);
    QString getTag();
    QString m_user;
    QString m_password;
    QString m_host;
    quint16 m_port = 143;
    NetworkLayerProtocol m_protocol = QAbstractSocket::AnyIPProtocol;
    EncryptionType m_encType = StartTLS;
    QChar m_hierarchysep = QLatin1Char('.');
    QString m_response;
    QString m_lastError;
    bool m_loggedIn = false;
    static QStringList m_capabilities;
    quint32 m_tagSequence = 0;
};

#endif // IMAP_H
