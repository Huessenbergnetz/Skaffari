CREATE TABLE IF NOT EXISTS accountuser (
  id int(10) unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT,
  domain_id int(10) unsigned NOT NULL,
  username varchar(255)  NOT NULL,
  password varchar(255) NOT NULL,
  prefix varchar(50) NOT NULL,
  domain_name varchar(255) NOT NULL,
  imap tinyint(1) unsigned NOT NULL DEFAULT 1,
  pop tinyint(1) unsigned NOT NULL DEFAULT 1,
  sieve tinyint(1) unsigned NOT NULL DEFAULT 1,
  smtpauth tinyint(1) unsigned NOT NULL DEFAULT 1,
  quota int(10) unsigned NOT NULL DEFAULT 0,
  created_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  updated_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  valid_until datetime NOT NULL DEFAULT '9998-12-31 23:59:59'
  UNIQUE KEY username (username)
) ENGINE = MyISAM;



CREATE TABLE IF NOT EXISTS adminuser (
  id int(10) unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT,
  username varchar(255) NOT NULL,
  password varchar(255) NOT NULL,
  type tinyint(1) unsigned NOT NULL DEFAULT 0,
  created_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  updated_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  UNIQUE KEY username (username)
) ENGINE = MyISAM;



CREATE TABLE IF NOT EXISTS alias (
  alias varchar(255) NOT NULL DEFAULT '',
  dest longtext,
  username varchar(50) NOT NULL DEFAULT '',
  status int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (alias)
) ENGINE = MyISAM;



CREATE TABLE IF NOT EXISTS domain (
  id int(10) unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT,
  domain_name varchar(255) NOT NULL,
  prefix varchar(50) NOT NULL,
  maxaccounts int(11) NOT NULL DEFAULT 20,
  quota int(10) unsigned NOT NULL DEFAULT 100000,
  domainquota int(10) unsigned NOT NULL DEFAULT 0,
  domainquotaused int(10) unsigned NOT NULL DEFAULT 0,
  transport varchar(255) NOT NULL DEFAULT 'cyrus',
  freenames tinyint(1) unsigned NOT NULL DEFAULT 0,
  freeaddress tinyint(1) unsigned NOT NULL DEFAULT 0,
  folders varchar(255) DEFAULT '',
  accountcount int(10) unsigned NOT NULL DEFAULT 0,
  created_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  updated_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  UNIQUE KEY domain_name (domain_name),
  UNIQUE KEY prefix (prefix)
) ENGINE = MyISAM;



CREATE TABLE IF NOT EXISTS domainadmin (
  domain_id int(10) unsigned NOT NULL,
  admin_id int(10) unsigned NOT NULL
) ENGINE = MyISAM;



CREATE TABLE IF NOT EXISTS log (
  id int(11) NOT NULL AUTO_INCREMENT,
  msg text NOT NULL,
  user varchar(255) NOT NULL DEFAULT '',
  host varchar(255) NOT NULL DEFAULT '',
  rhost varchar(255) NOT NULL DEFAULT '',
  time datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  pid varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (id),
  KEY idx_log_user (user)
) ENGINE = MyISAM;


CREATE TABLE IF NOT EXISTS settings (
  admin_id int(10) unsigned NOT NULL PRIMARY KEY,
  template varchar(255) NOT NULL DEFAULT 'default',
  maxdisplay tinyint(3) unsigned NOT NULL DEFAULT 25,
  warnlevel tinyint(2) unsigned NOT NULL DEFAULT 90,
  tz varchar(255) NOT NULL DEFAULT 'UTC',
  lang varchar(127) NOT NULL DEFAULT 'en'
) ENGINE = MyISAM;


CREATE TABLE IF NOT EXISTS virtual (
  alias varchar(255) NOT NULL,
  dest longtext,
  username varchar(255) NOT NULL DEFAULT '',
  status int(11) NOT NULL DEFAULT 1,
  KEY alias (alias)
) ENGINE = MyISAM;


CREATE TABLE IF NOT EXISTS systeminfo (
  name varchar(127) NOT NULL,
  val longtext NOT NULL
) ENGINE = MyISAM;


INSERT INTO systeminfo (name, val) VALUES ('skaffari_db_version', '0.0.1');

