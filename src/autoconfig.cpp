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

#include "autoconfig.h"
#include "utils/skaffariconfig.h"
#include "objects/autoconfigserver.h"
#include "objects/skaffarierror.h"
#include <Cutelyst/Plugins/Utils/validatoremail.h>
#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlQuery>
#include <QSqlError>
#include <QUrl>
#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>
#include <QDomText>

Q_LOGGING_CATEGORY(SK_AUTOCONFIG, "skaffari.autoconfig")

using namespace Cutelyst;

Autoconfig::Autoconfig(QObject *parent)
    : Controller(parent)
{
}

void Autoconfig::index(Context *c)
{
    if (!SkaffariConfig::autoconfigEnabled()) {
        c->res()->setBody(c->translate("Autoconfig", "Autoconfig is not enabled."));
        c->res()->setStatus(Response::NotFound);
        return;
    }

    const QString host = c->req()->uri().host();
    if (Q_UNLIKELY(!host.startsWith(QLatin1String("autoconfig")))) {
        c->res()->setBody(c->translate("Autoconfig", "Access to this URL is only allowed with the appropriate target host."));
        c->res()->setStatus(Response::Forbidden);
        return;
    }

    const QString email = c->req()->queryParam(QStringLiteral("emailaddress"));
    qCInfo(SK_AUTOCONFIG, "Autoconfig requested for email address \"%s\".", qUtf8Printable(email));
    if (Q_UNLIKELY(!ValidatorEmail::validate(email))) {
        qCWarning(SK_AUTOCONFIG, "Search for invalid email addres.");
        c->res()->setBody(c->translate("Autoconfig", "Invalid email address."));
        c->res()->setStatus(Response::BadRequest);
        return;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :alias"));
    q.bindValue(QStringLiteral(":alias"), email);

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(SK_AUTOCONFIG, "Failed to query database for username: %s", qUtf8Printable(q.lastError().text()));
        c->res()->setBody(c->translate("Autoconfig", "SQL query failed."));
        c->res()->setStatus(Response::InternalServerError);
        return;
    }

    const QString username = q.next() ? q.value(0).toString() : QString();

    if (username.isEmpty()) {
        qCWarning(SK_AUTOCONFIG, "Autoconfiguration requested for unknown email address %s from %s", qUtf8Printable(email), qUtf8Printable(c->req()->addressString()));
        c->res()->setBody(c->translate("Autoconfig", "Email address not found."));
        c->res()->setStatus(Response::NotFound);
        return;
    }

    const QString mailDomain = email.mid(email.lastIndexOf(QLatin1Char('@')) + 1);

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, autoconfig FROM domain WHERE domain_name = :domain_name"));
    q.bindValue(QStringLiteral(":domain_name"), mailDomain);

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(SK_AUTOCONFIG, "Failed to query database for domain’s autoconfig strategy.");
        c->res()->setBody(c->translate("Autoconfig", "SQL query failed."));
        c->res()->setStatus(Response::InternalServerError);
        return;
    }

    if (Q_UNLIKELY(!q.next())) {
        qCCritical(SK_AUTOCONFIG, "Can not find autoconfig strategy for domain %s.", qUtf8Printable(mailDomain));
        c->res()->setBody(c->translate("Autoconfig", "Domain not found."));
        c->res()->setStatus(Response::NotFound);
        return;
    }

    const dbid_t domainId = q.value(0).value<dbid_t>();
    const qint8 autoconfigStrategy = q.value(1).value<qint8>();

    if (autoconfigStrategy == 0) {
        c->res()->setBody(c->translate("Autoconfig", "Autoconfig is not enabled."));
        c->res()->setStatus(Response::NotFound);
        return;
    }

    SkaffariError e(c);
    const std::vector<AutoconfigServer> servers = autoconfigStrategy == 1 ? AutoconfigServer::list(c, 0, e) : AutoconfigServer::list(c, domainId, e);

    if (e.type() != SkaffariError::NoError) {
        c->res()->setStatus(Response::InternalServerError);
        c->res()->setBody(e.type() == SkaffariError::Sql ? c->translate("Autoconfig", "SQL query failed.") : c->translate("Autoconfig", "Internal server error."));
        return;
    }

    if (servers.empty()) {
        qCWarning(SK_AUTOCONFIG, "No autoconfig servers found for domain ID %u.", domainId);
        c->res()->setBody(c->translate("Autoconfig", "Autoconfig is not enabled."));
        c->res()->setStatus(Response::NotFound);
        return;
    }

    QStringList providerDomains{SkaffariConfig::autoconfigId()};
    if (SkaffariConfig::autoconfigId() != mailDomain) {
        providerDomains.push_back(mailDomain);
    }

    QDomDocument xml;

    auto root = xml.createElement(QStringLiteral("clientConfig"));
    root.setAttribute(QStringLiteral("version"), QStringLiteral("1.1"));
    xml.appendChild(root);

    auto provider = xml.createElement(QStringLiteral("emailProvider"));
    provider.setAttribute(QStringLiteral("id"), SkaffariConfig::autoconfigId());
    root.appendChild(provider);

    for (const QString &d : providerDomains) {
        auto providerDomain = xml.createElement(QStringLiteral("domain"));
        provider.appendChild(providerDomain);
        providerDomain.appendChild(xml.createTextNode(d));
    }

    auto displayName = xml.createElement(QStringLiteral("displayName"));
    provider.appendChild(displayName);
    displayName.appendChild(xml.createTextNode(SkaffariConfig::autoconfigDisplayName()));

    auto displayShortName = xml.createElement(QStringLiteral("displayShortName"));
    provider.appendChild(displayShortName);
    displayShortName.appendChild(xml.createTextNode(SkaffariConfig::autoconfigDisplayNameShort()));

    for (const AutoconfigServer &server : servers) {
        QDomElement s;
        switch (server.type()) {
        case AutoconfigServer::Imap:
            s = xml.createElement(QStringLiteral("incomingServer"));
            s.setAttribute(QStringLiteral("type"), QStringLiteral("imap"));
            break;
        case AutoconfigServer::Pop3:
            s = xml.createElement(QStringLiteral("incomingServer"));
            s.setAttribute(QStringLiteral("type"), QStringLiteral("pop3"));
            break;
        case AutoconfigServer::Smtp:
            s = xml.createElement(QStringLiteral("outgoingServer"));
            s.setAttribute(QStringLiteral("type"), QStringLiteral("smtp"));
            break;
        }
        provider.appendChild(s);

        auto host = xml.createElement(QStringLiteral("hostname"));
        s.appendChild(host);
        host.appendChild(xml.createTextNode(server.hostname()));

        auto port = xml.createElement(QStringLiteral("port"));
        s.appendChild(port);
        port.appendChild(xml.createTextNode(QString::number(server.port())));

        auto socketType = xml.createElement(QStringLiteral("socketType"));
        s.appendChild(socketType);
        switch (server.socketType()) {
        case AutoconfigServer::Plain:
            socketType.appendChild(xml.createTextNode(QStringLiteral("plain")));
            break;
        case AutoconfigServer::StartTls:
            socketType.appendChild(xml.createTextNode(QStringLiteral("STARTTLS")));
            break;
        case AutoconfigServer::Ssl:
            socketType.appendChild(xml.createTextNode(QStringLiteral("SSL")));
            break;
        }

        auto authentication = xml.createElement(QStringLiteral("authentication"));
        s.appendChild(authentication);
        switch (server.authentication()) {
        case AutoconfigServer::Cleartext:
            authentication.appendChild(xml.createTextNode(QStringLiteral("password-cleartext")));
            break;
        case AutoconfigServer::Encrypted:
            authentication.appendChild(xml.createTextNode(QStringLiteral("password-encrypted")));
            break;
        case AutoconfigServer::Ntlm:
            authentication.appendChild(xml.createTextNode(QStringLiteral("NTLM")));
            break;
        case AutoconfigServer::Gssapi:
            authentication.appendChild(xml.createTextNode(QStringLiteral("GSSAPI")));
            break;
        case AutoconfigServer::ClientIpAddress:
            authentication.appendChild(xml.createTextNode(QStringLiteral("client-IP-address")));
            break;
        case AutoconfigServer::TlsClientCert:
            authentication.appendChild(xml.createTextNode(QStringLiteral("TLS-client-cert")));
            break;
        }

        auto user = xml.createElement(QStringLiteral("username"));
        s.appendChild(user);
        user.appendChild(xml.createTextNode(username));

    }

    c->res()->setBody(xml.toString(2));
    c->res()->setContentType(QStringLiteral("text/xml; charset=utf-8"));
}

#include "moc_autoconfig.cpp"
