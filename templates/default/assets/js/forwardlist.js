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

Skaffari.DefaultTmpl.ForwardList = Skaffari.DefaultTmpl.ForwardList || {};

Skaffari.DefaultTmpl.ForwardList.forwardsTable = $('#forwardsTable');

Skaffari.DefaultTmpl.ForwardList.addForwardRow = function(d) {
    var tr = Skaffari.DefaultTmpl.ForwardList.forwardRowTemplate.content.querySelector("tr");
    tr.dataset.forward = d.forward;

    var td = Skaffari.DefaultTmpl.ForwardList.forwardRowTemplate.content.querySelectorAll("td");
    td[1].textContent = d.forward;

    var clone = document.importNode(Skaffari.DefaultTmpl.ForwardList.forwardRowTemplate.content, true);

    var tb = document.getElementById('forwardsTableBody');
    tb.appendChild(clone);

    var removeButton = tb.querySelectorAll('.remove-forward-btn');
    removeButton[removeButton.length-1].addEventListener("click", Skaffari.DefaultTmpl.ForwardList.removeForwardRow);
}

Skaffari.DefaultTmpl.ForwardList.toggleKeepLocalBtn = function(show, keepLocal) {
    var isHidden = Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.hasClass('d-none');
    if (show && isHidden) {
        Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.removeClass('d-none');
    } else if (!show && !isHidden) {
        Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.addClass('d-none');
    }
    if (keepLocal) {
        Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.data('keeplocal', 'true');
        $('#keepLocalIcon').removeClass().addClass('far fa-check-square');
    } else {
        Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.data('keeplocal', 'false');
        $('#keepLocalIcon').removeClass().addClass('far fa-square');
    }
}

Skaffari.DefaultTmpl.ForwardList.toggleKeepLocal = function() {
    Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.prop('disabled', true);
    var keepLocal = (Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.data('keeplocal') == 'true');
    var icon = $('#keepLocalIcon');
    icon.removeClass().addClass('fas fa-spinner fa-pulse');
    var klstr = !keepLocal ? "true" : "false";

    $.ajax({
        type: 'post',
        url: '/account/' + Skaffari.DefaultTmpl.ForwardList.domainId + '/' + Skaffari.DefaultTmpl.ForwardList.accountId + '/keep_local',
        data: { keeplocal: klstr },
        dataType: 'json'
    }).always(function(data) {
        Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.prop('disabled', false);
    }).done(function(data) {
        Skaffari.DefaultTmpl.ForwardList.toggleKeepLocalBtn(true, data.keep_local);
    }).fail(function(jqXHR) {
        Skaffari.DefaultTmpl.ForwardList.toggleKeepLocalBtn(true, keepLocal);
        if (jqXHR.responseJSON.error_msg) {
            Skaffari.DefaultTmpl.createAlert('warning', jqXHR.response.error_msg, '#messages-container', 'mt-1');
        }
    });
}

Skaffari.DefaultTmpl.ForwardList.removeForwardRow = function(e) {
    var btn = $(e.target);
    var row = btn.parents('tr').first();
    var quest = $('#forwardsTable').data('removequestion');
    var forward = row.data('forward');
    quest = quest.replace("$1", forward);
    if (confirm(quest)) {
        $('.alert').alert('close');
        btn.prop('disabled', true);
        var icon = btn.children().first();
        icon.removeClass('fa-trash').addClass('fa-cog fa-spin');

        $.ajax({
            type: 'post',
            url: '/account/' + Skaffari.DefaultTmpl.ForwardList.domainId + '/' + Skaffari.DefaultTmpl.ForwardList.accountId + '/remove_forward/' + encodeURIComponent(forward).replace(".", "%2E"),
            data: {email: forward},
            dataType: 'json'
        }).done(function(data) {
            row.hide(300, function()  {
                row.remove();
            });
            Skaffari.DefaultTmpl.ForwardList.toggleKeepLocalBtn(data.forward_count > 0, data.keep_local);
        }).fail(function(jqXHR) {
            icon.removeClass('fa-cog fa-spin').addClass('fa-trash');
            btn.prop('disabled', false);
            if (jqXHR.responseJSON.error_msg) {
                Skaffari.DefaultTmpl.createAlert('warning', jqXHR.response.error_msg, '#messages-container', 'mt-1');
            }
        });
    }
}

Skaffari.DefaultTmpl.ForwardList.editForwardRow = function(d) {
    var row = $('tr[data-forward="' + d.old_forward + '"]');
    row.attr('data-forward', d.new_forward);
    row.data('forward', d.new_forward);
    row.children('td').last().text(d.new_forward);
}

Skaffari.DefaultTmpl.ForwardList.init = function() {
    if (Skaffari.DefaultTmpl.ForwardList.forwardsTable.length > 0) {
        Skaffari.DefaultTmpl.ForwardList.domainId = Skaffari.DefaultTmpl.ForwardList.forwardsTable.data('domainid');
        Skaffari.DefaultTmpl.ForwardList.accountId = Skaffari.DefaultTmpl.ForwardList.forwardsTable.data('accountid');
        Skaffari.DefaultTmpl.ForwardList.forwardModal = $('#forwardModal');
        Skaffari.DefaultTmpl.ForwardList.forwardForm = $('#forwardForm');
        Skaffari.DefaultTmpl.ForwardList.forwardInput = $('#newforward');
        Skaffari.DefaultTmpl.ForwardList.submitIcon = $('#forwardSubmitIcon');
        Skaffari.DefaultTmpl.ForwardList.keepLocalBtn = $('#keepLocalBtn');
        Skaffari.DefaultTmpl.ForwardList.forwardRowTemplate = document.getElementById('forward-template');

        $('button.remove-forward-btn').click(Skaffari.DefaultTmpl.ForwardList.removeForwardRow);

        Skaffari.DefaultTmpl.ForwardList.keepLocalBtn.click(Skaffari.DefaultTmpl.ForwardList.toggleKeepLocal);

        Skaffari.DefaultTmpl.ForwardList.forwardModal.on('show.bs.modal', function(e) {
            var btn = $(e.relatedTarget);
            Skaffari.DefaultTmpl.ForwardList.action = btn.data('actiontype');
            var fml = $('#forwardModalLabel');
            var fst = $('#forwardSubmitText');
            $('#addbuttons button').prop('disabled', true);
            $('.edit-forward-btn').prop('disabled', true);
            if (Skaffari.DefaultTmpl.ForwardList.action == "add") {
                fml.text(fml.data('addlabel'));
                fst.text(fst.data('addlabel'));
                Skaffari.DefaultTmpl.ForwardList.actionRoute = Skaffari.DefaultTmpl.ForwardList.forwardForm.data('addaction');
                Skaffari.DefaultTmpl.ForwardList.forwardInput.val('');
            } else {
                fml.text(fml.data('editlabel'));
                fst.text(fst.data('editlabel'));
                var forward = btn.parents('tr').first().data('forward');
                Skaffari.DefaultTmpl.ForwardList.actionRoute = '/account/' + Skaffari.DefaultTmpl.ForwardList.domainId + '/' + Skaffari.DefaultTmpl.ForwardList.accountId + '/edit_forward/' + encodeURIComponent(forward).replace(".", "%2E");
                Skaffari.DefaultTmpl.ForwardList.forwardInput.val(forward);
            }
        });

        Skaffari.DefaultTmpl.ForwardList.forwardModal.on('hide.bs.modal', function(e) {
            $('#addbuttons button').prop('disabled', false);
            $('.edit-forward-btn').prop('disabled', false);
            $('#modal-message-container .alert').alert('close');
        });

        Skaffari.DefaultTmpl.ForwardList.forwardForm.submit(function(e) {
            $('#forwardSubmit').prop('disabled', true);
            $('#modal-message-container .alert').alert('close');
            Skaffari.DefaultTmpl.ForwardList.submitIcon.removeClass('fa-save').addClass('fa-circle-notch fa-spin');

            $.ajax({
                type: 'post',
                url: Skaffari.DefaultTmpl.ForwardList.actionRoute,
                data: Skaffari.DefaultTmpl.ForwardList.forwardForm.serialize(),
                dataType: 'json'
            }).always(function(data) {
                Skaffari.DefaultTmpl.ForwardList.submitIcon.removeClass('fa-circle-notch fa-spin').addClass('fa-save');
                $('#forwardSubmit').prop('disabled', false);
            }).done(function(data) {
                Skaffari.DefaultTmpl.ForwardList.forwardModal.modal('hide');
                if (Skaffari.DefaultTmpl.ForwardList.action == "add") {
                    Skaffari.DefaultTmpl.ForwardList.addForwardRow(data);
                    Skaffari.DefaultTmpl.ForwardList.toggleKeepLocalBtn(data.forward_count > 0, data.keep_local);
                } else {
                    Skaffari.DefaultTmpl.ForwardList.editForwardRow(data);
                }
            }).fail(function(jqXHR) {
                if (jqXHR.responseJSON.error_msg) {
                    Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, '#modal-message-container', 'mt-1');
                }
            });

            e.preventDefault();
        });
    }
}

$(function() {
    Skaffari.DefaultTmpl.ForwardList.init();
})
