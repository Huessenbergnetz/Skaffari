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

$(function() {
    var removeAccountModal = $('#removeAccountModal');
    
    if (removeAccountModal.length > 0) {
        
        var removeAccountName = $('#removeAccountName');
        var removeAccountForm = $('#removeAccountForm');
        var removeAccountIcon = $('#removeAccountIcon');
        var removeAccountSubmit = $('#removeAccountSubmit');
        
        removeAccountModal.on('show.bs.modal', function(e) {
            var btn = $(e.relatedTarget);
            removeAccountName.text(btn.data('username'));
            removeAccountForm.attr('action', '/account/' + btn.data('domainid') + '/' + btn.data('accountid') + '/remove');
            $('#removeAccountForm #accountName').val('');
            $('#remove-account-message-container').empty();
            $('button.remove-account-btn').prop('disabled', true);
        });
        
        removeAccountModal.on('hide.bs.modal', function(e) {
            $('button.remove-account-btn').prop('disabled', false);
        });
        
        removeAccountForm.submit(function(e) {
            $('#removeAccountForm #accountName').blur();
            $('#remove-account-message-container .alert').alert('close');
            removeAccountSubmit.prop('disabled', true);
            removeAccountIcon.removeClass('fa-trash');
            removeAccountIcon.addClass('fa-circle-o-notch fa-spin');
            
            $.ajax({
                method: 'post',
                url: removeAccountForm.attr('action'),
                data: removeAccountForm.serialize(),
                dataType: 'json'
            }).always(function() {
                removeAccountIcon.removeClass('fa-circle-o-notch fa-spin');
                removeAccountIcon.addClass('fa-trash');
                removeAccountSubmit.prop('disabled', false);
            }).done(function(data) {
                removeAccountModal.modal('hide');
                var row = $('#account-' + data.domain_id + '-' + data.account_id);
                row.hide(300, function() {
                    row.remove();
                });
            }).fail(function(jqXHR) {
                if (jqXHR.responseJSON.error_msg) {
                    skaffariCreateAlert('warning', jqXHR.responseJSON.error_msg, '#remove-account-message-container', 'mt-1');
                }
            });
            e.preventDefault();
        });
    }
});
