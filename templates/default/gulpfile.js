var elixir = require('laravel-elixir');
elixir.config.assetsPath = 'assets';
elixir.config.publicPath = '';

elixir(function(mix) {
    
    mix.sass('style.scss', 'static/css/style.css');
    
    mix.scripts([
        '../../node_modules/jquery/dist/jquery.js',
        '../../node_modules/tether/dist/js/tether.js',
        '../../node_modules/bootstrap/dist/js/bootstrap.js',
        '../../node_modules/queuejax/jquery.qjax.js',
        'createalert.js',
        'forwards.js',
        'checkdomain.js',
        'manageemailaddresses.js',
        'removedomain.js',
        'removeaccount.js'
    ], 'static/js/scripts.js');
    
    mix.copy('node_modules/font-awesome/fonts', 'static/fonts');
});
