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

var Skaffari = Skaffari || {};

Skaffari.DefaultTmpl = Skaffari.DefaultTmpl || {};

Skaffari.DefaultTmpl.initSelect2AjaxAccounts = function() {
    $('.select2-ajax-accounts').select2({
        ajax: {
            url: '/account/list',
            dataType: 'json',
            delay: 250,
            data: function(params) {
                return {
                    searchString: params.term,
                    page: params.page,
                    accountsPerPage: 30
                };
            },
            processResults: function(data, params) {
                params.page = params.page || 1;

                return {
                    results: data.accounts,
                    pagination: {
                        more: false
                    }
                };
            },
            cache: true
        },
        escapeMarkup: function(markup) { return markup; },
        minimumInputLength: 3,
        templateResult: function(account) {
            return account.username + ' (' + account.domainname + ')';
        },
        templateSelection: function(account) {
            return account.text || account.username + ' (' + account.domainname + ')';
        },
        allowClear: true
    });
}

