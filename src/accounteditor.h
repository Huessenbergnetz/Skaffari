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

#ifndef ACCOUNTEDITOR_H
#define ACCOUNTEDITOR_H

#include <Cutelyst/Controller>

using namespace Cutelyst;
class SkaffariEngine;

/*!
 * \ingroup skaffaricore
 * \brief Routes for the account namespace.
 */
class AccountEditor : public Controller
{
    Q_OBJECT
    C_NAMESPACE("account")
public:
    /*!
     * \brief Constructs a new %AccountEditor controller with the given \a parent.
     */
    explicit AccountEditor(QObject *parent = nullptr);

    /*!
     * \brief Destroys the %AccountEditor controller.
     */
    ~AccountEditor();

    /*!
     * \brief Base action for chained routes in the account namespace.
     *
     * As the chain base for the \a /account routing namespace, this requires
     * to get the \a domainId and \a accountId database IDs in the path to query
     * the correct domain and account.
     *
     * If neither the \a domainId nor the \a accountId or both could not be found,
     * the HTTP response status code will be set to 404 and the \link tmpl-404 404.html \endlink
     * template file will be rendered.
     *
     * This method also checks if the current user has access rights to the domain.
     * If access is not granted, the HTTP response status code will be set to 403
     * and the \link tmpl-403 403.html \endlink template file wille be rendered.
     *
     * If access is granted, the requested Domain and Account objects are put into
     * the stash as \a domain and \a account.
     *
     * \cationchainstart{account,2}
     *
     * \param c         pointer to the current context
     * \param domainId  database ID of the Domain, has to be convertible into a dbid_t
     * \param accountId database ID of the Account, has to be convertible into a dbid_t
     */
    C_ATTR(base, :Chained("/") :PathPart("account") :CaptureArgs(2))
    void base(Context *c, const QString &domainId, const QString &accountId);
    
    /*!
     * \brief Chained route action to edit a single account.
     *
     * \cactionchainend{base,edit,0,/account/&lowast;/&lowast;/edit,\ref tmpl-account-edit,no}
     *
     * <H3>Accepted POST parameters</H3>
     * \includedoc account-edit-accepted-post.html
     */
    C_ATTR(edit, :Chained("base") :PathPart("edit") :Args(0))
    void edit(Context *c);
    
    /*!
     * \brief Chained route action to remove a single account.
     *
     * \cactionchainend{base,remove,0,/account/&lowast;/&lowast;/remove,\ref tmpl-account-remove,yes}
     *
     * If this receives a non-AJAX POST request and the removal of the account was successful,
     * the repsonse will redirect the user to \link DomainEditor::accounts() /domain/&lowast;/accounts\endlink.
     *
     * <H3>AJAX POST requests</H3>
     * This route can be used with AJAX POST requests to remove an account without reloading the page. This
     * can for example be used, to directly remove an account from a list of accounts without reloading the
     * page. See \ref templating_ajax to learn more about using AJAX requests in your templates.
     *
     * <H4>Example response for a successful AJAX request</H4>
     * \include account-remove-response-json-success.json
     *
     * <H4>Example repsonse for a failed AJAX request</H4>
     * \include account-remove-response-json-failed.json
     *
     * <H3>Accepted POST parameters</H3>
     * \includedoc account-remove-accepted-post.html
     *
     * <H3>Repsonse Status Codes</H3>
     * \includedoc account-remove-response-status.html
     */
    C_ATTR(remove, :Chained("base") :PathPart("remove") :Args(0))
    void remove(Context *c);

    /*!
     * \brief Chained route action to list email addresses for a single account.
     *
     * \cactionchainend{base,addresses,0,/account/&lowast;/&lowast;/addresses,\ref tmpl-account-addresses,no}
     */
    C_ATTR(addresses, :Chained("base") :PathPart("addresses") :Args(0))
    void addresses(Context *c);

    /*!
     * \brief Chained route action to edit a single email address for a single account.
     *
     * \cactionchainend{base,edit_address,1,/account/&lowast;/&lowast;/edit_address/&lowast;,\ref tmpl-account-edit_address,yes}
     */
    C_ATTR(edit_address, :Chained("base") :PathPart("edit_address") :Args(1))
    void edit_address(Context *c, const QString &address);

    /*!
     * \brief Chained route action to remove a single email address from a single account.
     *
     * \cactionchainend{base,remove_address,1,/account/&lowast;/&lowast;/remove_address/&lowast;,\ref tmpl-account-remove_address,yes}
     */
    C_ATTR(remove_address, :Chained("base") :PathPart("remove_address") :Args(1))
    void remove_address(Context *c, const QString &address);

    /*!
     * \brief Chained route action to add a new email address to a single account.
     *
     * \cactionchainend{base,add_address,0,/account/&lowast;/&lowast;/add_address,\ref tmpl-account-add_address,yes}
     */
    C_ATTR(add_address, :Chained("base") :PathPart("add_address") :Args(0))
    void add_address(Context *c);

    /*!
     * \brief Chained route action to list forward addresses for a single account.
     *
     * \cactionchainend{base,forwards,0,/account/&lowast;/&lowast;/forwards,\ref tmpl-account-forwards,no}
     */
    C_ATTR(forwards, :Chained("base") :PathPart("forwards") :Args(0))
    void forwards(Context *c);

    /*!
     * \brief Chained route action to remove a single forward address from a single account.
     *
     * \cactionchainend{base,remove_forward,1,/account/&lowast;/&lowast;/remove_forward/&lowast;,\ref tmpl-account-remove_forward,yes}
     */
    C_ATTR(remove_forward, :Chained("base") :PathPart("remove_forward") :Args(1))
    void remove_forward(Context *c, const QString &forward);

    /*!
     * \brief Chained route action to add a new forward address to a single account.
     *
     * \cactionchainend{base,add_forward,0,/account/&lowast;/&lowast;/add_forward,\ref tmpl-account-add_forward,yes}
     */
    C_ATTR(add_forward, :Chained("base") :PathPart("add_forward") :Args(0))
    void add_forward(Context *c);

    /*!
     * \brief Chained route action to edit a single forward address for a single account.
     *
     * \cactionchainend{base,edit_forward,1,/account/&lowast;/&lowast;/edit_forward/&lowast;,\ref tmpl-account-edit_forward,yes}
     */
    C_ATTR(edit_forward, :Chained("base") :PathPart("edit_forward") :Args(1))
    void edit_forward(Context *c, const QString &oldForward);

    /*!
     * \brief Chained route action to change if forwarded messages should be kept in the local mailbox.
     *
     * \cactionchainend{base,keep_local,0,/account/&lowast;/&lowast;/keep_local;,\ref tmpl-account-keep_local,yes}
     */
    C_ATTR(keep_local, :Chained("base") :PathPart("keep_local") :Args(0))
    void keep_local(Context *c);

    /*!
     * \brief Chained route action to check a single account.
     *
     * \cactionchainend{base,check,0,/account/&lowast;/&lowast;/check;,\ref tmpl-account-check,yes}
     */
    C_ATTR(check, :Chained("base") :PathPart("check") :Args(0))
    void check(Context *c);

    /*!
     * \brief Route action to return a JSON array of basic account information.
     *
     * \clocalaction{0,/account/list,none,only}
     */
    C_ATTR(list, :Local("list") :Args(0))
    void list(Context *c);
};

#endif //ACCOUNTEDITOR_H

