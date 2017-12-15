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

Skaffari.DefaultTmpl.DomainList = Skaffari.DefaultTmpl.DomainList || {};

Skaffari.DefaultTmpl.DomainList.domainTable = $('#domainTable');

Skaffari.DefaultTmpl.DomainList.removeDomain = function() {
    var removeDomainSubmit = $('#removeDomainSubmit');
    var removeDomainIcon = $('#removeDomainIcon');

    $('#removeDomainForm #domainName').blur();
    $('#remove-domain-message-container .alert').alert('close');
    removeDomainSubmit.prop('disabled', true);
    removeDomainIcon.removeClass('fa-trash');
    removeDomainIcon.addClass('fa-circle-o-notch fa-spin');

    $.ajax({
        method: 'post',
        url: Skaffari.DefaultTmpl.DomainList.removeDomainForm.attr('action'),
        data: Skaffari.DefaultTmpl.DomainList.removeDomainForm.serialize(),
        dataType: 'json'
    }).always(function() {
        removeDomainIcon.removeClass('fa-circle-o-notch fa-spin');
        removeDomainIcon.addClass('fa-trash');
        removeDomainSubmit.prop('disabled', false);
    }).done(function(data) {
        Skaffari.DefaultTmpl.DomainList.removeDomainModal.modal('hide');
        var row = $('#domain-' + data.deleted_id);
        row.hide(300, function() {
            row.remove();
        })
    }).fail(function(jqXHR) {
        if (jqXHR.responseJSON.error_msg) {
            Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, '#remove-domain-message-container', 'mt-1');
        }
    });
}

Skaffari.DefaultTmpl.DomainList.init = function() {
    if (Skaffari.DefaultTmpl.DomainList.domainTable.length > 0) {
        var stupidDomainTable = Skaffari.DefaultTmpl.DomainList.domainTable.stupidtable();
        stupidDomainTable.bind('aftertablesort', function(event, data) {
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

        Skaffari.DefaultTmpl.DomainList.domainTable.filterTable({inputSelector: "#domainTableFilter", ignoreClass: "no-filtering"});

        Skaffari.DefaultTmpl.DomainList.removeDomainModal = $('#removeDomainModal');

        if (Skaffari.DefaultTmpl.DomainList.removeDomainModal.length > 0) {

            Skaffari.DefaultTmpl.DomainList.removeDomainForm = $('#removeDomainForm');

            Skaffari.DefaultTmpl.DomainList.removeDomainModal.on('show.bs.modal', function(e) {
                var btn = $(e.relatedTarget);
                $('#removeDomainName').text(btn.data('name'));
                Skaffari.DefaultTmpl.DomainList.removeDomainForm.attr('action', '/domain/' + btn.data('domainid') + '/remove');
                $('#removeDomainForm #domainName').val('');
                $('#remove-domain-message-container').empty();
                $('#accountCountDelete').text(btn.data('accountcount'));
                $('.remove-domain-btn').prop('disabled', true);
            });

            Skaffari.DefaultTmpl.DomainList.removeDomainModal.on('hide.bs.modal', function(e) {
                $('.remove-domain-btn').prop('disabled', false);
            });

            Skaffari.DefaultTmpl.DomainList.removeDomainForm.submit(function(e) {
                Skaffari.DefaultTmpl.DomainList.removeDomain();
                e.preventDefault();
            });
        }
    }
}

$(function() {
    Skaffari.DefaultTmpl.DomainList.init();
});
