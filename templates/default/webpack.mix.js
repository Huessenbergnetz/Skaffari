let mix = require('laravel-mix');

mix.options({
    processCssUrls: false
});

mix.sass('assets/sass/style.scss', 'static/css/');

mix.scripts([
    'node_modules/jquery/dist/jquery.js',
    'node_modules/popper.js/dist/umd/popper.js',
    'node_modules/bootstrap/js/dist/util.js',
    'node_modules/bootstrap/js/dist/index.js',
    'node_modules/bootstrap/js/dist/alert.js',
    'node_modules/bootstrap/js/dist/button.js',
    'node_modules/bootstrap/js/dist/collapse.js',
    'node_modules/bootstrap/js/dist/dropdown.js',
    'node_modules/bootstrap/js/dist/modal.js',
    'node_modules/bootstrap/js/dist/tab.js',
    'node_modules/select2/dist/js/select2.js',
    'node_modules/queuejax/jquery.qjax.js',
    'node_modules/stupid-table-plugin/stupidtable.js',
    'node_modules/js-cookie/src/js.cookie.js',
    'assets/js/jquery.i18n/CLDRPluralRuleParser.js',
    'assets/js/jquery.i18n/jquery.i18n.js',
    'assets/js/jquery.i18n/jquery.i18n.messagestore.js',
    'assets/js/jquery.i18n/jquery.i18n.fallbacks.js',
    'assets/js/jquery.i18n/jquery.i18n.parser.js',
    'assets/js/jquery.i18n/jquery.i18n.emitter.js',
    'assets/js/jquery.i18n/jquery.i18n.language.js',
    'assets/js/jquery.filtertable.js',
    'assets/js/general.js',
    'assets/js/forwardlist.js',
    'assets/js/checkdomain.js',
    'assets/js/addresslist.js',
    'assets/js/accountlist.js',
    'assets/js/domainlist.js',
    'assets/js/adminlist.js',
    'assets/js/select2ajaxaccounts.js',
    'assets/js/defaulttmpl.js'
], 'static/js/scripts.js');

mix.copy('node_modules/@fortawesome/fontawesome-free/webfonts', 'static/fonts');

mix.copy('l10n/*.json', 'static/i18n')
