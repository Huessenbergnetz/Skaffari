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

Skaffari.DefaultTmpl.messageContainer = $('#ajaxMessages');

Skaffari.DefaultTmpl.clearMessages = function() {
    if (Skaffari.DefaultTmpl.messageContainer.length > 0) {
        Skaffari.DefaultTmpl.messageContainer.empty();
    }
}

Skaffari.DefaultTmpl.humanBinarySize = function(size) {
    var ret = '';
    var _size = 0.0;
    var mulit = 'KiB';
    if (size < 1048576) {
        _size = size/1024.0;
        multi = 'KiB';
    } else if (size < 1073741824) {
        _size = size/1048576.0;
        multi = 'MiB';
    } else if (size < 1099511627776) {
        _size = size/1073741824.0;
        multi = 'GiB';
    } else {
        _size = size/1099511627776.0;
        multi = 'TiB';
    }
            
                ret = _size.toFixed(2) + ' ' + multi;
                
                return ret;
}

Skaffari.DefaultTmpl.createAlert = function(type, text, target, classes) {
    var warnDiv = $('<div>');
    warnDiv.addClass('alert');
    warnDiv.addClass('alert-' + type);
    warnDiv.addClass('alert-dismissible fade show');
    if (classes) {
        warnDiv.addClass(classes);
    }
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
        warnDiv.append(text);
        warnDiv.hide();
        $(target).append(warnDiv);
        warnDiv.show(300);
}