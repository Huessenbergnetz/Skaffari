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

function skaffariCreateAlert(type, text, target, classes) {
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
//     warnDiv.alert();
} 
