var elixir = require('laravel-elixir');
elixir.config.assetsPath = 'assets';
elixir.config.publicPath = '';

elixir(function(mix) {

    mix.sass('style.scss', 'static/css/style.css');

    mix.scripts([
        '../../node_modules/jquery/dist/jquery.js',
        '../../node_modules/popper.js/dist/umd/popper.js',
        '../../node_modules/bootstrap/dist/js/bootstrap.js',
        '../../node_modules/select2/dist/js/select2.js',
        '../../node_modules/queuejax/jquery.qjax.js',
        'stupidtable.js',
        'jquery.filtertable.js',
        'defaulttmpl.js',
        'createalert.js',
        'forwards.js',
        'checkdomain.js',
        'manageemailaddresses.js',
        'removedomain.js',
        'removeaccount.js',
        'removeadmin.js',
        'accountlist.js',
        'createdomain.js',
        'general.js'
    ], 'static/js/scripts.js');

    mix.copy('node_modules/font-awesome/fonts', 'static/fonts');
});
