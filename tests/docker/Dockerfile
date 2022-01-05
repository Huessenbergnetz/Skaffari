FROM opensuse/leap:15.2

WORKDIR /init

RUN zypper ar -G https://download.opensuse.org/repositories/home:/buschmann23/openSUSE_Leap_15.2/home:buschmann23.repo && \
zypper ar -G -p 90 https://download.opensuse.org/repositories/home:buschmann23:cyrus:3.0/openSUSE_Leap_15.2/home:buschmann23:cyrus:3.0.repo && \
zypper ref && \
zypper patch && \
zypper patch && \
zypper -n install cyrus-imapd mariadb hostname rsyslog cyrus-sasl cyrus-sasl-plain cyrus-sasl-sqlauxprop pam_mysql \
cyrus-sasl-crammd5 cyrus-sasl-digestmd5 cyrus-sasl-saslauthd cyrus-sasl-scram && \
zypper clean && \
/usr/bin/mysql_install_db --force --user=mysql && \
/usr/sbin/rsyslog-service-prepare

EXPOSE 143 4190 3306

ADD init.sh /init
ADD imapd.conf /etc

CMD ./init.sh
