$(function() {
    var checkDomainRunning = false;
    $('#checkdomain').click(function() {
        var checkDom = $('#checkdomain');
        var domainId = checkDom.data('domainid');
        var accountIds = checkDom.data('accountids').split(',');
        var baseUrl = checkDom.data('baseurl');
        var ntdStr = checkDom.data('ntdstr');
        var idCount = accountIds.length;
        var infoBlock = $('#checkdomaininfo');
        
        if ((idCount > 0) && !checkDomainRunning) {
            checkDomainRunning = true;
            checkDom.prop('disabled', true);
            infoBlock.empty();
        
            var _qjax = new $.qjax({
                timeout: 10000,
                ajaxSettings: {
                    dataType: "json"
                },
                onQueueChange: function(length) {
                    var finishedCount = idCount - length;
                    var percentFinished = (finishedCount / idCount) * 100;
                    var cdp = $('#checkdomainprogress');
                    cdp.attr('aria-valuenow', finishedCount);
                    cdp.css('width', percentFinished + '%');
                    cdp.text(finishedCount + '/' + idCount);
                    if (length === 0) {
                        checkDom.prop('disabled', false);
                        checkDomainRunning = false;
                    }
                }
            });
            
            for (i = 0; i < idCount; ++i) {
                var ret = _qjax.Queue({
                    url: '/account/' + domainId +'/' + accountIds[i] + '/check',
                });
                ret.done(function(e) {
                    var info = '<div class="mt-3"><h3>' + e.account + '</h3>';
                    var actions = e.actions;
                    var al = actions.length;
                    if (al > 0) {
                        info += '<ul>';
                        for (i = 0; i < al; ++i) {
                            info += '<li>' + actions[i] + '</li>';
                        }
                        info += '</ul>';
                    } else {
                        info += '<p>' + ntdStr + '</p>';
                    }
                    info += '</div>';
                    infoBlock.append(info);
                });
            }
        }
    });
});
