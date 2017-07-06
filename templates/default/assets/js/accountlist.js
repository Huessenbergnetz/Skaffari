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

Skaffari.DefaultTmpl.AccountList = Skaffari.DefaultTmpl.AccountList || {};

Skaffari.DefaultTmpl.AccountList.filterForm = $('#accountFilterForm');

Skaffari.DefaultTmpl.AccountList.createRow = function(a) {
    var l10n = Skaffari.DefaultTmpl.AccountList.l10n;
    var tr = $('<tr>');
    tr.addClass('account-row');
    tr.attr({
        id: 'account-' + a.domainId + '-' + a.id,
        "data-domainid": a.domainId,
        "data-accountid": a.id,
        "data-username": a.username
    });
    tr.data({
        domainid: a.domainId,
        accountid: a.id,
        username: a.username
    });
    
    /* Start action button dropdown */
    
    var actions = $('<td>');
    
    var btnGroup = $('<div>');
    btnGroup.addClass('btn-group');
    
    var mainBtn = $('<a>');
    mainBtn.addClass('btn btn-sm btn-secondary');
    mainBtn.attr({
        href: '/account/' + a.domainId + '/' + a.id + '/edit',
        title: l10n.editAccount,
        role: 'button'
    });
    var mainBtnIcon = $('<i>');
    mainBtnIcon.addClass('fa fa-edit fa-fw');
    mainBtn.append(mainBtnIcon);
    
    btnGroup.append(mainBtn);
    
    var toggleBtn = $('<button>');
    toggleBtn.addClass('btn btn-sm btn-secondary dropdown-toggle dropdown-toggle-split');
    toggleBtn.attr({
        type: "button",
        "data-toggle": "dropdown",
        "aria-haspopup": "true",
        "aria-expanded": "false"
    });
    toggleBtn.data('toggle', 'dropdown');
    var toggleBtnDesc = $('<span>');
    toggleBtnDesc.addClass('sr-only');
    toggleBtnDesc.text(l10n.toggleDropdown);
    toggleBtn.append(toggleBtnDesc);
    
    btnGroup.append(toggleBtn);
    
    var dropdownMenu = $('<div>');
    dropdownMenu.addClass('dropdown-menu');
    
    var addressesBtn = $('<a>');
    addressesBtn.addClass('dropdown-item');
    addressesBtn.attr('href', '/account/' + a.domainId + '/' + a.id + '/addresses');
    var addressesBtnIcon = $('<i>');
    addressesBtnIcon.addClass('fa fa-envelope-o fa-fw');
    addressesBtn.append(addressesBtnIcon);
    addressesBtn.append(' ' + l10n.emailAddresses);    
    dropdownMenu.append(addressesBtn);
    
    var forwardsBtn = $('<a>');
    forwardsBtn.addClass('dropdown-item');
    forwardsBtn.attr('href', '/account/' + a.domainId + '/' + a.id + '/forwards');
    var forwardsBtnIcon = $('<i>');
    forwardsBtnIcon.addClass('fa fa-mail-forward fa-fw');
    forwardsBtn.append(forwardsBtnIcon);
    forwardsBtn.append(' ' + l10n.forwards);
    dropdownMenu.append(forwardsBtn);
    
    var checkBtn = $('<a>');
    checkBtn.addClass('dropdown-item check-account-btn');
    checkBtn.attr({href: '#', 'data-target': '#checkAccountModal', 'data-toggle': 'modal'});
    checkBtn.data({
        target: '#checkAccountModal',
        toggle: 'modal'
    })
    var checkBtnIcon = $('<i>');
    checkBtnIcon.addClass('fa fa-stethoscope fa-fw');
    checkBtn.append(checkBtnIcon);
    checkBtn.append(' ' + l10n.checkAccount);
    dropdownMenu.append(checkBtn);
    
    var removeBtn = $('<a>');
    removeBtn.addClass('dropdown-item remove-account-btn text-danger');
    removeBtn.attr({
        href: '#',
        'data-target': '#removeAccountModal',
        'data-toggle': 'modal',
        'data-domainid': a.domainId,
        'data-accountid': a.id,
        'data-username': a.username
    });
    removeBtn.data({
        target: '#removeAccountModal',
        toggle: 'modal',
        domainid: a.domainId,
        accountid: a.id,
        username: a.username
    });
    var removeBtnIcon = $('<i>');
    removeBtnIcon.addClass('fa fa-trash fa-fw');
    removeBtn.append(removeBtnIcon);
    removeBtn.append(' ' + l10n.deleteAccount);
    dropdownMenu.append(removeBtn);
    
    btnGroup.append(dropdownMenu);
    
    actions.append(btnGroup);
    
    tr.append(actions);
    
    /* End action button dropdown */
    
    var username = $('<td>');
    var unLink = $('<a>');
    unLink.attr('href', '/account/' + a.domainId + '/' + a.id + '/addresses');
    unLink.attr('title', l10n.emailAddresses);
    unLink.text(a.username);
    username.append(unLink);
    tr.append(username);
    
    var addresses = $('<td>');
    if (a.catchAll) {
        addresses.append('<span class="text-warning">' + l10n.catchAll + '</span><br>')
    }
    addresses.append(a.addresses.join('<br>'));
    tr.append(addresses);
    
    var forwards = $('<td>');
    forwards.append(a.forwards.join('<br>'));
    if (a.forwards.length > 0) {
        forwards.append('<br>');
        var klIcon = $('<i>');
        if (a.keepLocal) {
            klIcon.addClass('fa fa-check-circle-o');
        } else {
            klIcon.addClass('fa fa-times-circle-o');
        }
                forwards.append(klIcon);
                forwards.append(' ' + l10n.keepLocal)
    }
    tr.append(forwards);
    
    var progress = $('<td>');
    var usagePercent = 0;
    if ((a.quota != 0) && (a.usage != 0)) {
        usagePercent = (a.usage / a.quota) * 100;
    }
    var progDiv = $('<div>');
    progDiv.addClass('progress');
    progDiv.attr('title', usagePercent.toFixed(2) + '%');
    
    var barDiv = $('<div>');
    barDiv.addClass('progress-bar bg-info pb-w-' + Math.round(usagePercent).toString());
    barDiv.attr({
        role: "progressbar",
        "aria-valuenow": a.usage,
        "aria-valuemin": 0,
        "aria-valuemax": a.quota,
        title: usagePercent.toFixed(2) + '%'
    });
    barDiv.text(usagePercent.toFixed(2) + '%');
    progDiv.append(barDiv);
    progress.append(progDiv);
    
    var humanUsage = Skaffari.DefaultTmpl.humanBinarySize(a.usage * 1024);
    var humanQuota = Skaffari.DefaultTmpl.humanBinarySize(a.quota * 1024);
    var progText = $('<p>');
    progText.addClass('text-right');
    var progTextSmall = $('<small>');
    progTextSmall.text(humanUsage + '/' + humanQuota);
    progText.append(progTextSmall);
    progress.append(progText);
    
    tr.append(progress);
    
    var times = $('<td>');
    var created = new Date(a.created);
    var updated = new Date(a.updated);
    var validUntil = new Date(a.validUntil);
    var passwordExpires = new Date(a.passwordExpires)
    var dateOptions = {
        year: "2-digit",
        month: "2-digit",
        day: "2-digit",
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit"
    };
    times.append(created.toLocaleString(undefined, dateOptions) + '<br>');
    times.append(updated.toLocaleString(undefined, dateOptions) + '<br>');
    var validUntilSpan = $('<span>')
    if (a.expired) {
        validUntilSpan.addClass('text-danger');
    }
    validUntilSpan.append(validUntil.toLocaleString(undefined, dateOptions) + '<br>');
    times.append(validUntilSpan);
    var pwExpirationSpan = $('<span>');
    if (a.passwordExpired) {
        pwExpirationSpan.addClass('text-danger');
    }
    pwExpirationSpan.append(passwordExpires.toLocaleString(undefined, dateOptions))
    times.append(pwExpirationSpan);
    tr.append(times);
    
    var services = $('<td>');
    var slist = ['imap', 'pop', 'sieve', 'smtpauth'];
    for (var i = 0; i < 4; i++) {
        var service = slist[i];
        var sOn = a[service];
        var sicon = $('<i>');
        sicon.addClass('fa fa-fw');
        if (sOn) {
            sicon.addClass('fa-check-square-o text-success');
        } else {
            sicon.addClass('fa-square-o text-danger');
        }
                services.append(sicon);
                switch(service) {
                    case "imap":
                        services.append(' IMAP<br>');
                        break;
                    case "pop":
                        services.append(' POP<br>');
                        break;
                    case "sieve":
                        services.append(' Sieve<br>');
                        break;
                    case "smtpauth":
                        services.append(' SMTP-Auth');
                        break;
                }
    }
    tr.append(services);
    
    return tr;
};


Skaffari.DefaultTmpl.AccountList.load = function(loadMore) {
    var aff = Skaffari.DefaultTmpl.AccountList.filterForm;
    if (aff.data('loading') == "0") {
        Skaffari.DefaultTmpl.clearMessages();
        var al = Skaffari.DefaultTmpl.AccountList;
        aff.data('loading', '1');
        if (!loadMore) {
            al.tbody.empty();
        } else {
            al.currentPage.val(parseInt(al.currentPage.val()) + 1);
        }
        al.emptyListInfo.hide();
        var formData = aff.serialize();
        al.loadingActive.show();
        al.searchString.prop('disabled', true);
        al.searchRole.prop('disabled', true);
        al.submitBtn.prop('disabled', true);
        al.resetBtn.prop('disabled', true);
        var loadMoreBtn = $('#loadMoreBtn');
        if (loadMoreBtn.length > 0) {
            loadMoreBtn.hide();
        }
        
        // the number of table rows will be the index
        // of the next newly added row
        var newStartIdx = al.tbody.children().length;
        
        $.ajax({
            url: '/domain/' + aff.data('domainid') + '/accounts',
            data: formData,
            dataType: 'json'
        }).always(function() {
            al.loadingActive.hide();
            al.searchString.prop('disabled', false);
            al.searchRole.prop('disabled', false);
            al.submitBtn.prop('disabled', false);
            al.resetBtn.prop('disabled', false);
            aff.data('loading', '0');
        }).done(function(data) {
            var accounts = data.accounts;
            var accountsLength = accounts.length;
            if (accountsLength > 0) {
                for (var i = 0; i < accountsLength; i++) {
                    var tr = Skaffari.DefaultTmpl.AccountList.createRow(accounts[i]);
                    al.tbody.append(tr);
                }
            } else {                    
                al.emptyListInfo.show();                    
            }
            
            if (data.currentPage < data.lastPage) {
                if (loadMoreBtn.length > 0) {
                    loadMoreBtn.show();
                } else {
                    loadMoreBtn = $('<div>');
                    loadMoreBtn.attr('id', 'loadMoreBtn');
                    loadMoreBtn.addClass('row mt-4');
                    var lmbCol = $('<div>');
                    lmbCol.addClass('col-12');
                    var lmbtn = $('<button>');
                    lmbtn.attr('type', 'button');
                    lmbtn.addClass('btn btn-secondary btn-lg btn-block');
                    lmbtn.text(al.l10n.loadMore);
                    lmbtn.click(function() {
                        al.load(true);
                    });
                    lmbCol.append(lmbtn);
                    loadMoreBtn.append(lmbCol);
                    al.loadingActive.after(loadMoreBtn);
                }
            }
            
            if (loadMore) {
                // calculate the offset to the top
                // if we do not take the fixed top navabar into account,
                // scroll target will be behind it, 15 is an extra margin
                var dist = al.tbody.children().eq(newStartIdx).offset().top - $('nav.fixed-top').height() - 15;
                $('html, body').animate({
                    scrollTop: dist
                }, 400);
            }
            
        }).fail(function(jqXHR) {
            Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, Skaffari.DefaultTmpl.messageContainer, 'mt-1')
        });
    }
}

Skaffari.DefaultTmpl.AccountList.resetFilters = function() {
    var al = Skaffari.DefaultTmpl.AccountList;
    al.searchString.val('');
    al.searchRole.val('username');
    al.currentPage.val('1');
    al.load(false);
}

Skaffari.DefaultTmpl.AccountList.init = function() {
    var al = Skaffari.DefaultTmpl.AccountList;
    var aff = al.filterForm;
    if (aff.length > 0) {
        
        al.emptyListInfo = $('#emptyListInfo');
        al.emptyListInfo.hide();
        al.searchString = $('#searchString');
        al.searchRole = $('#searchRole');
        al.submitBtn = $('#accountFilterFormSubmit');
        al.resetBtn = $('#accountFilterFormReset');
        al.tbody = $('#accountsTable tbody');
        al.loadingActive = $('#loadingActive');
        al.currentPage = $('#currentPage');
        al.accountsPerPage = $('#accountsPerPage');
        
        al.l10n = JSON.parse(document.getElementById('translationStrings').innerHTML);
        
        aff.data('loading', '0');
        
        al.load(false);
        
        aff.submit(function(e) {
            al.currentPage.val('1');
            al.load(false);
            e.preventDefault();
        });
        
        al.searchRole.change(function() {
            if (al.searchString.val().trim().length > 0) {
                al.currentPage.val('1');
                al.load(false);
            }
        });
        
        al.resetBtn.click(function() {
            al.resetFilters();
        });
        
        // initialize icon on column header
        var currentCol = $('#accountsTable th[data-sortby="' + $('#sortBy').val() + '"]');
        currentCol.append(' <small><i class="fa fa-sort-' + currentCol.data('sorttype') + '-' + $('#sortOrder').val() + ' text-muted"></i></small>');
        
        $('#accountsTable th.sortable').click(function(e) {
            if (aff.data('loading') == "0") {
                $('#accountsTable th > small').remove();
                var th = $(e.target);
                var currentSortBy = $('#sortBy').val();
                var currentSortOrder = $('#sortOrder').val();
                var sortBy = th.data('sortby');
                var sortOrder = th.data('sortorder');
                
                if (currentSortBy == sortBy) {
                    if (currentSortOrder == "asc") {
                        $('#sortOrder').val('desc');
                    } else {
                        $('#sortOrder').val('asc');
                    }
                } else {
                    $('#sortBy').val(sortBy);
                    $('#sortOrder').val(sortOrder);
                }
                th.append(' <small><i class="fa fa-sort-' + th.data('sorttype') + '-' + $('#sortOrder').val() + ' text-muted"></i></small>');
                al.load();
            }
        });
    }
}

$(function() {
    Skaffari.DefaultTmpl.AccountList.init();
});
