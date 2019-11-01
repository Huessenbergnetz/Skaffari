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

#include "autodiscover.h"
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
#include <QTime>
#include <QCryptographicHash>

Q_LOGGING_CATEGORY(SK_AUTODISCOVER, "skaffari.autoconfig")

using namespace Cutelyst;

Autodiscover::Autodiscover(QObject *parent)
    : Controller(parent)
{
}

void Autodiscover::index(Context *c)
{
    if (!SkaffariConfig::autoconfigEnabled()) {
        setError(c, Response::NotFound, c->translate("Autodiscover", "Autodiscover is not enabled."), 601);
        return;
    }

    const QString host = c->req()->uri().host();
    if (Q_UNLIKELY(!host.startsWith(QLatin1String("autodiscover")))) {
        setError(c, Response::Unauthorized, c->translate("Autodiscover", "Access to this URL is only allowed with the appropriate target host."), 601);
        return;
    }

    QString email;
//    const int mapiCapable = c->req()->header(QStringLiteral("X-MapiHttpCapability")).toInt();
//    if (mapiCapable > 0) {
//        email = c->req()->header(QStringLiteral("X-AnchorMailbox"));
//    }

    if (email.isEmpty() && c->req()->body()) {
        if (!c->req()->body()->open(QIODevice::ReadOnly)) {
            qCWarning(SK_AUTODISCOVER, "%s", "Failed to parse autodiscover request xml.");
            setError(c, Response::InternalServerError, c->translate("Autodiscover", "Internal server error."), 603);
            return;
        }

        QDomDocument xml;
        QString errorMsg;

        if (!xml.setContent(c->req()->body(), &errorMsg)) {
            qCWarning(SK_AUTODISCOVER, "%s", "Failed to parse autodiscover request xml.");
            setError(c, Response::BadRequest, c->translate("Autodiscover", "Failed to parse request XML: %1").arg(errorMsg), 600);
            c->req()->body()->close();
            return;
        }
        c->req()->body()->close();

        email = xml.documentElement().firstChildElement(QStringLiteral("Request")).firstChildElement(QStringLiteral("EMailAddress")).text();
    }

    if (email.isEmpty()) {
        qCWarning(SK_AUTODISCOVER, "Search for invalid email addres.");
        setError(c, Response::BadRequest, c->translate("Autodiscover", "Empty email address or address not recognizable."), 500);
        return;
    }

    if (Q_UNLIKELY(!ValidatorEmail::validate(email))) {
        qCWarning(SK_AUTODISCOVER, "Search for invalid email addres.");
        setError(c, Response::BadRequest, c->translate("Autodiscover", "Invalid email address."), 500);
        return;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :alias"));
    q.bindValue(QStringLiteral(":alias"), email);

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(SK_AUTODISCOVER, "Failed to query database for username: %s", qUtf8Printable(q.lastError().text()));
        setError(c, Response::InternalServerError, c->translate("Autodiscover", "Internal server error."), 603);
        return;
    }

    const QString username = q.next() ? q.value(0).toString() : QString();

    if (username.isEmpty()) {
        qCWarning(SK_AUTODISCOVER, "Autoconfiguration requested for unknown email address %s from %s", qUtf8Printable(email), qUtf8Printable(c->req()->addressString()));
        setError(c, Response::NotFound, c->translate("Autodiscover", "Email address not found."), 500);
        return;
    }

    qCInfo(SK_AUTODISCOVER, "Autodiscover requested for email address \"%s\".", qUtf8Printable(email));

    const QString mailDomain = email.mid(email.lastIndexOf(QLatin1Char('@')) + 1);

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, autoconfig FROM domain WHERE domain_name = :domain_name"));
    q.bindValue(QStringLiteral(":domain_name"), mailDomain);

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(SK_AUTODISCOVER, "Failed to query database for domain’s autoconfig strategy.");
        setError(c, Response::InternalServerError, c->translate("Autodiscover", "Internal server error."), 603);
        return;
    }

    if (Q_UNLIKELY(!q.next())) {
        qCCritical(SK_AUTODISCOVER, "Can not find autoconfig strategy for domain %s.", qUtf8Printable(mailDomain));
        setError(c, Response::InternalServerError, c->translate("Autodiscover", "Can not determine autoconfiguration."), 602);
        return;
    }

    const dbid_t domainId = q.value(0).value<dbid_t>();
    const qint8 autoconfigStrategy = q.value(1).value<qint8>();

    if (autoconfigStrategy == 0) {
        setError(c, Response::NotFound, c->translate("Autodiscover", "Autodiscover is not enabled."), 601);
        return;
    }

    SkaffariError e(c);
    const std::vector<AutoconfigServer> servers = autoconfigStrategy == 1 ? AutoconfigServer::list(c, 0, e) : AutoconfigServer::list(c, domainId, e);

    if (e.type() != SkaffariError::NoError) {
        setError(c, Response::InternalServerError, c->translate("Autodiscover", "Internal server error."), 603);
        return;
    }

    if (servers.empty()) {
        qCWarning(SK_AUTODISCOVER, "No autoconfig servers found for domain ID %u.", domainId);
        setError(c, Response::InternalServerError, c->translate("Autodiscover", "Can not determine autoconfiguration."), 602);
        return;
    }

    QDomDocument xml;
    auto autodiscover = xml.createElement(QStringLiteral("Autodiscover"));
    autodiscover.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://schemas.microsoft.com/exchange/autodiscover/responseschema/2006"));
    xml.appendChild(autodiscover);

    auto response = xml.createElement(QStringLiteral("Response"));
    response.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://schemas.microsoft.com/exchange/autodiscover/outlook/responseschema/2006a"));
    autodiscover.appendChild(response);

    auto account = xml.createElement(QStringLiteral("Account"));
    response.appendChild(account);

    auto accountType = xml.createElement(QStringLiteral("AccountType"));
    account.appendChild(accountType);
    accountType.appendChild(xml.createTextNode(QStringLiteral("email")));

    auto action = xml.createElement(QStringLiteral("Action"));
    account.appendChild(action);
    action.appendChild(xml.createTextNode(QStringLiteral("settings")));

    for (const AutoconfigServer &server : servers) {
        auto proto = xml.createElement(QStringLiteral("Protocol"));
        account.appendChild(proto);

        auto serverType = xml.createElement(QStringLiteral("Type"));
        proto.appendChild(serverType);
        switch(server.type()) {
        case AutoconfigServer::Imap:
            serverType.appendChild(xml.createTextNode(QStringLiteral("IMAP")));
            break;
        case AutoconfigServer::Pop3:
            serverType.appendChild(xml.createTextNode(QStringLiteral("POP3")));
            break;
        case AutoconfigServer::Smtp:
            serverType.appendChild(xml.createTextNode(QStringLiteral("SMTP")));
            break;
        }

        auto hostname = xml.createElement(QStringLiteral("Server"));
        proto.appendChild(hostname);
        hostname.appendChild(xml.createTextNode(server.hostname()));

        auto port = xml.createElement(QStringLiteral("Port"));
        proto.appendChild(port);
        port.appendChild(xml.createTextNode(QString::number(server.port())));

        auto loginName = xml.createElement(QStringLiteral("LoginName"));
        proto.appendChild(loginName);
        loginName.appendChild(xml.createTextNode(username));

        auto domainRequired = xml.createElement(QStringLiteral("DomainRequired"));
        proto.appendChild(domainRequired);
        domainRequired.appendChild(xml.createTextNode(QStringLiteral("off")));

        auto spa = xml.createElement(QStringLiteral("SPA"));
        proto.appendChild(spa);
        spa.appendChild(xml.createTextNode(QStringLiteral("off")));

        auto ssl = xml.createElement(QStringLiteral("SSL"));
        proto.appendChild(ssl);
        switch (server.socketType()) {
        case AutoconfigServer::Plain:
            ssl.appendChild(xml.createTextNode(QStringLiteral("off")));
            break;
        case AutoconfigServer::Ssl:
            ssl.appendChild(xml.createTextNode(QStringLiteral("on")));
            break;
        case AutoconfigServer::StartTls:
            ssl.appendChild(xml.createTextNode(QStringLiteral("on")));
            break;
        }

        if (server.socketType() != AutoconfigServer::Plain) {
            auto encryption = xml.createElement(QStringLiteral("Encryption"));
            proto.appendChild(encryption);
            if (server.socketType() == AutoconfigServer::StartTls) {
                encryption.appendChild(xml.createTextNode(QStringLiteral("TLS")));
            } else {
                encryption.appendChild(xml.createTextNode(QStringLiteral("SSL")));
            }
        }

        auto authRequired = xml.createElement(QStringLiteral("AuthRequired"));
        proto.appendChild(authRequired);
        authRequired.appendChild(xml.createTextNode(QStringLiteral("on")));

        if (server.type() == AutoconfigServer::Smtp) {
            auto usepopauth = xml.createElement(QStringLiteral("UsePOPAuth"));
            proto.appendChild(usepopauth);
            usepopauth.appendChild(xml.createTextNode(QStringLiteral("on")));

            auto smtplast = xml.createElement(QStringLiteral("SMTPLast"));
            proto.appendChild(smtplast);
            smtplast.appendChild(xml.createTextNode(QStringLiteral("off")));
        }
    }

    c->res()->setBody(xml.toString(4));
    c->res()->setContentType(QStringLiteral("application/xml"));
}

void Autodiscover::setError(Context *c, Response::HttpStatus status, const QString &msg, int errorCode)
{
    Q_ASSERT_X(c, "set autodiscov error", "invalid context object");

//    c->res()->setStatus(status);

    QDomDocument xml;
    auto autodiscover = xml.createElement(QStringLiteral("Autodiscover"));
    autodiscover.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://schemas.microsoft.com/exchange/autodiscover/responseschema/2006"));
    xml.appendChild(autodiscover);

    auto response = xml.createElement(QStringLiteral("Response"));
    autodiscover.appendChild(response);

    auto error = xml.createElement(QStringLiteral("Error"));
    error.setAttribute(QStringLiteral("Time"), QTime::currentTime().toString(Qt::ISODateWithMs));
    error.setAttribute(QStringLiteral("Id"), QString::fromLatin1(QCryptographicHash::hash(c->req()->uri().host().toUtf8(), QCryptographicHash::Md4).toHex()));
    autodiscover.appendChild(error);

    auto code = xml.createElement(QStringLiteral("ErrorCode"));
    error.appendChild(code);
    code.appendChild(xml.createTextNode(QString::number(errorCode)));

    auto message = xml.createElement(QStringLiteral("Message"));
    error.appendChild(message);
    message.appendChild(xml.createTextNode(msg));

    auto debugdata = xml.createElement(QStringLiteral("DebugData"));
    error.appendChild(debugdata);

    c->res()->setBody(xml.toString(4));
    c->res()->setContentType(QStringLiteral("application/xml"));
}

#include "moc_autodiscover.cpp"
