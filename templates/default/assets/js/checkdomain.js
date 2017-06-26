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
                    if (actions) {
                        var al = actions.length;
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
