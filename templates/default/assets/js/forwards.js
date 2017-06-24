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
    $('#addforward').click(function() {
        var count = $('#forwards').data('count');
        count++;
        
        var div1 = $('<div>');
        div1.addClass('form-group');
        div1.attr('id', 'forward' + count);
        
        var div2 = $('<div>');
        div2.addClass('input-group');
        
        var input = $('<input>');
        input.addClass('form-control');
        input.attr('type', 'email');
        input.attr('name', 'forward');
        
        var span = $('<span>');
        span.addClass('input-group-btn');
        
        var btn = $('<button>');
        btn.addClass('btn btn-danger forward-delete-btn');
        btn.attr('type', 'button');
        btn.data('target', '#forward' + count);
        btn.click(function() {
            $($(this).data('target')).remove();
        });
        
        var icon = $('<i>');
        icon.addClass('fa fa-trash');
        
        btn.append(icon);
        span.append(btn);
        div2.append(input);
        div2.append(span);
        div1.append(div2);
        
        $('#forwards').append(div1);
        $('#forwards').data('count', count);
    });
    
    $('.forward-delete-btn').click(function() {
        var btn = $(this);
        $(btn.data('target')).remove();
    });
});

