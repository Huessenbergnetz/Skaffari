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
    $.i18n().load({
        en: '/i18n/en.json',
        de: '/i18n/de.json'
    });
});

var Skaffari = Skaffari || {};

Skaffari.DefaultTmpl = Skaffari.DefaultTmpl || {};

Skaffari.DefaultTmpl.messageContainer = $('#ajaxMessages');

Skaffari.DefaultTmpl.templateSupport = ('content' in document.createElement('template'));

Skaffari.DefaultTmpl.clearMessages = function() {
    if (Skaffari.DefaultTmpl.messageContainer.length > 0) {
        Skaffari.DefaultTmpl.messageContainer.empty();
    }
}

Skaffari.DefaultTmpl.humanBinarySize = function(size) {
    var ret = '';
    var _size = 0.0;
    var multi = 'KiB';
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
    if (typeof text === 'object') {
        if (Array.isArray(text)) {
            if (text.length > 1) {
                var ul = $('<ul>');
                for (var i = 0; i < text.length; i++) {
                    var li = $('<li>');
                    li.text(text[i]);
                }
                warnDiv.append(ul);
            } else {
                warnDiv.append(text[0]);
            }
        }
    } else {
        warnDiv.append(text);
    }
    warnDiv.hide();
    $(target).append(warnDiv);
    warnDiv.show(300);
}

Skaffari.DefaultTmpl.relativeTime = function(datetime) {
    var _dt = (typeof datetime === 'object') ? datetime : new Date(datetime);
    var _msecs = _dt.getTime();
    if ((typeof _dt === 'object') && (_msecs > 0)) {

        var timeDiff = Date.now() - _msecs;
        timeDiff = timeDiff/1000;
        if (timeDiff > 0) {
            if (timeDiff > 63072000) { // more than 2 years
                return $.i18n('sk-def-tmpl-reltime-yearsago', Math.floor(timeDiff / 31536000));
            } else if (timeDiff > 5270400) { // more than 61 days
                return $.i18n('sk-def-tmpl-reltime-monthsago', Math.floor(timeDiff / 2635200))
            } else if (timeDiff > 172800) { // more than 2 days
                return $.i18n('sk-def-tmpl-reltime-daysago', Math.floor(timeDiff / 86400));
            } else if (timeDiff > 7200) { // more than 2 hours
                return $.i18n('sk-def-tmpl-reltime-hoursago', Math.floor(timeDiff / 3600));
            } else if (timeDiff > 120) { // more than 2 minutes
                return $.i18n('sk-def-tmpl-reltime-minutesago', Math.floor(timeDiff / 60));
            } else {
                return $.i18n('sk-def-tmpl-reltime-secondsago', Math.floor(timeDiff));
            }
        } else if (timeDiff < 0) {
            timeDiff = timeDiff * -1;
            if (timeDiff > 3153600000) { // in more than 100 years
                return $.i18n('sk-def-tmpl-reltime-never');
            } else if (timeDiff > 63072000) { // in more than 2 years
                return $.i18n('sk-def-tmpl-reltime-inyears', Math.floor(timeDiff / 31536000));
            } else if (timeDiff > 5270400) { // in more than 61 days
                return $.i18n('sk-def-tmpl-reltime-inmonths', Math.floor(timeDiff / 2635200));
            } else if (timeDiff > 172800) { // in more than 2 days
                return $.i18n('sk-def-tmpl-reltime-indays', Math.floor(timeDiff / 86400));
            } else if (timeDiff > 7200) { // in more than 2 hours
                return $.i18n('sk-def-tmpl-reltime-inhours', Math.floor(timeDiff / 3600));
            } else if (timeDiff > 120) { // in more than 2 minutes
                return $.i18n('sk-def-tmpl-reltime-inminutes', Math.floor(timeDiff / 60));
            } else {
                return $.i18n('sk-def-tmpl-reltime-inseconds', Math.floor(timeDiff));
            }
        } else {
            return $.i18n('sk-def-tmpl-reltime-justnow');
        }

    } else {
        return $.i18n('sk-def-tmpl-undefined');
    }
}

Skaffari.DefaultTmpl.csrfSafeMethod = function(method) {
    // these HTTP methods do not require CSRF protection
    return (/^(GET|HEAD|OPTIONS|TRACE)$/.test(method));
}

Skaffari.DefaultTmpl.init = function() {
    $.fn.select2.defaults.set("theme", "bootstrap4");
    $('.select2').select2();
    $.ajaxSetup({
        beforeSend: function(xhr, settings) {
            if (!Skaffari.DefaultTmpl.csrfSafeMethod(settings.type) && !this.crossDomain) {
                xhr.setRequestHeader("X-CSRFTOKEN", Cookies.get('csrftoken'));
            }
        }
    });
}
