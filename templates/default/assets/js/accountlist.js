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

    var tr = Skaffari.DefaultTmpl.AccountList.accountRowTemplate.content.querySelector("tr");
    tr.id = 'account-' + a.domainId + '-' + a.id;
    tr.dataset.domainid = a.domainId;
    tr.dataset.accountid = a.id;
    tr.dataset.username = a.username;

    var td = Skaffari.DefaultTmpl.AccountList.accountRowTemplate.content.querySelectorAll("td");

    // start action buttons
    var actionBtns = td[0].querySelectorAll("a");

    actionBtns[0].setAttribute('href', '/account/' + a.domainId + '/' + a.id + '/edit');
    actionBtns[1].setAttribute('href', '/account/' + a.domainId + '/' + a.id + '/addresses');
    actionBtns[2].setAttribute('href', '/account/' + a.domainId + '/' + a.id + '/forwards');
    // end action buttons

    // start column for username and id
    var unLink = td[1].querySelector("a");
    unLink.setAttribute('href', '/account/' + a.domainId + '/' + a.id + '/addresses');
    unLink.textContent = a.username;
    td[1].querySelector("small").textContent = l10n.id + ' ' + a.id;
    // end column for username and id


    // start setting the addresses
    // at first remove the old content
    while (td[2].firstChild) {
        td[2].removeChild(td[2].firstChild);
    }
    var addressesLength = a.addresses.length;
    if (a.catchAll) {
        var warnSpan = document.createElement('span');
        warnSpan.textContent = l10n.catchAll;
        warnSpan.className = "text-warning";
        td[2].appendChild(warnSpan);
        if (addressesLength > 0) {
            td[2].appendChild(document.createElement('br'));
        }
    }
    for (var i = 0; i < addressesLength; i++) {
        td[2].appendChild(document.createTextNode(a.addresses[i]));
        if (i < (addressesLength - 1)) {
            td[2].appendChild(document.createElement('br'));
        }
    }
    // end setting the addresses

    // start setting forwards
    // at first remove the old content
    while (td[3].firstChild) {
        td[3].removeChild(td[3].firstChild);
    }
    var forwardsLength = a.forwards.length;
    for (var i = 0; i < forwardsLength; i++) {
        td[3].appendChild(document.createTextNode(a.forwards[i]));
        if (i < (forwardsLength - 1)) {
            td[3].appendChild(document.createElement('br'));
        }
    }
    if (a.keepLocal) {
        var klIcon = document.createElement('i');
        klIcon.className = "fas fa-copy";
        td[3].appendChild(document.createElement('br'));
        td[3].appendChild(klIcon);
        td[3].appendChild(document.createTextNode(' ' + l10n.keepLocal));
    }
    // end setting forwards

    // start setting contingent
    var progDivs = td[4].querySelectorAll('div');
    var usagePercent = 0;
    if ((a.quota != 0) && (a.usage != 0)) {
        usagePercent = (a.usage / a.quota) * 100;
    }
    var usagePercentStr = usagePercent.toLocaleString(undefined, {minimumFractionDigits: 0, maximumFractionDigits: 2}) + '%';
    progDivs[0].setAttribute('title', usagePercentStr);
    progDivs[1].className = 'progress-bar bg-info pb-w-' + Math.round(usagePercent).toString();
    progDivs[1].setAttribute('title', usagePercentStr);
    progDivs[1].setAttribute('aria-valuenow', a.usage);
    progDivs[1].setAttribute('aria-valuemax', a.quota);
    progDivs[1].textContent = usagePercentStr;
    td[4].querySelector('small').textContent = Skaffari.DefaultTmpl.humanBinarySize(a.usage * 1024) + '/' + Skaffari.DefaultTmpl.humanBinarySize(a.quota * 1024);
    // end setting contingent

    // start settings account times
    // at first remove the old content
    while (td[5].firstChild) {
        td[5].removeChild(td[5].firstChild);
    }
    var dateOptions = {
        year: "2-digit",
        month: "2-digit",
        day: "2-digit",
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit"
    };
    var created = new Date(a.created);
    td[5].appendChild(document.createTextNode(created.toLocaleString(undefined, dateOptions)));
    td[5].appendChild(document.createElement('br'));
    var updated = new Date(a.updated);
    td[5].appendChild(document.createTextNode(updated.toLocaleString(undefined, dateOptions)));
    td[5].appendChild(document.createElement('br'));
    var validUntil = new Date(a.validUntil);
    if (a.expired) {
        var validUntilSpan = document.createElement('span');
        validUntilSpan.className = "text-danger";
        validUntilSpan.appendChild(document.createTextNode(validUntil.toLocaleString(undefined, dateOptions)));
        td[5].appendChild(validUntilSpan);
    } else {
        td[5].appendChild(document.createTextNode(validUntil.toLocaleString(undefined, dateOptions)));
    }
    td[5].appendChild(document.createElement('br'));
    var passwordExpires = new Date(a.passwordExpires);
    if (a.passwordExpired) {
        var passwordExpiresSpan = document.createElement('span');
        passwordExpiresSpan.className = "text-danger";
        passwordExpiresSpan.appendChild(document.createTextNode(passwordExpires.toLocaleString(undefined, dateOptions)));
    } else {
        td[5].appendChild(document.createTextNode(passwordExpires.toLocaleString(undefined, dateOptions)));
    }
    // end setting account times

    // start setting account services
    var baseClass = 'far fa-fw ';
    var enabledClass = 'fa-check-square text-success';
    var disabledClass = 'fa-square text-danger';

    var serviceIcons = td[6].querySelectorAll('i');
    serviceIcons[0].className = baseClass + (a.imap ? enabledClass : disabledClass);
    serviceIcons[1].className = baseClass + (a.pop ? enabledClass : disabledClass);
    serviceIcons[2].className = baseClass + (a.sieve ? enabledClass : disabledClass);
    serviceIcons[3].className = baseClass + (a.smtpauth ? enabledClass : disabledClass);

    // end setting account services

    var clone = document.importNode(Skaffari.DefaultTmpl.AccountList.accountRowTemplate.content, true);

    return clone;
};

Skaffari.DefaultTmpl.AccountList.checkAccount = function() {
    var cam = Skaffari.DefaultTmpl.AccountList.checkAccountModal;
    var domainid = cam.data('domainid');
    var accountid = cam.data('accountid');
    var checkAccountList = $('#checkAccountList');
    checkAccountList.empty();
    var checkOptions = $('#checkAccountForm').serialize();
    var checkAccountActive = $('#checkAccountActive');
    checkAccountActive.show();
    var checkAccountSubmit = $('#checkAccountSubmit');
    checkAccountSubmit.prop('disabled', true);
    var checkChildAddresses = $('input[name="checkChildAddresses"]');
    checkChildAddresses.prop('disabled', true);
    $('#check-account-message-container').empty();

    $.ajax({
        method: 'get',
        url: '/account/' + domainid + '/' + accountid + '/check',
        dataType: 'json',
        data: checkOptions
    }).always(function(data) {
        checkAccountActive.hide();
        checkAccountSubmit.prop('disabled', false);
        checkChildAddresses.prop('disabled', false);
    }).done(function(data) {
        if (data.actions) {
            var actions = data.actions;
            var actionsLength = actions.length;
            for (i = 0; i < actionsLength; ++i) {
                var li = $('<li>');
                li.text(actions[i]);
                checkAccountList.append(li);
            }
            var a = data.account;
            var row = Skaffari.DefaultTmpl.AccountList.createRow(a);
            $('#account-' + a.domainId + '-' + a.id).replaceWith(row);
        } else {
            Skaffari.DefaultTmpl.createAlert('success', data.status_msg, '#check-account-message-container');
        }
    }).fail(function(jqXHR) {
        if (jqXHR.responseJSON.error_msg) {
            Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, '#check-account-message-container');
        }
    });
}

Skaffari.DefaultTmpl.AccountList.removeAccount = function() {
    $('#removeAccountForm #accountName').blur();
    $('#remove-account-message-container .alert').alert('close');
    var removeAccountSubmit = $('#removeAccountSubmit');
    removeAccountSubmit.prop('disabled', true);
    var removeAccountIcon = $('#removeAccountIcon');
    removeAccountIcon.removeClass('fa-trash');
    removeAccountIcon.addClass('fa-circle-notch fa-spin');
    var removeAccountForm = $('#removeAccountForm');

    $.ajax({
        method: 'post',
        url: removeAccountForm.attr('action'),
        data: removeAccountForm.serialize(),
        dataType: 'json'
    }).always(function() {
        removeAccountIcon.removeClass('fa-circle-notch fa-spin');
        removeAccountIcon.addClass('fa-trash');
        removeAccountSubmit.prop('disabled', false);
    }).done(function(data) {
        Skaffari.DefaultTmpl.AccountList.removeAccountModal.modal('hide');
        var row = $('#account-' + data.domain_id + '-' + data.account_id);
        row.hide(300, function() {
            row.remove();
        });
    }).fail(function(jqXHR) {
        if (jqXHR.responseJSON.error_msg) {
            Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, '#remove-account-message-container', 'mt-1');
        }
    });
}

Skaffari.DefaultTmpl.AccountList.load = function(loadMore) {

    if (Skaffari.DefaultTmpl.templateSupport) {

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
            al.emptyListInfo.css('display', 'none')
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
                    al.emptyListInfo.css('display', 'flex');
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
                        lmbtn.addClass('btn btn-light btn-lg btn-block');
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
                Skaffari.DefaultTmpl.createAlert('warning', jqXHR.responseJSON.error_msg, Skaffari.DefaultTmpl.messageContainer, 'mt-1');
            });
        }
    } else {
        Skaffari.DefaultTmpl.createAlert('warning', Skaffari.DefaultTmpl.AccountList.l10n.htmlTemplatesNotAvailable, Skaffari.DefaultTmpl.messageContainer, 'mt-1');
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
        al.emptyListInfo.css('dislay', 'none')
        al.searchString = $('#searchString');
        al.searchRole = $('#searchRole');
        al.submitBtn = $('#accountFilterFormSubmit');
        al.resetBtn = $('#accountFilterFormReset');
        al.tbody = $('#accountsTable tbody');
        al.loadingActive = $('#loadingActive');
        al.currentPage = $('#currentPage');
        al.accountsPerPage = $('#accountsPerPage');
        al.checkAccountModal = $('#checkAccountModal');
        al.removeAccountModal = $('#removeAccountModal');
        al.accountRowTemplate = document.getElementById('account-template');

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
        var _sortOrder = ($('#sortOrder').val() == "asc") ? "down" : "up";
        currentCol.append(' <small><i class="fas fa-sort-' + currentCol.data('sorttype') + '-' + _sortOrder + ' text-muted"></i></small>');

        $('#accountsTable th.sortable').click(function(e) {
            if (aff.data('loading') == "0") {
                $('#accountsTable th > small').remove();
                var th = $(e.target);
                var currentSortBy = $('#sortBy').val();
                var currentSortOrder = $('#sortOrder').val();
                var _sortOrder = (currentSortOrder == "asc") ? "down" : "up";
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
                                th.append(' <small><i class="fas fa-sort-' + th.data('sorttype') + '-' + _sortOrder + ' text-muted"></i></small>');
                al.load();
            }
        });

        if (al.checkAccountModal.length > 0) {
            $('#checkAccountActive').hide();

            $('#checkAccountSubmit').click(function() {
                Skaffari.DefaultTmpl.AccountList.checkAccount();
            });

            al.checkAccountModal.on('show.bs.modal', function(e) {
                $('.check-account-btn').prop('disabled', true);
                var btn = $(e.relatedTarget);
                var accountRow = btn.parents('.account-row').first();
                $('#checkAccountName').text(accountRow.data('username'));
                var did = accountRow.data('domainid');
                var aid = accountRow.data('accountid');
                al.checkAccountModal.data({
                    domainid: did,
                    accountid: aid
                });
//                 Skaffari.DefaultTmpl.AccountList.checkAccount();
            });

            al.checkAccountModal.on('hide.bs.modal', function() {
                $('.check-account-btn').prop('disabled', false);
            });
        }

        if (al.removeAccountModal.length > 0) {
            al.removeAccountModal.on('show.bs.modal', function(e) {
                $('.remove-account-btn').prop('disabled', true);
                var btn = $(e.relatedTarget);
                var accountRow = btn.parents('.account-row').first();
                $('#removeAccountName').text(accountRow.data('username'));
                $('#removeAccountForm').attr('action', '/account/' + accountRow.data('domainid') + '/' + accountRow.data('accountid') + '/remove');
                $('#removeAccountForm #accountName').val('');
                $('#remove-account-message-container').empty();
            });

            al.removeAccountModal.on('hide.bs.modal', function() {
                $('.remove-account-btn').prop('disabled', false);
            });

            $('#removeAccountForm').submit(function(e) {
                Skaffari.DefaultTmpl.AccountList.removeAccount();
                e.preventDefault();
            });
        }
    }
}

$(function() {
    Skaffari.DefaultTmpl.AccountList.init();
});
