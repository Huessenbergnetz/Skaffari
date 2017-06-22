function skaffariRemoveAddress(e) {
    var btn = $(e.target);
    var quest = $('#addressesTable').data('removequestion');
    var address = btn.data('address');
    quest = quest.replace("$1", address)
    if (confirm(quest)) {
        $('.alert').alert('close');
        btn.prop('disabled', true);
        var icon = btn.children().first();
        icon.removeClass('fa-trash');
        icon.addClass('fa-cog fa-spin');

        $.ajax({
            type: 'post',
            url: '/account/' + btn.data('domainid') + '/' + btn.data('accountid') + '/remove_email/' + address,
            data: {email: address},
            dataType: 'json'
        }).always(function() {
            icon.removeClass('fa-cog fa-spin');
            icon.addClass('fa-trash');
        }).done(function(data) {
            if (($('#addressesTableBody tr').length - 1) <= 1) {
                $('.remove-address-btn').prop('disabled', true);
            }
            var tr = btn.parents('tr').first();
            tr.hide(300, function() {
                tr.remove();
            });
        }).fail(function(jqXHR) {
            btn.prop('disabled', false);
            $('#messages-container').append('<div class="row"><div class="col"><div class="alert alert-warning alert-dismissible fade show mt-3" role="alert"><button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button>' + jqXHR.responseJSON.error_msg  + '</div></div></div>');
            $('.alert').alert();
        });
    }
}

function skaffariEditAddress(e) {
    var btn = $(e.target);
    $('#addressForm').attr('action', '/account/' + btn.data('domainid') + '/' + btn.data('accountid') + '/email/' + btn.data('address'));
    var parts = btn.data('address').split('@');
    $('#newlocalpart').val(parts[0]);
    $('#newmaildomain').val(parts[1]);
    var am = $('#addressModal');
    am.data('actiontype', 'edit');
    am.modal('show');
}

function skaffariAddAddress(e) {
    var btn = $(e.target);
    $('#addressForm').attr('action', btn.data('addaction'));
    var am = $('#addressModal');
    am.data('actiontype', 'add');
    am.modal('show');
}

$(function() {
    var addressModal = $('#addressModal');
    
    if (addressModal.length > 0) {
        var addressForm = $('#addressForm');
        
        addressModal.on('show.bs.modal', function(e) {
            $('button#addAddressButton').prop('disabled', true);
            $('button.edit-address-btn').prop('disabled', true);
            var at = addressModal.data('actiontype');
            var mt = $('#addressModalLabel');
            var ast = $('#addressSubmitText')
            if (at === "add") {
                mt.text(mt.data('addlabel'));
                ast.text(ast.data('addlabel'));
            } else if (at === "edit") {
                mt.text(mt.data('editlabel'));
                ast.text(ast.data('editlabel'));
            }
        });
        
        addressModal.on('hide.bs.modal', function(e) {
            $('button#addAddressButton').prop('disabled', false);
            $('button.edit-address-btn').prop('disabled', false);
            $('#modal-message-container .alert').alert('close');
        });
        
        $('button.edit-address-btn').click(skaffariEditAddress);
        $('button.remove-address-btn').click(skaffariRemoveAddress);
        $('#addAddressButton').click(skaffariAddAddress);
        
        addressForm.submit(function(e) {
            $('#modal-message-container .alert').alert('close');
            $('#addressSubmit').prop('disabled', true);
            var aasi = $('#addressSubmitIcon');
            aasi.removeClass('fa-save');
            aasi.addClass('fa-circle-o-notch fa-spin');
            
            $.ajax({
                type: 'post',
                url: addressForm.attr('action'),
                data: addressForm.serialize(),
                dataType: 'json'
            }).always(function(data) {
                
                aasi.removeClass('fa-circle-o-notch fa-spin');
                aasi.addClass('fa-save');
                $('#addressSubmit').prop('disabled', false);
                
            }).done(function(data) {
                
                $('#addressModal').modal('hide');
                $('#newlocalpart').val('');
                
                var actionType = addressModal.data('actiontype');
                
                if (actionType === "add") {
                    var newAddress = data.address;

                    var newRow = $('<tr>');
                    var td1 = $('<td>');

                    var btnGrp = $('<div>');
                    btnGrp.addClass('btn-group btn-group-sm');
                    btnGrp.attr({role: "group", "aria-label": data.actions_btn_label});

                    var btn1 = $('<button>');
                    btn1.attr({type: 'button', title: data.edit_btn_label, "data-address": newAddress});
                    btn1.addClass('btn btn-primary edit-address-btn');
                    btn1.data({address: newAddress, domainid: data.domain_id, accountid: data.account_id});
                    btn1.append('<i class="fa fa-edit fa-fw"></i>');
                    btn1.click(skaffariEditAddress);
                    btnGrp.append(btn1);

                    var btn2 = $('<button>');
                    btn2.attr({type: 'button', title: data.delete_btn_label});
                    btn2.addClass('btn btn-danger remove-address-btn');
                    btn2.data({address: newAddress, domainid: data.domain_id, accountid: data.account_id});
                    btn2.append('<i class="fa fa-trash fa-fw"></i>');
                    btn2.click(skaffariRemoveAddress);
                    btnGrp.append(btn2);

                    td1.append(btnGrp);
                    newRow.append(td1);

                    var td2 = $('<td>');
                    td2.append(newAddress);
                    newRow.append(td2);

                    newRow.hide();

                    var added = false;
                    $('#addressesTableBody tr').each(function() {
                        var curRow = $(this);
                        var curAdr = curRow.children('td').last().text();
                        if (newAddress.localeCompare(curAdr) <= 0) {
                            curRow.before(newRow);
                            newRow.show(300);
                            added = true;
                            return false;
                        }
                    });
                    
                    if (!added) {
                        $('#addressesTableBody').append(newRow);
                        newRow.show(300);
                    }

                    if (($('#addressesTableBody tr').length - 1) <= 1) {
                        $('.remove-address-btn').prop('disabled', false);
                    }
                    
                } else if (actionType === "edit") {
                    var oa = data.old_address;
                    var na = data.new_address;
                    var orgBtn = $('button[data-address="' + oa + '"]');
                    orgBtn.data('address', na);
                    orgBtn.attr('data-address', na);
                    var row = orgBtn.parents('tr').first();
                    var td = row.children('td').last();
                    td.text(na);
                }
                
            }).fail(function(jqXHR) {
                if (jqXHR.responseJSON.error_msg) {
                    var warnDiv = $('<div>');
                    warnDiv.addClass('alert alert-warning alert-dismissible fade show mt-1');
                    warnDiv.attr('role', 'alert');
                    var warnDivCb = $('<button>');
                    warnDivCb.attr({type: "button", "aria-label": "Close"});
                    warnDivCb.addClass('close');
                    warnDivCb.data('dismiss', 'alert');
                    warnDivCb.click(function() {
                        warnDiv.alert('close');
                    });
                    var warnDivSpan = $('<span>');
                    warnDivSpan.attr('aria-hidden', 'true');
                    warnDivSpan.html('&times;')
                    warnDivCb.append(warnDivSpan);
                    warnDiv.append(warnDivCb);
                    warnDiv.append(jqXHR.responseJSON.error_msg);
                    warnDiv.hide();
                    $('#modal-message-container').append(warnDiv);
                    warnDiv.show(300);
                    warnDiv.alert();
                }
            });
            
            e.preventDefault();
        });
    }
});
