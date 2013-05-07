Name:           msg-service
Version:        0.9.2
Release:        3
License:        Samsung
Summary:        Messaging Framework Library
Group:          System/Libraries
Source0:        %{name}-%{version}.tar.gz
Source101:      msg-service.service

Requires(post): /usr/bin/sqlite3
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires(post): systemd
Requires(postun): systemd
BuildRequires: cmake
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(contacts-service2)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(drm-client)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(libsystemd-daemon)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(libwbxml2)
BuildRequires: pkgconfig(media-thumbnail)
BuildRequires: pkgconfig(mm-fileinfo)
BuildRequires: pkgconfig(mm-player)
BuildRequires: pkgconfig(mm-session)
BuildRequires: pkgconfig(mm-sound)
BuildRequires: pkgconfig(network)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(pmapi)
BuildRequires: pkgconfig(mmutil-imgp)
BuildRequires: pkgconfig(mmutil-jpeg)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(sensor)
BuildRequires: pkgconfig(svi)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(feedback)

%description
Description: Messaging Framework Library


%package devel
License:        Flora License v1.0
Summary:        Messaging Framework Library (development)
Requires:       %{name} = %{version}-%{release}
Group:          Development/Libraries

%description devel
Description: Messaging Framework Library (development)


%package tools
License:        Flora License v1.0
Summary:        Messaging server application
Requires:       %{name} = %{version}-%{release}
Group:          TO_BU / FILL_IN
Requires(post): /usr/bin/sqlite3
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description tools
Description:  Messaging server application


%package -n sms-plugin
License:        Flora License v1.0
Summary:        SMS plugin library
Requires:       %{name} = %{version}-%{release}
Group:          System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description -n sms-plugin
Description: SMS plugin library

%package -n mms-plugin
License:        Flora License v1.0
Summary:        MMS plugin library
Requires:       %{name} = %{version}-%{release}
Group:          System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description -n mms-plugin
Description: MMS plugin library

%prep
%setup -q


%build
%cmake .
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/etc/config

mkdir -p %{buildroot}/var/log/msgfw

%make_install

mkdir -p %{buildroot}/usr/lib/systemd/user/tizen-middleware.target.wants
#install -m 0644 %SOURCE101 %{buildroot}/usr/lib/systemd/user/
ln -s ../msg-service.service %{buildroot}/usr/lib/systemd/user/tizen-middleware.target.wants/msg-service.service
ln -s ../msg-service-log.service %{buildroot}/usr/lib/systemd/user/tizen-middleware.target.wants/msg-service-log.service

mkdir -p  %{buildroot}%{_sysconfdir}/rc.d/rc3.d
ln -s %{_sysconfdir}/rc.d/init.d/msg-server  %{buildroot}%{_sysconfdir}/rc.d/rc3.d/S70msg-server
mkdir -p  %{buildroot}%{_sysconfdir}/rc.d/rc5.d
ln -s %{_sysconfdir}/rc.d/init.d/msg-server  %{buildroot}%{_sysconfdir}/rc.d/rc5.d/S70msg-server

mkdir -p %{buildroot}/opt/usr/data/msg-service

%if 0%{?simulator}
rm %{buildroot}/etc/config/sysinfo-message.xml
mv %{buildroot}/etc/config/sysinfo-message.emul.xml %{buildroot}/etc/config/sysinfo-message.xml
%else
rm %{buildroot}/etc/config/sysinfo-message.emul.xml
%endif


%post tools -p /sbin/ldconfig
%post -n sms-plugin -p /sbin/ldconfig
%post -n mms-plugin -p /sbin/ldconfig

%post
/sbin/ldconfig

if [ ! -f /opt/usr/dbspace/.msg_service.db ]
then
    mkdir -p /opt/usr/dbspace/
    sqlite3 /opt/usr/dbspace/.msg_service.db "PRAGMA journal_mode = PERSIST;

    CREATE TABLE MSG_CONVERSATION_TABLE (
	CONV_ID INTEGER PRIMARY KEY ,
	UNREAD_CNT INTEGER DEFAULT 0 ,
	SMS_CNT INTEGER DEFAULT 0 ,
	MMS_CNT INTEGER DEFAULT 0 ,
	MAIN_TYPE INTEGER DEFAULT 0 ,
	SUB_TYPE INTEGER DEFAULT 0 ,
	MSG_DIRECTION INTEGER DEFAULT 0 ,
	DISPLAY_TIME DATETIME ,
	DISPLAY_NAME TEXT ,
	MSG_TEXT TEXT );

    CREATE TABLE MSG_ADDRESS_TABLE (
	ADDRESS_ID INTEGER PRIMARY KEY ,
	CONV_ID INTEGER  NOT NULL ,
	ADDRESS_TYPE INTEGER ,
	RECIPIENT_TYPE INTEGER ,
	ADDRESS_VAL TEXT ,
	CONTACT_ID INTEGER ,
	DISPLAY_NAME TEXT ,
	FIRST_NAME TEXT ,
	LAST_NAME TEXT ,
	IMAGE_PATH TEXT ,
	SYNC_TIME DATETIME ,
	FOREIGN KEY (CONV_ID) REFERENCES MSG_CONVERSATION_TABLE (CONV_ID) );

    CREATE TABLE MSG_FOLDER_TABLE (
	FOLDER_ID INTEGER PRIMARY KEY ,
	FOLDER_NAME TEXT NOT NULL ,
	FOLDER_TYPE INTEGER DEFAULT 0 );

    CREATE TABLE MSG_MESSAGE_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	CONV_ID INTEGER NOT NULL ,
	FOLDER_ID INTEGER NOT NULL ,
	STORAGE_ID INTEGER NOT NULL ,
	MAIN_TYPE INTEGER NOT NULL ,
	SUB_TYPE INTEGER NOT NULL ,
	DISPLAY_TIME DATETIME ,
	DATA_SIZE INTEGER DEFAULT 0 ,
	NETWORK_STATUS INTEGER DEFAULT 0 ,
	READ_STATUS INTEGER DEFAULT 0 ,
	PROTECTED INTEGER DEFAULT 0 ,
	PRIORITY INTEGER DEFAULT 0 ,
	MSG_DIRECTION INTEGER NOT NULL ,
	SCHEDULED_TIME DATETIME ,
	BACKUP INTEGER DEFAULT 0 ,
	SUBJECT TEXT ,
	MSG_DATA TEXT ,
	THUMB_PATH TEXT ,
	MSG_TEXT TEXT ,
	ATTACHMENT_COUNT INTEGER DEFAULT 0 ,
	FOREIGN KEY (CONV_ID) REFERENCES MSG_CONVERSATION_TABLE (CONV_ID) ,
	FOREIGN KEY (FOLDER_ID) REFERENCES MSG_FOLDER_TABLE (FOLDER_ID) );

    CREATE TABLE MSG_SIM_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	SIM_ID INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_PUSH_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	ACTION INTEGER ,
	CREATED INTEGER ,
	EXPIRES INTEGER ,
	ID TEXT ,
	HREF TEXT ,
	CONTENT TEXT ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_CBMSG_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	CB_MSG_ID INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_SYNCML_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	EXT_ID INTEGER NOT NULL ,
	PINCODE INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_SCHEDULED_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	ALARM_ID INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_SMS_SENDOPT_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	DELREP_REQ INTEGER NOT NULL ,
	KEEP_COPY INTEGER NOT NULL ,
	REPLY_PATH INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_FILTER_TABLE (
	FILTER_ID INTEGER PRIMARY KEY ,
	FILTER_TYPE INTEGER NOT NULL ,
	FILTER_VALUE TEXT NOT NULL ,
	FILTER_ACTIVE INTEGER DEFAULT 0 );

    CREATE TABLE MSG_MMS_MESSAGE_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	TRANSACTION_ID TEXT ,
	MESSAGE_ID TEXT ,
	FWD_MESSAGE_ID TEXT ,
	CONTENTS_LOCATION TEXT ,
	FILE_PATH TEXT ,
	VERSION INTEGER NOT NULL ,
	DATA_TYPE INTEGER DEFAULT -1 ,
	DATE DATETIME ,
	HIDE_ADDRESS INTEGER DEFAULT 0 ,
	ASK_DELIVERY_REPORT INTEGER DEFAULT 0 ,
	REPORT_ALLOWED INTEGER DEFAULT 0 ,
	READ_REPORT_ALLOWED_TYPE INTEGER DEFAULT 0 ,
	ASK_READ_REPLY INTEGER DEFAULT 0 ,
	READ INTEGER DEFAULT 0 ,
	READ_REPORT_SEND_STATUS INTEGER DEFAULT 0 ,
	READ_REPORT_SENT INTEGER DEFAULT 0 ,
	PRIORITY INTEGER DEFAULT 0 ,
	KEEP_COPY INTEGER DEFAULT 0 ,
	MSG_SIZE INTEGER NOT NULL ,
	MSG_CLASS INTEGER DEFAULT -1 ,
	EXPIRY_TIME DATETIME ,
	CUSTOM_DELIVERY_TIME INTEGER DEFAULT 0 ,
	DELIVERY_TIME DATETIME ,
	MSG_STATUS INTEGER DEFAULT -1 ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_MMS_PREVIEW_INFO_TABLE (
	MSG_ID INTEGER NOT NULL ,
	TYPE INTEGER,
	VALUE TEXT,
	COUNT INTEGER,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_REPORT_TABLE (
	MSG_ID INTEGER NOT NULL ,
	ADDRESS_VAL TEXT ,
	STATUS_TYPE INTEGER ,
	STATUS INTEGER DEFAULT 0 ,
	TIME DATETIME );

    CREATE TABLE MSG_PUSHCFG_TABLE (
	PUSH_ID INTEGER PRIMARY KEY ,
	CONTENT_TYPE TEXT,
	APP_ID TEXT,
	PKG_NAME TEXT,
	LAUNCH INTEGER,
	APPCODE INTEGER,
	SECURE INTEGER );

    CREATE INDEX MSG_CONVERSATION_INDEX ON MSG_CONVERSATION_TABLE(CONV_ID);
    CREATE INDEX MSG_FOLDER_INDEX ON MSG_FOLDER_TABLE(FOLDER_ID);
    CREATE INDEX MSG_MESSAGE_INDEX ON MSG_MESSAGE_TABLE(MSG_ID, CONV_ID, FOLDER_ID);

    INSERT INTO MSG_FOLDER_TABLE VALUES (1, 'INBOX', 1);
    INSERT INTO MSG_FOLDER_TABLE VALUES (2, 'OUTBOX', 2);
    INSERT INTO MSG_FOLDER_TABLE VALUES (3, 'SENTBOX', 2);
    INSERT INTO MSG_FOLDER_TABLE VALUES (4, 'DRAFT', 3);
    INSERT INTO MSG_FOLDER_TABLE VALUES (5, 'CBMSGBOX', 1);
    INSERT INTO MSG_FOLDER_TABLE VALUES (6, 'SPAMBOX', 4);
    INSERT INTO MSG_FOLDER_TABLE VALUES (7, 'SMS TEMPLATE', 5);
    INSERT INTO MSG_FOLDER_TABLE VALUES (8, 'MMS TEMPLATE', 5);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (1, 'text/vnd.wap.si', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 1, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (2, 'application/vnd.wap.sic', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 2, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (3, 'text/vnd.wap.sl', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 3, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (4, 'application/vnd.wap.slc', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 4, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (5, 'text/vnd.wap.co', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 5, 0);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (6, 'application/vnd.wap.coc', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 6, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (7, 'application/vnd.wap.mms-message', 'X-Wap-Application-Id: x-wap-application:mms.ua', '', 0, 7, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (8, 'application/vnd.wap.sia', 'X-Wap-Application-Id: x-wap-application:push.sia', '', 0, 8, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (9, 'application/vnd.syncml.dm+wbxml', 'X-Wap-Application-Id: x-wap-application:push.syncml.dm', '', 0, 9, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (10, 'application/vnd.syncml.dm+xml', 'X-Wap-Application-Id: x-wap-application:push.syncml.dm', '', 0, 10, 0);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (11, 'application/vnd.syncml.notification', 'X-Wap-Application-Id: x-wap-application:push.syncml.dm', '', 0, 11, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (12, 'application/vnd.syncml.ds.notification', 'X-Wap-Application-Id: x-wap-application:push.syncml.ds', '', 0, 12, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (13, 'application/vnd.syncml+wbxml', 'X-Wap-Application-Id:x-wap-application:push.syncml', '', 0, 13, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (14, 'application/vnd.wap.locc+wbxml', 'X-Wap-Application-Id: x-wap-application:loc.ua', '', 0, 14, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (15, 'application/vnd.wap.loc+xml', 'X-Wap-Application-Id: x-wap-application:loc.ua', '', 0, 15, 0);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (16, 'application/vnd.oma.dd+xml', 'X-Wap-Application-Id: x-wap-application:loc.ua', '', 0, 16, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (17, 'application/vnd.oma.drm.message', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 17, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (18, 'application/vnd.oma.drm.content', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 18, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (19, 'application/vnd.oma.drm.rights+xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 19, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (20, 'application/vnd.oma.drm.rights+wbxml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 20, 0);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (21, 'application/vnd.oma.drm.ro+xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 21, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (22, 'application/vnd.oma.drm.roap-pdu+xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 22, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (23, 'application/vnd.oma.drm.roap-trigger+xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 23, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (24, 'application/vnd.oma.drm.roap-trigger+wbxml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 24, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (25, 'text/vnd.wap.connectivity-xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 26, 0);

	INSERT INTO MSG_PUSHCFG_TABLE VALUES (26, 'application/vnd.wap.connectivity-wbxml', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 27, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (27, 'application/x-wap-prov.browser-settings', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 28, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (28, 'application/x-wap-prov.browser-bookmarks', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 29, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (29, 'application/x-wap-prov.syncset+xml', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 30, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (30, 'application/x-wap-prov.syncset+wbxml', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 31, 0);

	INSERT INTO MSG_PUSHCFG_TABLE VALUES (31, 'text/vnd.wap.emn+xml', 'X-Wap-Application-Id: x-wap-application:emn.ua', '', 0, 32, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (32, 'application/vnd.wap.emn+wbxml', 'X-Wap-Application-Id: x-wap-application:emn.ua', '', 0, 33, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (33, 'application/vnd.wv.csp.cir', 'X-Wap-Application-Id: x-wap-application:wv.ua', '', 0, 34, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (34, 'application/vnd.omaloc-supl-init', 'X-Wap-Application-Id: x-oma-application:ulp.ua', '', 0, 44, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (35, 'application/vnd.wap.emn+wbxml', 'X-oma-docomo:xmd.mail.ua', '', 0, 45, 1);"
fi

chown :6011 /opt/usr/dbspace/.msg_service.db
chown :6011 /opt/usr/dbspace/.msg_service.db-journal
chmod 660 /opt/usr/dbspace/.msg_service.db
chmod 660 /opt/usr/dbspace/.msg_service.db-journal
mkdir -p /opt/usr/data/msg-service
chgrp db_msg_service /opt/usr/data/msg-service

if [ -f /usr/lib/rpm-plugins/msm.so ]
then
	chsmack -a 'msg-service::db' /opt/usr/dbspace/.msg_service.db*
        chsmack -a "_" -e "_" /etc/rc.d/init.d/msg-server
        chsmack -a "_" -e "_" /etc/rc.d/rc3.d/S70msg-server
        chsmack -a "_" -e "_" /etc/rc.d/rc5.d/S70msg-server
fi

########## Setting Config Value (Internal keys) ##########
vcuid=5000
# Message Server Status
vconftool set -t bool memory/msg/ready 0 -i -g 5000 -u $vcuid

# SMS Send Options
vconftool set -t int db/msg/network_mode 2 -u $vcuid

# New Message Count
vconftool set -t int db/msg/recv_sms 0 -u $vcuid
vconftool set -t int db/msg/recv_mms 0 -u $vcuid

########## Setting Config Value (Private keys) ##########
# General Options
vconftool set -t bool db/private/msg-service/general/keep_copy 1 -u $vcuid
vconftool set -t bool db/private/msg-service/general/auto_erase 0 -u $vcuid
vconftool set -t bool db/private/msg-service/general/block_msg 0 -u $vcuid
vconftool set -t int db/private/msg-service/general/contact_sync_time 0 -u $vcuid

# SMS Send Options
vconftool set -t int db/private/msg-service/sms_send/dcs 3 -u $vcuid
vconftool set -t bool db/private/msg-service/sms_send/reply_path 0 -u $vcuid
vconftool set -t bool db/private/msg-service/sms_send/delivery_report 0 -u $vcuid
vconftool set -t int db/private/msg-service/sms_send/save_storage 1 -u $vcuid

# SMSC
vconftool set -t int db/private/msg-service/smsc/total_count 1 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/selected 0 -u $vcuid

vconftool set -t int db/private/msg-service/smsc/pid/0 1 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/val_period/0 255 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/name/0 "" -u $vcuid
vconftool set -t int db/private/msg-service/smsc/ton/0 1 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/npi/0 1 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/address/0 "" -u $vcuid

vconftool set -t int db/private/msg-service/smsc/pid/1 0 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/val_period/1 0 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/name/1 "" -u $vcuid
vconftool set -t int db/private/msg-service/smsc/ton/1 0 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/npi/1 0 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/address/1 "" -u $vcuid

vconftool set -t int db/private/msg-service/smsc/pid/2 0 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/val_period/2 0 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/name/2 "" -u $vcuid
vconftool set -t int db/private/msg-service/smsc/ton/2 0 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/npi/2 0 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/address/2 "" -u $vcuid

# MMS Send Options
vconftool set -t int db/private/msg-service/mms_send/msg_class 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/priority 1 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/expiry_time 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/custom_delivery 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/sender_visibility 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/delivery_report 1 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/read_reply 1 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/keep_copy 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/body_replying 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/hide_recipients 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/report_allowed 1 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/reply_charging 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/reply_charging_deadline 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/reply_charging_size 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/delivery_time 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/creation_mode 2 -u $vcuid

# MMS Receive Options
vconftool set -t int db/private/msg-service/mms_recv/home_network 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_recv/abroad_network 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_recv/read_receipt 1 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_recv/delivery_receipt 1 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_recv/reject_unknown 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_recv/reject_advertisement 0 -u $vcuid

# MMS Receive Options
vconftool set -t int db/private/msg-service/mms_style/font_size 30 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_style/font_style/bold 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_style/font_style/italic 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_style/font_style/underline 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/font_color/red 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/font_color/green 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/font_color/blue 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/font_color/hue 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/bg_color/red 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/bg_color/green 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/bg_color/blue 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/bg_color/hue 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/page_dur 2 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/page_custom_dur 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/page_dur_manual 0 -u $vcuid

# Push Msg Options
vconftool set -t bool db/private/msg-service/push_msg/recv_option 1 -u $vcuid
vconftool set -t int db/private/msg-service/push_msg/service_load 1 -u $vcuid

# CB Msg Options
vconftool set -t bool db/private/msg-service/cb_msg/receive 1 -f -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/save 1 -f -u $vcuid
vconftool set -t int db/private/msg-service/cb_msg/max_sim_count 0 -u $vcuid
vconftool set -t int db/private/msg-service/cb_msg/channel_count 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/0 1 -f -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/1 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/2 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/3 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/4 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/5 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/6 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/7 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/8 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/9 0 -u $vcuid

# Voice Mail Options
vconftool set -t string db/private/msg-service/voice_mail/voice_mail_number "5500" -f -u $vcuid
vconftool set -t int db/private/msg-service/voice_mail/voice_mail_count 0 -u $vcuid

# MMS Size Options
vconftool set -t int db/private/msg-service/size_opt/msg_size 300 -u $vcuid

# SIM message count
vconftool set -t int db/private/msg-service/sim_count/used_cnt 0 -u $vcuid
vconftool set -t int db/private/msg-service/sim_count/total_cnt 0 -u $vcuid

# SIM information
vconftool set -t int memory/private/msg-service/sim_changed 0 -i -u $vcuid
vconftool set -t string memory/private/msg-service/sim_imsi "" -i -u $vcuid
vconftool set -t bool memory/private/msg-service/national_sim 0 -i -u $vcuid
vconftool set -t string memory/private/msg-service/msisdn "" -i -u $vcuid

vconftool set -t int db/private/msg-service/notification_priv_id 0 -u $vcuid

/sbin/ldconfig
/bin/systemctl daemon-reload
if [ "$1" = "1" ]; then
    systemctl stop msg-service.service
fi

%postun -p /sbin/ldconfig

%postun tools -p /sbin/ldconfig
%postun -n sms-plugin -p /sbin/ldconfig
%postun -n mms-plugin -p /sbin/ldconfig

%files
%manifest msg-service.manifest
%defattr(-,root,root,-)
%dir %attr(775,root,db_msg_service) /opt/usr/data/msg-service
%{_libdir}/libmsg_plugin_manager.so
%{_libdir}/libmsg_mapi.so.*
%{_libdir}/libmsg_framework_handler.so
%{_libdir}/libmsg_transaction_manager.so
%{_libdir}/libmsg_utils.so
%{_libdir}/libmsg_transaction_proxy.so
%{_libdir}/libmsg_vobject.so
/usr/share/license/msg-service/LICENSE.Flora

%files devel
%defattr(-,root,root,-)
%{_libdir}/libmsg_mapi.so
%{_libdir}/pkgconfig/msg-service.pc
%{_includedir}/msg-service/*

%files tools
%manifest msg-service-tools.manifest
%defattr(-,root,root,-)
%{_bindir}/msg-helper
%{_bindir}/msg-server
%{_datadir}/media/Sherbet.wav
%attr(0644,root,root)/usr/share/msg-service/plugin.cfg
%{_sysconfdir}/rc.d/init.d/msg-server
%{_sysconfdir}/rc.d/rc3.d/S70msg-server
%{_sysconfdir}/rc.d/rc5.d/S70msg-server
/usr/lib/systemd/user/msg-service.service
/usr/lib/systemd/user/msg-service-log.service
/usr/lib/systemd/user/tizen-middleware.target.wants/msg-service.service
/usr/lib/systemd/user/tizen-middleware.target.wants/msg-service-log.service
/usr/share/license/msg-service/LICENSE.Flora
/opt/etc/smack/accesses.d/msg-service.rule
/etc/config/sysinfo-message.xml

%attr(0755,app,app)/var/log/msgfw


%files -n sms-plugin
%manifest sms-plugin.manifest
%defattr(-,root,root,-)
%{_libdir}/libmsg_sms_plugin.so
/usr/share/license/msg-service/LICENSE.Flora

%files -n mms-plugin
%manifest mms-plugin.manifest
%defattr(-,root,root,-)
%{_libdir}/libmsg_mms_plugin.so
/usr/share/license/msg-service/LICENSE.Flora

%changelog
* Wed Oct 25 2012 Sangkoo Kim <sangkoo.kim@samsung.com>
- New release version

* Wed Aug 8 2012 KeeBum Kim <keebum.kim@samsung.com>
- Apply New TAPI.
- Modify transaction data size of sos recipient list.

* Tue Aug 7 2012 KeeBum Kim <keebum.kim@samsung.com>
- Fix contact sync defect.
- New MessageFW API.

* Fri Jul 27 2012 KeeBum Kim <keebum.kim@samsung.com>
- Change devman_haptic.h to devman_managed.h.
- Modify to set MSG_SERVER_READY before sim status check.
- Fix bug in sim message save related operations.
- Limit sim related APIs not to work on sim not available status.
- Modify indicator icon image path & related.

* Tue Jul 17 2012 KeeBum Kim <keebum.kim@samsung.com>
- Modify MAX_SEGMENT_NUM to 15.
- Modify MMplayer related to support interrupted event.
- Fix bug in storage change callback which could cause on submit request.

* Fri Jun 29 2012 Sangkoo Kim <sangkoo.kim@samsung.com>
- Modify vconf key path for sos sending option.

* Mon Jun 18 2012 Seunghwan Lee <sh.cat.lee@samsung.com>
- Storage change callback for scheduled message
- code chage to support glib2-2.32
- Apply Backup && Restore of Mms Type

* Fri Jun 15 2012 Sangkoo Kim <sangkoo.kim@samsung.com>
- Update display time for scheduled message when it is sent.

* Thu Jun 14 2012 Seunghwan Lee <sh.cat.lee@samsung.com>
- Comment the test app in CMakefile.
- Modify MMS BG color of vconf value.
- Bug fixed wrong query in MsgStoCheckReadReportIsSent
- Add '-i' option for vconf keys of memory type
- Add smsc vconf keys(default value).
- To avoid msg incoming sound and notification on SOS

* Fri Jun 8 2012 Sangkoo Kim <sangkoo.kim@samsung.com>
- Add '-i' option for vconf keys of memory type.
- Add smsc vconf keys(default value).
- To avoid msg incoming sound and notification on SOS state.
- Change BGcolor to black && font colore to white.

* Wed May 31 2012 Keebum Kim <keebum.kim@samsung.com>
- Apply Mdm policy.
- SOS message.
- Fix horizontality development problem.
- Remove vconf key for new message count. (db/badge/com.samsung.message)
- Change vconf key path of "mms_send/msg_class" to private.

* Thu May 24 2012 Keebum Kim <keebum.kim@samsung.com>
- Add new MAPI.

* Fri May 18 2012 Sangkoo Kim <sangkoo.kim@samsung.com>
- Modify to support new DRM service.
- Rename private vconf keys.
- Modify .postinst and .spec file to pre-install used vconf keys.
- Remove compile warnings.
- Fix bug in manual retrieve operation.

* Fri May 11 2012 Jaeyun Jeong <jyjeong@samsung.com>
- Fix DB conflict which cause db lock.
- Fix bug in Find My Mobile function.
- Add '*' and '#' symbol as valid number.

* Wed May 9 2012 Keebum Kim <keebum.kim@samsung.com>
- Change mobile tracker related vconf key define values.
- Apply MDM policy for allowing text messaging.
- Fix bug for getting thumbnail path.
- Enable functionality of scheduled message.
- Change alarm setting of scheduled message from volatile type to non-volatile.
- Fix error in search query.

* Thu May 3 2012 Keebum Kim <keebum.kim@samsung.com>
- Change some thread list related APIs to support DB change.
- DB change to support multiple recipient.

* Thu Apr 19 2012 Keebum Kim <keebum.kim@samsung.com>
- Modify to manage contact sync time by vconf.
- Use g_idle_add() routine for updating unread message count(vconf values).
- apply try{}catch{} code for handling unexpected exception to avoid thread terminating.
- Fix bug for allocated size of replacing string.
- Resolve search problem for special characters.
- add xmlFree.

* Tue Apr 10 2012 Keebum Kim <keebum.kim@samsung.com>
- Remove unused vconf keys.
- Initialize SMSC selected_index.
- Remove systemd related.

* Thu Apr 05 2012 Jaeyun Jeong <jyjeong@samsung.com>
- Add notification property(NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE)
- Fix S1-2397/2417/2418/2419.
- Remove sent status callback check during submit request.
- Modify offset and limit operation on search.
- Remove invalid folder and file.
- Change browser launching API from aul to service-capi.
- Remove unused file.
- Invalid type checking is fixed.

* Fri Mar 16 2012 Jaeyun Jeong <jyjeong@samsung.com>
- Add #include <sys/stat.h> to support chmod related defines.
- Fix DRM content issue(unregistered mo content)

* Wed Mar 14 2012 Jaeyun Jeong <jyjeong@samsung.com>
- Modify plugin configuration file location for FOTA.
- Remove the db query which create sample data.

* Wed Feb 29 2012 Jaeyun Jeong <jyjeong@samsung.com>
- Update msg-service.spec for OBS.
- Fix TC execute failure.
- Fix S1-1419(Removed mms raw file issue after rebooting)
