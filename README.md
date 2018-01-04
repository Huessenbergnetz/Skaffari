# Skaffari

Skaffari is a web application for managing e-mail accounts, based on [Cutelyst](https://cutelyst.org/) and written in [Qt/C++](https://www.qt.io/). It serves as a link and access to a combination of SQL database, IMAP and SMTP server. Skaffari bundles administrative tasks such as the creation of new e-mail domains and e-mail accounts, as well as the creation of new e-mail addresses and e-mail forwards.

Administrators can be either global or only responsible for specific domains. Individual domains and accounts can be subject to certain restrictions such as storage space, number of accounts or user names.

Skaffari has been tested to work with [Cyrus IMAP](https://www.cyrusimap.org/), [Postfix](http://www.postfix.org/) and [pam_mysql](https://github.com/NigelCunningham/pam-MySQL) and was inspired by a PHP-based tool called [web-cyradm](https://github.com/web-cyradm/web-cyradm).

By the way, Skaffari is the Old High German word for steward.
