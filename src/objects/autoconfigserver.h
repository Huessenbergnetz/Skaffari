/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2019 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef AUTOCONFIGSERVER_H
#define AUTOCONFIGSERVER_H

#include "../../common/global.h"
#include <QObject>
#include <QSharedDataPointer>
#include <vector>

namespace Cutelyst {
class Context;
}

class SkaffariError;

/*!
 * \ingroup skaffariobjects
 * \brief Represents a server used for autoconfiguration of mail user agents.
 */
class AutoconfigServer
{
    Q_GADGET
    Q_PROPERTY(dbid_t id READ id CONSTANT)
    Q_PROPERTY(dbid_t domainId READ domainId CONSTANT)
    Q_PROPERTY(AutoconfigServer::Type type READ type CONSTANT)
    Q_PROPERTY(QString hostname READ hostname CONSTANT)
    Q_PROPERTY(quint16 port READ port CONSTANT)
    Q_PROPERTY(AutoconfigServer::SocketType socketType READ socketType CONSTANT)
    Q_PROPERTY(AutoconfigServer::Authentication authentication READ authentication CONSTANT)
    Q_PROPERTY(qint16 sorting READ sorting CONSTANT)
public:
    /*!
     * \brief Type of the server.
     */
    enum Type : qint8 {
        Imap    = 0,    /**< IMAP server */
        Pop3    = 1,    /**< POP3 server */
        Smtp    = 2     /**< SMTP server */
    };
    Q_ENUM(Type)

    /*!
     * \brief Type of the socket to use for this server.
     */
    enum SocketType : qint8 {
        Plain       = 0,    /**< Unencrypted connection */
        StartTls    = 1,    /**< Use STARTTLS for this connection */
        Ssl         = 2     /**< USE SSL/TLS socket connection */
    };
    Q_ENUM(SocketType)

    /*!
     * \brief %Authentication method to use for this server.
     */
    enum Authentication : qint8 {
        Cleartext       = 0,    /**< Use unencrypted cleartext authentication like SASL PLAIN or LOGIN. Should \b not be used with SocketType::Plain */
        Encrypted       = 1,    /**< Use encrypted authentication method like CRAM-MD5 or DIGEST-MD5. Not NTLM. */
        Ntlm            = 2,    /**< Use NTLM (or NTLMv2 or successors), the Windows login mechanism. */
        Gssapi          = 3,    /**< Use Kerberos / GSSAPI, a single-signon mechanism used for big sites. */
        ClientIpAddress = 4,    /**< The server recognizes this user based on the IP address. No authentication needed, the server will require no username nor password. */
        TlsClientCert   = 5     /**< On the SSL/TLS layer, the server requests a client certificate and the client sends one (possibly after letting the user select/confirm one), if available. */
    };
    Q_ENUM(Authentication)

    /*!
     * \brief Constructs an invalid, empty %AutoconfigServer object.
     */
    AutoconfigServer();

    /*!
     * \brief Constructs a new %AutoconfigServer objec twith the givern parameters.
     * \param id                Database ID.
     * \param domainId          Database ID of the domain the server belongs to. Global servers use 0.
     * \param type              Type of the server.
     * \param hostname          The server hostname.
     * \param port              The port the service defined by \a type runs on.
     * \param socketType        The used socket.
     * \param authentication    The authentication mechanism to use.
     * \param sorting           Sorting value, lower values come first.
     */
    AutoconfigServer(dbid_t id, dbid_t domainId, Type type, const QString &hostname, quint16 port, SocketType socketType, Authentication authentication, qint8 sorting);

    /*!
     * \brief Constructs a copy of \a other.
     */
    AutoconfigServer(const AutoconfigServer &other);

    /*!
     * \brief Move-constructs an %AutoconfigServer instance, making it point at the same object that \a other was pointing to.
     */
    AutoconfigServer(AutoconfigServer &&other) noexcept;

    /*!
     * \brief Assigns \a other to the %AutoconfigServer and returns a reference to this %AutoconfigServer.
     */
    AutoconfigServer& operator=(const AutoconfigServer &other);

    /*!
     * \brief Move-assigns \a other to this %AutoconfigServer.
     */
    AutoconfigServer& operator=(AutoconfigServer &&other) noexcept;

    /*!
     * \brief Destroys the %AutoconfigServer object.
     */
    ~AutoconfigServer();

    /*!
     * \brief Swaps this %AutoconfigServer instance with \a other.
     */
    void swap(AutoconfigServer &other);

    /*!
     * \brief Returns the database ID of the server.
     */
    dbid_t id() const;

    /*!
     * \brief Returns the databse ID of the domain this server belongs to.
     *
     * If \c 0 is returned, this is a global autoconfig server.
     */
    dbid_t domainId() const;

    /*!
     * \brief Returns the type of the server.
     */
    Type type() const;

    /*!
     * \brief Returns the server’s hostname.
     */
    QString hostname() const;

    /*!
     * \brief Returns the server’s port.
     */
    quint16 port() const;

    /*!
     * \brief Returns the SocketType to use.
     */
    SocketType socketType() const;

    /*!
     * \brief Returns the Authentication mechanism to use.
     */
    Authentication authentication() const;

    /*!
     * \brief Returns the sorting value. Lower comes first.
     */
    qint16 sorting() const;

    /*!
     * \brief Returns \c true if the server is valid, otherwise \c false.
     *
     * A server is valid if the id() is greater than \c 0.
     */
    bool isValid() const;

    /*!
     * \brief Returns \c true if this server is equal to \a other; otherwise returns \c false.
     */
    bool operator==(const AutoconfigServer &other);

    /*!
     * \brief Returns \c true if this server is not equal to \a other; otherwise returns \c false.
     */
    bool operator!=(const AutoconfigServer &other);

    /*!
     * \brief Returns the %AutoconfigServer specified by \a domainId and \a serverId from the database.
     * \param c         Pointer to the current context, used for translating strings.
     * \param domainId  The database ID of the domain the server belongs to, \c 0 indicates global servers.
     * \param serverId  The database ID of the server to return.
     * \param e         Object taking error information.
     */
    static AutoconfigServer get(Cutelyst::Context *c, dbid_t domainId, dbid_t serverId, SkaffariError &e);

    /*!
     * \brief Returns a list of %AutoconfigServer for the specified \a domainId from the database.
     * \param c         Pointer to the current context, used for translating strings.
     * \param domainId  The database ID of the domain the server belongs to, \c 0 indicates global servers.
     * \param e         Object taking error information.
     */
    static std::vector<AutoconfigServer> list(Cutelyst::Context *c, dbid_t domainId, SkaffariError &e);

    /*!
     * \brief Creates a new %AutoconfigServer and returns it.
     *
     * The returned server might be invalid if the creation was not successful. User isValid() to check
     * for validity.
     *
     * \par Keys in the parameters \a p
     * Key            | Converted Type | Description
     * ---------------|----------------|------------
     * type           | Type           | The type of the server.
     * hostname       | QString        | The hostname of the server.
     * port           | quint16        | The port of the server.
     * socketType     | SocketType     | The type of socket to use for this server.
     * authentication | Authentication | The authentication method to use for this server.
     * sorting        | qint8          | The sorting number for this server.
     *
     * \param c         Pointer to the current context, used for translating strings.
     * \param domainId  The database ID of the domain the server belongs to, \c 0 indicates global servers.
     * \param p         Parameters used to create the server.
     * \param e         Object taking error information.
     */
    static AutoconfigServer create(Cutelyst::Context *c, dbid_t domainId, const QVariantHash &p, SkaffariError &e);

    /*!
     * \brief Removes this server from the database and returns \c true on success, otherwise \c false.
     * \param c Pointer to the current context, used for translating strings.
     * \param e Object taking error information.
     */
    bool remove(Cutelyst::Context *c, SkaffariError &e);

    /*!
     * \brief Updates this server in the database and returns \c true on success, otherwise \c false.
     *
     * \par Keys in the parameters \a p
     * Key            | Converted Type | Description
     * ---------------|----------------|------------
     * type           | Type           | The type of the server.
     * hostname       | QString        | The hostname of the server.
     * port           | quint16        | The port of the server.
     * socketType     | SocketType     | The type of socket to use for this server.
     * authentication | Authentication | The authentication method to use for this server.
     * sorting        | qint8          | The sorting number for this server.
     *
     * \param c Pointer to the current context, used for translating strings.
     * \param p Parameters used to create the server.
     * \param e Object taking error information.
     */
    bool update(Cutelyst::Context *c, const QVariantHash &p, SkaffariError &e);

private:
    class Data;
    QSharedDataPointer<Data> d;

    friend QDataStream &operator<<(QDataStream &stream, const AutoconfigServer &server);
    friend QDataStream &operator>>(QDataStream &stream, AutoconfigServer &server);
};

Q_DECLARE_METATYPE(AutoconfigServer)
Q_DECLARE_TYPEINFO(AutoconfigServer, Q_MOVABLE_TYPE);

/*!
 * \relates AutoconfigServer
 * \brief Writes the \a server to the \a dbg stream and returns the stream.
 */
QDebug operator<<(QDebug dbg, const AutoconfigServer &server);

/*!
 * \relates AutoconfigServer
 * \brief Writes the given \a server to the given \a stream.
 */
QDataStream &operator<<(QDataStream &stream, const AutoconfigServer &server);

/*!
 * \relates AutoconfigServer
 * \brief Reads an %AutoconfigServer from the given \a stream and stores it in the given \a server.
 */
QDataStream &operator>>(QDataStream &stream, AutoconfigServer &server);

/*!
 * \relates AutoconfigServer
 * \brief Writes the given \a servers to the given \a stream.
 */
QDataStream &operator<<(QDataStream &stream, const std::vector<AutoconfigServer> &servers);

/*!
 * \relates AutoconfigServer
 * \brief Reads a list of %AutoconfigServer from the given \a stream and stores it in the given \a servers.
 */
QDataStream &operator>>(QDataStream &stream, std::vector<AutoconfigServer> &servers);

#endif // AUTOCONFIGSERVER_H
