CREATE TABLE IF NOT EXISTS accountuser (
  id int unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT,
  domain_id int unsigned NOT NULL,
  username varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  password varchar(255) NOT NULL,
  imap tinyint(1) unsigned NOT NULL DEFAULT 1,
  pop tinyint(1) unsigned NOT NULL DEFAULT 1,
  sieve tinyint(1) unsigned NOT NULL DEFAULT 1,
  smtpauth tinyint(1) unsigned NOT NULL DEFAULT 1,
  quota bigint unsigned NOT NULL DEFAULT 0,
  created_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  updated_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  valid_until datetime NOT NULL DEFAULT '2998-12-31 23:59:59',
  pwd_expire datetime NOT NULL DEFAULT '2998-12-31 23:59:59',
  status tinyint unsigned NOT NULL DEFAULT 0,
  UNIQUE KEY username (username),
  KEY idx_accountuser_domain_id (domain_id)
) ENGINE = InnoDB DEFAULT CHARSET=latin1;


CREATE TABLE IF NOT EXISTS adminuser (
  id int unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT,
  username varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  password varchar(255) NOT NULL,
  type tinyint unsigned NOT NULL DEFAULT 0,
  created_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  updated_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  valid_until datetime NOT NULL DEFAULT '2998-12-31 23:59:59',
  pwd_expire datetime NOT NULL DEFAULT '2998-12-31 23:59:59',
  UNIQUE KEY username (username)
) ENGINE = InnoDB DEFAULT CHARSET=latin1;


CREATE TABLE IF NOT EXISTS alias (
  id int unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT,
  idn_id int unsigned NOT NULL DEFAULT 0,
  ace_id int unsigned NOT NULL DEFAULT 0,
  alias varchar(255) NOT NULL DEFAULT '',
  dest longtext,
  username varchar(50) NOT NULL DEFAULT '',
  status int NOT NULL DEFAULT '1',
  UNIQUE KEY (alias)
) ENGINE = InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


CREATE TABLE IF NOT EXISTS domain (
  id int unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT,
  parent_id int unsigned NOT NULL DEFAULT 0,
  idn_id int unsigned NOT NULL DEFAULT 0,
  ace_id int unsigned NOT NULL DEFAULT 0,
  domain_name varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  prefix varchar(50) NOT NULL,
  maxaccounts int unsigned NOT NULL DEFAULT 20,
  quota bigint unsigned NOT NULL DEFAULT 100000,
  domainquota bigint unsigned NOT NULL DEFAULT 0,
  domainquotaused bigint unsigned NOT NULL DEFAULT 0,
  transport varchar(255) NOT NULL DEFAULT 'cyrus',
  freenames tinyint(1) unsigned NOT NULL DEFAULT 0,
  freeaddress tinyint(1) unsigned NOT NULL DEFAULT 0,
  accountcount int unsigned NOT NULL DEFAULT 0,
  created_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  updated_at datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  valid_until datetime NOT NULL DEFAULT '2998-12-31 00:00:00',
  UNIQUE KEY domain_name (domain_name),
  UNIQUE KEY prefix (prefix)
) ENGINE = InnoDB DEFAULT CHARSET=latin1;


CREATE TABLE IF NOT EXISTS folder (
  id int unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT,
  domain_id int unsigned NOT NULL,
  name varchar(255) NOT NULL,
  special_use text CHARACTER SET latin1,
  KEY idx_folder_domain_id (domain_id),
  FOREIGN KEY domain_to_folder (domain_id) REFERENCES domain(id) ON DELETE CASCADE
) ENGINE = InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


CREATE TABLE IF NOT EXISTS domainadmin (
  domain_id int unsigned NOT NULL,
  admin_id int unsigned NOT NULL,
  KEY domainadmin_domain_id_idx (domain_id),
  KEY domainadmin_admin_id_idx (admin_id),
  FOREIGN KEY domain_to_domainadmin (domain_id) REFERENCES domain(id) ON DELETE CASCADE,
  FOREIGN KEY admin_to_domainadmin (admin_id) REFERENCES adminuser(id) ON DELETE CASCADE
) ENGINE = InnoDB DEFAULT CHARSET=latin1;


CREATE TABLE IF NOT EXISTS log (
  id int unsigned NOT NULL AUTO_INCREMENT,
  msg text NOT NULL,
  user varchar(255) NOT NULL DEFAULT '',
  host varchar(255) NOT NULL DEFAULT '',
  rhost varchar(255) NOT NULL DEFAULT '',
  time datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  pid varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (id),
  KEY idx_log_user (user)
) ENGINE = InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


CREATE TABLE IF NOT EXISTS settings (
  admin_id int unsigned NOT NULL PRIMARY KEY,
  template varchar(255) NOT NULL DEFAULT 'default',
  maxdisplay tinyint(3) unsigned NOT NULL DEFAULT 25,
  warnlevel tinyint(2) unsigned NOT NULL DEFAULT 90,
  tz varchar(255) NOT NULL DEFAULT 'UTC',
  lang varchar(127) NOT NULL DEFAULT 'en',
  FOREIGN KEY admin_to_settings (admin_id) REFERENCES adminuser(id) ON DELETE CASCADE
) ENGINE = InnoDB DEFAULT CHARSET=latin1;


CREATE TABLE IF NOT EXISTS virtual (
  id int unsigned NOT NULL AUTO_INCREMENT,
  idn_id int unsigned NOT NULL DEFAULT 0,
  ace_id int unsigned NOT NULL DEFAULT 0,
  alias varchar(255) NOT NULL,
  dest longtext,
  username varchar(255) NOT NULL DEFAULT '',
  status int NOT NULL DEFAULT 1,
  KEY alias (alias)
) ENGINE = InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS systeminfo (
  name varchar(127) NOT NULL,
  val longtext NOT NULL,
  UNIQUE KEY (name)
) ENGINE = InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS options (
  option_id int unsigned NOT NULL AUTO_INCREMENT,
  option_name varchar(127) NOT NULL,
  option_value longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  PRIMARY KEY (option_id),
  UNIQUE KEY option_name (option_name)
) ENGINE = InnoDB DEFAULT CHARSET=latin1;

INSERT INTO systeminfo (name, val) VALUES ('skaffari_db_version', '0.0.1');

