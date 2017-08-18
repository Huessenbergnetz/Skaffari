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

#ifndef SKAFFARIIMAP_H
#define SKAFFARIIMAP_H

#include <QSslSocket>
#include <QLoggingCategory>

#include "skaffariimaperror.h"
#include "../../common/global.h"

Q_DECLARE_LOGGING_CATEGORY(SK_IMAP)

namespace Cutelyst {
class Context;
}

/*!
 * \brief Helper class to perform IMAP4rev1 operations used by Skaffari.
 *
 * As Skaffari is not an IMAP email client, it only needs a few of the IMAP commands to be implemented. So, this
 * class is quite Skaffari specific.
 *
 * All default values for performing the IMAP operations will be read from the Skaffari configuration file (see SkaffariConfig)
 * but can be changed via the setter functions.
 *
 * \par Usage example
 * \code
 * SkaffariIMAP imap(c);
 * if (imap.login()) {
 *     imap.createMailbox("jhondoe");
 * }
 */
class SkaffariIMAP : public QSslSocket
{
	Q_OBJECT
public:
	/*!
	 * \brief Availbale methods of connection encryptions.
	 */
    enum EncryptionType : quint8 {
        Unsecured	= 0,	/**< no encryption, mostly on port 143 */
        StartTLS	= 1,	/**< use <A HREF="https://en.wikipedia.org/wiki/STARTTLS">StartTLS</A>, mostly on port 143 */
        IMAPS		= 2, 	/**< use <A HREF="https://en.wikipedia.org/wiki/IMAPS">IMAPS</A>, mostly on port 993 */
	};
    Q_ENUM(EncryptionType)
	
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
     * \brief Constructs a new SkaffariIMAP object.
     *
     * A newly created object will have the configuration read from the Skaffari configuration file IMAP section.
     *
     * \param context   Pointer to the current context. Used for translation of strings.
     * \param parent    Pointer to a parent object.
     */
    explicit SkaffariIMAP(Cutelyst::Context *context, QObject *parent = nullptr);

    /*!
     * \brief Deconstructs the SkaffariIMAP object.
     *
     * If there is a user logged in to the server it will be logged out.
     */
    ~SkaffariIMAP();

    /*!
     * \brief Performs the login operation for the current user.
     *
     * If the login operation failed, lastError() will provide further information.
     *
     * \sa logout(), isLoggedIn()
     * \return True on success.
     */
	bool login();

    /*!
     * \brief Performs logout operation for the current user.
     *
     * If the logout operation failed, lastError() will provide further information.
     *
     * \sa login(), isLoggedIn()
     * \return True on success.
     */
	bool logout();

    /*!
     * \brief Returns true if the current user is logged in.
     * \return True if the current user is logged in.
     */
    bool isLoggedIn() const;

    /*!
     * \brief Requests the capabilities from the server.
     *
     * The list of capabilities is cached. To reload the capabilities, set \a forceReload
     * to \c true. If the list is empty, lastError() will provide further information.
     *
     * \param forceReload   Set to true to force a reload and don't use the cached values.
     * \return List of capability strings.
     */
    QStringList getCapabilities(bool forceReload = false);

    /*!
     * \brief Requests the quota values for \a user.
     *
     * If both quota values are \c 0, lastError() might provide further information about occured errors,
     * but there also might be no quota set for the \a user account.
     *
     * \param user  The user to request the quota values for.
     * \return A quota pair containing used storage quota as first and total storage quota as second value. Both values in KiB.
     */
    quota_pair getQuota(const QString &user);

    /*!
     * \brief Sets the storage \a quota for the \a user.
     *
     * If setting the quota failed, lastError() will provide further information.
     *
     * \param user      The user to set the storage quota for.
     * \param quota     The storage quota value to set in KiB.
     * \return True on success.
     */
    bool setQuota(const QString &user, quota_size_t quota);

    /*!
     * \brief Creates the mailbox for the \a user.
     *
     * If mailbox creation failed, lastError() will provide further information.
     *
     * \sa deleteMailbox()
     * \param user  Mailbox/user name.
     * \return True on success.
     */
    bool createMailbox(const QString &user);

    /*!
     * \brief Deletes the mailbox for the \a user.
     *
     * If mailbox deletion failed, lastError() will provide further information.
     *
     * \sa createMailbox()
     * \param user  Mailbox/user name.
     * \return True on success.
     */
    bool deleteMailbox(const QString &user);

    /*!
     * \brief Creates a new \a folder in the current user's mailbox.
     *
     * If folder creation failed, lastError() will provide further information.
     *
     * \param folder    Name of the new folder. Can be UTF-8 an will be automatically converted into UTF-7-IMAP.
     * \return True on success.
     */
    bool createFolder(const QString &folder);

    /*!
     * \brief Sets the \a acl for the \a user on the \a mailbox.
     *
     * If setting the ACL failed, lastError() will provide further information.
     *
     * \sa deleteAcl()
     * \param mailbox   The mailbox to set the ACL on.
     * \param user      The user to set the ACL for.
     * \param acl       The string defining the ACL.
     * \return True on success.
     */
    bool setAcl(const QString &mailbox, const QString &user, const QString &acl = QString());

    /*!
     * \brief Deletes the ACL for the \a user on the \a mailbox.
     *
     * If deleting the ACL failed, lastError() will provide further information.
     *
     * \sa setAcl()
     * \param mailbox   The mailbox to delete the ACL on.
     * \param user      The user that should have the ACL deleted on the mailbox.
     * \return True on success.
     */
    bool deleteAcl(const QString &mailbox, const QString &user);

    /*!
     * \brief Requests a list of all mailboxes on the server.
     * \return List of all mailboxes on the server.
     */
    QStringList getMailboxes();

    /*!
     * \brief Returns the last occured error.
     * \return Last error object.
     */
	SkaffariIMAPError lastError() const;

    /*!
     * \brief Sets the \a user to connect to the server.
     *
     * Defaults to the Skaffari configuration value of \a IMAP/user.
     *
     * \param user User name.
     */
	void setUser(const QString &user);
    /*!
     * \brief Sets the user's \a password to connect to the server.
     *
     * Defaults to the Skaffari configuration value of \a IMAP/password.
     *
     * \param password User's password.
     */
	void setPassword(const QString &password);
    /*!
     * \brief Sets the IMAP server \a host address.
     *
     * Defaults to the Skaffari configuration value of \a IMAP/host.
     *
     * \param host IMAP server host address.
     */
	void setHost(const QString &host);
    /*!
     * \brief Sets the IMAP server \a port.
     *
     * Defaults to the Skaffari configuration value of \a IMAP/port (143 if not set).
     *
     * \param port IMAP server port.
     */
	void setPort(const quint16 &port);
    /*!
     * \brief Sets the \a protocol to be used.
     *
     * Defaults to the Skaffari configuration value of \a IMAP/protocol (any if not set).
     *
     * \param protocol The protocol to use (IPv4 or IPv6 or any).
     */
	void setProtocol(NetworkLayerProtocol protocol);
    /*!
     * \brief Sets the encryption type.
     *
     * Defaults to the Skaffari configuration value of \a IMAP/encryption (StartTLS if not set).
     *
     * \param encType Encryption mechanism to use.
     */
	void setEncryptionType(EncryptionType encType);

    /*!
     * \brief Converts an UTF-8 string into UTF-7-IMAP
     * \param str UTF-8 string to convert.
     * \return UTF-7-IMAP representation of the string.
     */
    static QString toUTF7Imap(const QString &str);

    /*!
     * \brief Converts an UTF-7-IMAP byte array into an UTF-8 string.
     * \param ba UTF-7-IMAP byte array to convert.
     * \return UTF-8 string.
     */
    static QString fromUTF7Imap(const QByteArray &ba);

private:
    /*!
     * \brief Sets the last error object to a timeout error and aborts the operation.
     * \return Always false.
     */
	bool connectionTimeOut();

    /*!
     * \brief Checks the response of the IMAP server.
     *
     * If the check fails, lastError() will provide further information.
     *
     * \param data      The data requested from the IMAP server.
     * \param tag       The tag used for the request.
     * \param response  Pointer to a vector that will contain the resulting lines (if any).
     * \return True on success.
     */
    bool checkResponse(const QByteArray &data, const QString &tag = QString(), QVector<QByteArray> *response = nullptr);
    /*!
     * \brief Returns a new tag.
     * \return New sequential tag.
     */
    QString getTag();
    /*!
     * \brief Sets the lastError() to no error.
     */
    void setNoError();
    /*!
     * \brief Sends the \a command to the IMAP server.
     *
     * If sending the command failed, lastError() will provide further information.
     *
     * \param command   The command to send.
     * \return True on success.
     */
    bool sendCommand(const QString &command);

	QString m_user;
	QString m_password;
	QString m_host;
    quint16 m_port = 143;
    NetworkLayerProtocol m_protocol = QAbstractSocket::AnyIPProtocol;
    EncryptionType m_encType = StartTLS;
    QChar m_hierarchysep = QLatin1Char('.');
    Cutelyst::Context *m_c;
	SkaffariIMAPError m_imapError;
    bool m_loggedIn = false;
    static QStringList m_capabilities;
    quint32 m_tagSequence = 0;
    static const QString m_allAcl;

    Q_DISABLE_COPY(SkaffariIMAP)
};

#endif // SKAFFARIIMAP_H
