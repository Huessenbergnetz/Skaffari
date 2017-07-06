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

function skaffariCheckAccount() {
    var cam = $('#checkAccountModal');
    var domainid = cam.data('domainid');
    var accountid = cam.data('accountid');
    var checkAccountList = $('#checkAccountList');
    checkAccountList.empty();
    var checkAccountActive = $('#checkAccountActive');
    checkAccountActive.show();
    var checkAccountSubmit = $('#checkAccountSubmit');
    checkAccountSubmit.prop('disabled', true);
    $('#check-account-message-container').empty();
    
    $.ajax({
        method: 'get',
        url: '/account/' + domainid + '/' + accountid + '/check',
        dataType: 'json'
    }).always(function(data) {
        checkAccountActive.hide();
        checkAccountSubmit.prop('disabled', false);
        console.log(data);
    }).done(function(data) {
        if (data.actions) {
            var actions = data.actions;
            var actionsLength = actions.length;
            for (i = 0; i < actionsLength; ++i) {
                var li = $('<li>');
                li.text(actions[i]);
                checkAccountList.append(li);
            }
        } else {
            skaffariCreateAlert('success', data.status_msg, '#check-account-message-container');
        }
    }).fail(function(jqXHR) {
        if (jqXHR.responseJSON.error_msg) {
            skaffariCreateAlert('warning', jqXHR.responseJSON.error_msg, '#check-account-message-container');
        }
    });
}

$(function() {
    var checkAccountModal = $('#checkAccountModal');
    
    if (checkAccountModal.length > 0) {
        
        $('#checkAccountSubmit').click(function() {
            skaffariCheckAccount();
        });
        
        checkAccountModal.on('show.bs.modal', function(e) {
            $('.check-account-btn').prop('disabled', true);
            var btn = $(e.relatedTarget);
            var accountRow = btn.parents('.account-row').first();
            $('#checkAccountName').text(accountRow.data('username'));
            var did = accountRow.data('domainid');
            var aid = accountRow.data('accountid');
            checkAccountModal.data({
                domainid: did,
                accountid: aid
            });
            skaffariCheckAccount();
        });
        
        checkAccountModal.on('hide.bs.modal', function() {
            $('.check-account-btn').prop('disabled', false);
        });
    }
});
