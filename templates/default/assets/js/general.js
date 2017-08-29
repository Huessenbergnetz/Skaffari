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

// domain table sort and filter
$(function() {
    Skaffari.DefaultTmpl.init();
    Skaffari.DefaultTmpl.initSelect2AjaxAccounts();

    var domainTable = $('#domainTable').stupidtable();
    if (domainTable.length > 0) {
        domainTable.bind('aftertablesort', function(event, data) {
            $('#domainTable th > small').remove();
            var type = data.$th.data('sort');
            var sortType = '';
            if (type === "string" || type === "string-ins") {
                sortType = "alpha";
            } else {
                sortType = "amount"
            }
            data.$th.append(' <small><i class="fa fa-sort-' + sortType + '-' + data.direction + ' text-muted"></i></small>');
        });

        domainTable.filterTable({inputSelector: "#domainTableFilter", ignoreClass: "no-filtering"});
    }
});
