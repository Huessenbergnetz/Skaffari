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
    var removeAdminModal = $('#removeAdminModal');
    
    if (removeAdminModal.length > 0) {
        var removeAdminName = $('#removeAdminName');
        var removeAdminForm = $('#removeAdminForm');
        var removeAdminIcon = $('#removeAdminIcon');
        var removeAdminSubmit = $('#removeAdminSubmit');
        
        removeAdminModal.on('show.bs.modal', function(e) {
            var btn = $(e.relatedTarget);
            removeAdminName.text(btn.data('username'));
            removeAdminForm.attr('action', '/admin/' + btn.data('adminid') + '/remove');
            $('#removeAdminForm #adminName').val('');
            $('#remove-admin-message-container').empty();
            $('button.remove-admin-btn').prop('disabled', true);
        });
        
        removeAdminModal.on('hide.bs.modal', function(e) {
            $('button.remove-admin-btn').prop('disabled', false);
        });
        
        removeAdminForm.submit(function(e) {
            $('#removeAdminForm #adminName').blur();
            $('#remove-admin-message-container .alert').alert('close');
            removeAdminSubmit.prop('disabled', true);
            removeAdminIcon.removeClass('fa-trash');
            removeAdminIcon.addClass('fa-circle-o-notch fa-spin');
            
            $.ajax({
                method: 'post',
                url: removeAdminForm.attr('action'),
                data: removeAdminForm.serialize(),
                dataType: 'json'
            }).always(function() {
                removeAdminIcon.removeClass('fa-circle-o-notch fa-spin');
                removeAdminIcon.addClass('fa-trash');
                removeAdminSubmit.prop('disabled', false);
            }).done(function(data) {
                removeAdminModal.modal('hide');
                var row = $('tr#admin-' + data.admin_id);
                row.hide(300, function() {
                    row.remove();
                });
            }).fail(function(jqXHR) {
                if (jqXHR.responseJSON.error_msg) {
                    skaffariCreateAlert('warning', jqXHR.responseJSON.error_msg, '#remove-admin-message-container', 'mt-1');
                }
            });
            e.preventDefault();
        });
    }
})
