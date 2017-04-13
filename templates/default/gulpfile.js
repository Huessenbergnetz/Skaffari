var elixir = require('laravel-elixir');
elixir.config.assetsPath = 'assets';
elixir.config.publicPath = '';

elixir(function(mix) {
    
    mix.sass('style.scss', 'static/css/style.css');
    
    mix.scripts([
        '../../node_modules/jquery/dist/jquery.slim.js',
        '../../node_modules/tether/dist/js/tether.js',
        '../../node_modules/bootstrap/dist/js/bootstrap.js',
        'forwards.js'
    ], 'static/js/scripts.js');
    
    mix.copy('node_modules/font-awesome/fonts', 'static/fonts');
});
