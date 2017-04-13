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

