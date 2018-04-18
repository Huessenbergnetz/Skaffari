#!/bin/bash
/usr/sbin/rsyslogd -iNONE
/usr/sbin/mysqld --bind-address=0.0.0.0 --user=mysql &
sleep 2s
mysql -u root -e "CREATE DATABASE ${SKAFFARI_DB_NAME:=skaffaridb};"
mysql -u root -e "CREATE USER '${SKAFFARI_DB_USER:=skaffari}'@'localhost' IDENTIFIED BY '${SKAFFARI_DB_PASS:=schalke05}';"
mysql -u root -e "CREATE USER '${SKAFFARI_DB_USER}'@'%' IDENTIFIED BY '${SKAFFARI_DB_PASS}';"
mysql -u root -e "GRANT ALL ON ${SKAFFARI_DB_NAME}.* TO '${SKAFFARI_DB_USER}'@'%';"

echo "admins: ${IMAP_ADMINS:=cyrus}" >> /etc/imapd.conf
echo "unixhierarchysep: ${UNIXHIERARCHYSEP:=no}" >> /etc/imapd.conf
echo "virtdomains: ${VIRTDOMAINS:=no}" >> /etc/imapd.conf
echo "sasl_mech_list: ${SASL_MECH_LIST:=plain login}" >> /etc/imapd.conf
echo "sasl_pwcheck_method: ${SASL_PWCHECK_METHOD:=auxprop}" >> /etc/imapd.conf
if [ "${SASL_PWCHECK_METHOD}" = "auxprop" ]
then
	echo "sasl_auxprop_plugin: ${SASL_AUXPROP_PLUGIN:=sql}" >> /etc/imapd.conf

	if [ "${SASL_AUXPROP_PLUGIN}" = "sql" ]
	then
		echo "sasl_sql_engine: ${SASL_SQL_ENGINE:=mysql}" >> /etc/imapd.conf
		echo "sasl_sql_hostnames: ${SASL_SQL_HOSTANMES:=127.0.0.1:3306}" >> /etc/imapd.conf
		echo "sasl_sql_user: ${SKAFFARI_DB_USER}" >> /etc/imapd.conf
		echo "sasl_sql_passwd: ${SKAFFARI_DB_PASS}" >> /etc/imapd.conf
		echo "sasl_sql_database: ${SKAFFARI_DB_NAME}" >> /etc/imapd.conf
		echo "sasl_sql_select: SELECT password FROM accountuser WHERE username = '%u'" >> /etc/imapd.conf
	fi
fi
/usr/lib/cyrus/bin/master -D
