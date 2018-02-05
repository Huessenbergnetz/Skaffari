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

Skaffari.DefaultTmpl.AddressList = Skaffari.DefaultTmpl.AddressList || {};

Skaffari.DefaultTmpl.AddressList.addressTable = $('#addressesTable');

Skaffari.DefaultTmpl.AddressList.addAddressRow = function(d) {
    var tr = Skaffari.DefaultTmpl.AddressList.addressRowTemplate.content.querySelector("tr");
    tr.dataset.address = d.address;

    var td = Skaffari.DefaultTmpl.AddressList.addressRowTemplate.content.querySelectorAll("td");
    td[1].textContent = d.address;

    var clone = document.importNode(Skaffari.DefaultTmpl.AddressList.addressRowTemplate.content, true);

    var tb = document.getElementById('addressesTableBody');
    tb.appendChild(clone);

    var removeButton = tb.querySelectorAll('.remove-address-btn');
    removeButton[removeButton.length-1].addEventListener("click", Skaffari.DefaultTmpl.AddressList.removeAddressRow);
}

Skaffari.DefaultTmpl.AddressList.removeAddressRow = function(e) {
    var btn = $(e.target);
    var row = btn.parents('tr').first();
    var address = row.data('address');
    if (confirm($.i18n('sk-def-tmpl-addressremove-question', address))) {
        $('.alert').alert('close');
        btn.prop('disabled', true);
        var icon = btn.children().first();
        icon.removeClass('fa-trash');
        icon.addClass('fa-cog fa-spin');

        $.ajax({
            type: 'post',
            url: '/account/' + Skaffari.DefaultTmpl.AddressList.domainId + '/' + Skaffari.DefaultTmpl.AddressList.accountId + '/remove_address/' + encodeURIComponent(address).replace(".", "%2E"),
            data: {email: address},
            dataType: 'json'
        }).always(function() {
            icon.removeClass('fa-cog fa-spin');
            icon.addClass('fa-trash');
        }).done(function(data) {
            if (data.address_count <= 1) {
                $('.remove-address-btn').prop('disabled', true);
            }
            row.hide(300, function() {
                row.remove();
            });
        }).fail(function(jqXHR) {
            btn.prop('disabled', false);
            Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, '#messages-container', 'mt-1');
        });
    }
}

Skaffari.DefaultTmpl.AddressList.editAddressRow = function(d) {
    var orgRow = $('tr[data-address="' + d.old_address + '"]');
    var na = d.new_address;
    orgRow.attr('data-address', na);
    orgRow.data('address', na);
    orgRow.children('td').last().text(na);
}

Skaffari.DefaultTmpl.AddressList.init = function() {
    if (Skaffari.DefaultTmpl.AddressList.addressTable.length > 0) {
        Skaffari.DefaultTmpl.AddressList.domainId = Skaffari.DefaultTmpl.AddressList.addressTable.data('domainid');
        Skaffari.DefaultTmpl.AddressList.accountId = Skaffari.DefaultTmpl.AddressList.addressTable.data('accountid');
        Skaffari.DefaultTmpl.AddressList.addressModal = $('#addressModal');
        Skaffari.DefaultTmpl.AddressList.addressForm = $('#addressForm');
        Skaffari.DefaultTmpl.AddressList.localInput = $('#newlocalpart');
        Skaffari.DefaultTmpl.AddressList.domainInput = $('#newmaildomain');
        Skaffari.DefaultTmpl.AddressList.submitIcon = $('#addressSubmitIcon');
        Skaffari.DefaultTmpl.AddressList.addressRowTemplate = document.getElementById('address-template');

        $('button.remove-address-btn').click(Skaffari.DefaultTmpl.AddressList.removeAddressRow);

        Skaffari.DefaultTmpl.AddressList.addressModal.on('show.bs.modal', function(e) {
            var btn = $(e.relatedTarget);
            Skaffari.DefaultTmpl.AddressList.action = btn.data('actiontype');
            var aml = $('#addressModalLabel');
            var ast = $('#addressSubmitText')
            $('#addbuttons button').prop('disabled', true);
            $('.edit-address-btn').prop('disabled', true);
            if (Skaffari.DefaultTmpl.AddressList.action == "add") {
                aml.text($.i18n('sk-def-tmpl-addressmodal-add'));
                ast.text($.i18n('sk-def-tmpl-submitbtn-add'));
                Skaffari.DefaultTmpl.AddressList.actionRoute = Skaffari.DefaultTmpl.AddressList.addressForm.data('addaction');
                Skaffari.DefaultTmpl.AddressList.localInput.val('');
            } else {
                aml.text($.i18n('sk-def-tmpl-addressmodal-edit'));
                ast.text($.i18n('sk-def-tmpl-submitbtn-change'));
                var address  = btn.parents('tr').first().data('address');
                Skaffari.DefaultTmpl.AddressList.actionRoute =  '/account/' + Skaffari.DefaultTmpl.AddressList.domainId + '/' + Skaffari.DefaultTmpl.AddressList.accountId + '/edit_address/' + encodeURIComponent(address).replace(".", "%2E");
                var parts = address.split('@');
                Skaffari.DefaultTmpl.AddressList.localInput.val(parts[0]);
                var opts = Skaffari.DefaultTmpl.AddressList.domainInput.children('option');
                var optsLength = opts.length;
                if (optsLength > 0) {
                    var domainPart = parts[1];
                    var atDomainPart = '@' + domainPart;
                    var selVal = 0;
                    for (var i = 0; i < optsLength; ++i) {
                        if (opts[i].text === atDomainPart) {
                            selVal = parseInt(opts[i].value);
                            break;
                        }
                    }
                    if (selVal > 0) {
                        Skaffari.DefaultTmpl.AddressList.domainInput.val(selVal);
                    }
                }
            }
        });

        Skaffari.DefaultTmpl.AddressList.addressModal.on('hide.bs.modal', function(e) {
            $('#addbuttons button').prop('disabled', false);
            $('.edit-address-btn').prop('disabled', false);
            $('#modal-message-container .alert').alert('close');
        });

        Skaffari.DefaultTmpl.AddressList.addressForm.submit(function(e) {
            $('#addressSubmit').prop('disabled', true);
            $('#modal-message-container .alert').alert('close');
            Skaffari.DefaultTmpl.AddressList.submitIcon.removeClass('fa-save').addClass('fa-circle-notch fa-spin');

            $.ajax({
                type: 'post',
                url: Skaffari.DefaultTmpl.AddressList.actionRoute,
                data: Skaffari.DefaultTmpl.AddressList.addressForm.serialize(),
                dataType: 'json'
            }).always(function(data) {
                Skaffari.DefaultTmpl.AddressList.submitIcon.removeClass('fa-circle-notch fa-spin').addClass('fa-save');
                $('#addressSubmit').prop('disabled', false);
            }).done(function(data) {
                Skaffari.DefaultTmpl.AddressList.addressModal.modal('hide');
                if (Skaffari.DefaultTmpl.AddressList.action == "add") {
                    Skaffari.DefaultTmpl.AddressList.addAddressRow(data);
                } else {
                    Skaffari.DefaultTmpl.AddressList.editAddressRow(data);
                }
                if (data.address_count > 1) {
                    $('.remove-address-btn').prop('disabled', false);
                } else {
                    $('.remove-address-btn').prop('disabled', false);
                }
            }).fail(function(jqXHR) {
                if (jqXHR.responseJSON.error_msg) {
                    Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, '#modal-message-container', 'mt-1');
                } else if (jqXHR.responseJSON.field_errors) {
                    var fe = jqXHR.responseJSON.field_errors;
                    var errorStrings = [];
                    for (var key in fe) {
                        var fes = fe[key];
                        for (var i = 0; i < fes.length; i++) {
                            errorStrings.push(fes[i]);
                        }
                    }
                    Skaffari.DefaultTmpl.createAlert('warning', errorStrings, '#modal-message-container', 'mt-1');
                }
            });

            e.preventDefault();
        });
    }
}

$(function() {
    Skaffari.DefaultTmpl.AddressList.init();
});
