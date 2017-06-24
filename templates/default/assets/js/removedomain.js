$(function() {
    var removeDomainModal = $('#removeDomainModal');
    
    if (removeDomainModal.length > 0) {
        
        var removeDomainForm = $('#removeDomainForm');
        var removeDomainIcon = $('#removeDomainIcon');
        var removeDomainName = $('#removeDomainName');
        var removeDomainSubmit = $('#removeDomainSubmit')
        
        removeDomainModal.on('show.bs.modal', function(e) {
            var btn = $(e.relatedTarget);
            removeDomainName.text(btn.data('name'));
            removeDomainForm.attr('action', '/domain/' + btn.data('domainid') + '/remove');
            $('#removeDomainForm #domainName').val('');
            $('#remove-domain-message-container').empty();
            $('#accountCountDelete').text(btn.data('accountcount'));
            $('button.remove-domain-btn').prop('disabled', true);
        });
        
        removeDomainModal.on('hide.bs.modal', function(e) {
            $('button.remove-domain-btn').prop('disabled', false);
        });
        
        removeDomainForm.submit(function(e) {
            $('#removeDomainForm #domainName').blur();
            $('#remove-domain-message-container .alert').alert('close');
            removeDomainSubmit.prop('disabled', true);
            removeDomainIcon.removeClass('fa-trash');
            removeDomainIcon.addClass('fa-circle-o-notch fa-spin');
            
            $.ajax({
                method: 'post',
                url: removeDomainForm.attr('action'),
                data: removeDomainForm.serialize(),
                dataType: 'json'
            }).always(function() {
                removeDomainIcon.removeClass('fa-circle-o-notch fa-spin');
                removeDomainIcon.addClass('fa-trash');
                removeDomainSubmit.prop('disabled', false);
            }).done(function(data) {
                removeDomainModal.modal('hide');
                var row = $('#domain-' + data.deleted_id);
                row.hide(300, function() {
                    row.remove();
                });
            }).fail(function(jqXHR) {
                if (jqXHR.responseJSON.error_msg) {
                    skaffariCreateAlert('warning', jqXHR.responseJSON.error_msg, '#remove-domain-message-container', 'mt-1');
                }
            });
            e.preventDefault();
        });
    }
});
