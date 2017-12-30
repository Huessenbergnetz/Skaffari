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

Skaffari.DefaultTmpl.AdminList = Skaffari.DefaultTmpl.AdminList || {};

Skaffari.DefaultTmpl.AdminList.adminTable = $('#adminsTable');

Skaffari.DefaultTmpl.AdminList.removeAdmin = function() {
    var removeAdminSubmit = $('#removeAdminSubmit');
    var removeAdminIcon = $('#removeAdminIcon');
    $('#removeAdminForm #adminName').blur();
    $('#remove-admin-message-container .alert').alert('close');
    removeAdminSubmit.prop('disabled', true);
    removeAdminIcon.removeClass('fa-trash');
    removeAdminIcon.addClass('fa-circle-notch fa-spin');

    $.ajax({
        method: 'post',
        url: Skaffari.DefaultTmpl.AdminList.removeAdminForm.attr('action'),
        data: Skaffari.DefaultTmpl.AdminList.removeAdminForm.serialize(),
        dataType: 'json'
    }).always(function() {
        removeAdminIcon.removeClass('fa-circle-notch fa-spin');
        removeAdminIcon.addClass('fa-trash');
        removeAdminSubmit.prop('disabled', false);
    }).done(function(data) {
        Skaffari.DefaultTmpl.AdminList.removeAdminModal.modal('hide');
        var row = $('tr#admin-' + data.admin_id);
        row.hide(300, function() {
            row.remove();
        });
    }).fail(function(jqXHR) {
        if (jqXHR.responseJSON.error_msg) {
            Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, '#remove-admin-message-container', 'mt-1');
        }
    });
}

Skaffari.DefaultTmpl.AdminList.init = function() {
    Skaffari.DefaultTmpl.AdminList.removeAdminModal = $('#removeAdminModal');
    if (Skaffari.DefaultTmpl.AdminList.removeAdminModal.length > 0) {
        Skaffari.DefaultTmpl.AdminList.removeAdminForm = $('#removeAdminForm');

        Skaffari.DefaultTmpl.AdminList.removeAdminModal.on('show.bs.modal', function(e) {
            var btn = $(e.relatedTarget);
            $('#removeAdminName').text(btn.data('username'));
            Skaffari.DefaultTmpl.AdminList.removeAdminForm.attr('action', '/admin/' + btn.data('adminid') + '/remove');
            $('#removeAdminForm #adminName').val('');
            $('#remove-admin-message-container').empty();
            $('.remove-admin-btn').prop('disabled', true);
        });

        Skaffari.DefaultTmpl.AdminList.removeAdminModal.on('hide.bs.modal', function(e) {
            $('.remove-admin-btn').prop('disabled', false);
        });

        Skaffari.DefaultTmpl.AdminList.removeAdminForm.submit(function(e) {
            Skaffari.DefaultTmpl.AdminList.removeAdmin();
            e.preventDefault();
        });
    }
}

$(function() {
    Skaffari.DefaultTmpl.AdminList.init();
});
