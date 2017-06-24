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
