FROM opensuse/leap:42.3

WORKDIR /init

RUN zypper ar -G https://download.opensuse.org/repositories/home:/buschmann23/openSUSE_Leap_42.3/home:buschmann23.repo && \
zypper ref && \
zypper -n install cyrus-imapd mariadb hostname rsyslog cyrus-sasl cyrus-sasl-plain cyrus-sasl-sqlauxprop pam_mysql \
cyrus-sasl-crammd5 cyrus-sasl-digestmd5 cyrus-sasl-saslauthd cyrus-sasl-scram && \
zypper clean && \
mkdir /var/run/mysql && \
chown mysql:mysql /var/run/mysql && \
/usr/bin/mysql_install_db --force --user=mysql && \
/usr/sbin/rsyslog-service-prepare

EXPOSE 143 4190 3306

ADD init.sh /init
ADD imapd.conf /etc

CMD ./init.sh
