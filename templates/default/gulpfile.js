var elixir = require('laravel-elixir');
elixir.config.assetsPath = 'assets';
elixir.config.publicPath = '';

elixir(function(mix) {

    mix.sass('style.scss', 'static/css/style.css');

    mix.scripts([
        '../../node_modules/jquery/dist/jquery.js',
        '../../node_modules/popper.js/dist/umd/popper.js',
        '../../node_modules/bootstrap/js/dist/util.js',
        '../../node_modules/bootstrap/js/dist/index.js',
        '../../node_modules/bootstrap/js/dist/alert.js',
        '../../node_modules/bootstrap/js/dist/button.js',
        '../../node_modules/bootstrap/js/dist/collapse.js',
        '../../node_modules/bootstrap/js/dist/dropdown.js',
        '../../node_modules/bootstrap/js/dist/modal.js',
        '../../node_modules/bootstrap/js/dist/tab.js',
        '../../node_modules/select2/dist/js/select2.js',
        '../../node_modules/queuejax/jquery.qjax.js',
        '../../node_modules/stupid-table-plugin/stupidtable.js',
        '../../node_modules/js-cookie/src/js.cookie.js',
        'jquery.i18n/CLDRPluralRuleParser.js',
        'jquery.i18n/jquery.i18n.js',
        'jquery.i18n/jquery.i18n.messagestore.js',
        'jquery.i18n/jquery.i18n.fallbacks.js',
        'jquery.i18n/jquery.i18n.parser.js',
        'jquery.i18n/jquery.i18n.emitter.js',
        'jquery.i18n/jquery.i18n.language.js',
        'jquery.filtertable.js',
        'general.js',
        'forwardlist.js',
        'checkdomain.js',
        'addresslist.js',
        'accountlist.js',
        'domainlist.js',
        'adminlist.js',
        'select2ajaxaccounts.js',
        'defaulttmpl.js'
    ], 'static/js/scripts.js');

    mix.copy('node_modules/@fortawesome/fontawesome-free-webfonts/webfonts', 'static/fonts');

    mix.copy('l10n/*.json', 'static/i18n')
});
