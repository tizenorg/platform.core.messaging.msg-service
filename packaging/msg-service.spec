Name:           msg-service
Version:        0.8.7
Release:        5
License:        Samsung Proprietary
Summary:        Messaging Framework Library 
Group:          System/Libraries
Source0:	%{name}-%{version}.tar.gz
Source101:      msg-service.service
Source1001:     msg-service.manifest 

Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake

BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(contacts-service)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(devman_haptic)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(drm-service)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(libwbxml2)
BuildRequires: pkgconfig(media-thumbnail)
BuildRequires: pkgconfig(mm-fileinfo)
BuildRequires: pkgconfig(mm-player)
BuildRequires: pkgconfig(mm-session)
BuildRequires: pkgconfig(network)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(pmapi)
BuildRequires: pkgconfig(mmutil-imgp)
BuildRequires: pkgconfig(mmutil-jpeg)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(sensor)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(svi)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(capi-appfw-application)

%description 
Description: Messaging Framework Library 


%package devel
License:        Samsung Proprietary
Summary:        Messaging Framework Library (development)
Requires:       %{name} = %{version}-%{release}
Group:          Development/Libraries

%description devel
Description: Messaging Framework Library (development)


%package tools
License:        Samsung Proprietary
Summary:        Messaging server application
Requires:       %{name} = %{version}-%{release}
Group:          TO_BU / FILL_IN
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description tools
Description:  Messaging server application


%package -n sms-plugin
License:        Samsung Proprietary
Summary:        SMS plugin library 
Requires:       %{name} = %{version}-%{release}
Group:          System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description -n sms-plugin
Description: SMS plugin library  

%package -n mms-plugin
License:        Samsung Proprietary
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
cp %{SOURCE1001} .
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install

rm -rf %{buildroot}
%make_install

mkdir -p  %{buildroot}%{_sysconfdir}/rc.d/rc3.d
ln -s %{_sysconfdir}/rc.d/init.d/msg-server  %{buildroot}%{_sysconfdir}/rc.d/rc3.d/S70msg-server
mkdir -p  %{buildroot}%{_sysconfdir}/rc.d/rc5.d
ln -s %{_sysconfdir}/rc.d/init.d/msg-server  %{buildroot}%{_sysconfdir}/rc.d/rc5.d/S70msg-server

mkdir -p %{buildroot}/opt/data/msg-service

if [ ! -f %{buildroot}/opt/dbspace/.msg_service.db ]
then
        mkdir -p %{buildroot}/opt/dbspace/
        sqlite3 %{buildroot}/opt/dbspace/.msg_service.db "PRAGMA journal_mode = PERSIST;

        CREATE TABLE MSG_ADDRESS_TABLE(ADDRESS_ID INTEGER PRIMARY KEY, ADDRESS_TYPE INTEGER, RECIPIENT_TYPE INTEGER, ADDRESS_VAL TEXT, CONTACT_ID INTEGER, DISPLAY_NAME TEXT, FIRST_NAME TEXT, LAST_NAME TEXT, IMAGE_PATH TEXT, SYNC_TIME DATETIME, UNREAD_CNT INTEGER DEFAULT 0, SMS_CNT INTEGER DEFAULT 0, MMS_CNT INTEGER DEFAULT 0, MAIN_TYPE INTEGER NOT NULL, SUB_TYPE INTEGER NOT NULL, MSG_DIRECTION INTEGER NOT NULL, MSG_TIME DATETIME, MSG_TEXT TEXT);
        CREATE TABLE MSG_FOLDER_TABLE(FOLDER_ID INTEGER PRIMARY KEY, FOLDER_NAME TEXT NOT NULL, FOLDER_TYPE INTEGER DEFAULT 0);
        CREATE TABLE MSG_MESSAGE_TABLE(MSG_ID INTEGER PRIMARY KEY, ADDRESS_ID INTEGER, FOLDER_ID INTEGER, REFERENCE_ID INTEGER, STORAGE_ID INTEGER NOT NULL, MAIN_TYPE INTEGER NOT NULL, SUB_TYPE INTEGER NOT NULL, DISPLAY_TIME DATETIME, DATA_SIZE INTEGER DEFAULT 0, NETWORK_STATUS INTEGER DEFAULT 0, READ_STATUS INTEGER DEFAULT 0, PROTECTED INTEGER DEFAULT 0, PRIORITY INTEGER DEFAULT 0, MSG_DIRECTION INTEGER NOT NULL, SCHEDULED_TIME DATETIME, BACKUP INTEGER DEFAULT 0, SUBJECT TEXT, MSG_DATA TEXT, THUMB_PATH TEXT, MSG_TEXT TEXT, DELIVERY_REPORT_STATUS INTEGER DEFAULT 0, DELIVERY_REPORT_TIME DATETIME, READ_REPORT_STATUS INTEGER DEFAULT 0, READ_REPORT_TIME DATETIME, ATTACHMENT_COUNT INTEGER DEFAULT 0, FOREIGN KEY(ADDRESS_ID) REFERENCES MSG_ADDRESS_TABLE(ADDRESS_ID), FOREIGN KEY(FOLDER_ID) REFERENCES MSG_FOLDER_TABLE(FOLDER_ID));
        CREATE TABLE MSG_SIM_TABLE(MSG_ID INTEGER, SIM_ID INTEGER NOT NULL, FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID));
        CREATE TABLE MSG_PUSH_TABLE(MSG_ID INTEGER, ACTION INTEGER, CREATED INTEGER, EXPIRES INTEGER, ID TEXT, HREF TEXT, CONTENT TEXT, FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID));
        CREATE TABLE MSG_CBMSG_TABLE(MSG_ID INTEGER, CB_MSG_ID INTEGER NOT NULL, FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID));
        CREATE TABLE MSG_SYNCML_TABLE(MSG_ID INTEGER, EXT_ID INTEGER NOT NULL, PINCODE INTEGER NOT NULL, FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID));
        CREATE TABLE MSG_SCHEDULED_TABLE(MSG_ID INTEGER, ALARM_ID INTEGER NOT NULL, LISTENER_FD INTEGER NOT NULL, FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID));
        CREATE TABLE MSG_SMS_SENDOPT_TABLE(MSG_ID INTEGER, DELREP_REQ INTEGER NOT NULL, KEEP_COPY INTEGER NOT NULL, REPLY_PATH INTEGER NOT NULL, FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID));
        CREATE TABLE MSG_FILTER_TABLE(FILTER_ID INTEGER PRIMARY KEY, FILTER_TYPE INTEGER NOT NULL, FILTER_VALUE TEXT NOT NULL);
        CREATE TABLE MSG_MMS_MESSAGE_TABLE(REFERENCE_ID INTEGER, TRANSACTION_ID TEXT, MESSAGE_ID TEXT, FWD_MESSAGE_ID TEXT, CONTENTS_LOCATION TEXT, FILE_PATH TEXT, FOREIGN KEY(REFERENCE_ID) REFERENCES MSG_MESSAGE_TABLE(REFERENCE_ID));
        CREATE TABLE MSG_MMS_ATTR_TABLE(REFERENCE_ID INTEGER, VERSION INTEGER NOT NULL, DATA_TYPE INTEGER DEFAULT -1, DATE DATETIME, HIDE_ADDRESS INTEGER DEFAULT 0, ASK_DELIVERY_REPORT INTEGER DEFAULT 0, REPORT_ALLOWED INTEGER DEFAULT 0, READ_REPORT_ALLOWED_TYPE INTEGER DEFAULT 0, ASK_READ_REPLY INTEGER DEFAULT 0, READ INTEGER DEFAULT 0, READ_REPORT_SEND_STATUS INTEGER DEFAULT 0, READ_REPORT_SENT INTEGER DEFAULT 0, PRIORITY INTEGER DEFAULT 0, KEEP_COPY INTEGER DEFAULT 0, MSG_SIZE INTEGER NOT NULL, MSG_CLASS INTEGER DEFAULT -1, EXPIRY_TIME DATETIME, CUSTOM_DELIVERY_TIME INTEGER DEFAULT 0, DELIVERY_TIME DATETIME, MSG_STATUS INTEGER DEFAULT -1, FOREIGN KEY(REFERENCE_ID) REFERENCES MSG_MESSAGE_TABLE(REFERENCE_ID));

        CREATE INDEX MSG_ADDRESS_INDEX ON MSG_ADDRESS_TABLE(ADDRESS_ID);
        CREATE INDEX MSG_FOLDER_INDEX ON MSG_FOLDER_TABLE(FOLDER_ID);
        CREATE INDEX MSG_MESSAGE_INDEX ON MSG_MESSAGE_TABLE(MSG_ID, ADDRESS_ID, FOLDER_ID);

        INSERT INTO MSG_FOLDER_TABLE VALUES (1, 'INBOX', 1);
        INSERT INTO MSG_FOLDER_TABLE VALUES (2, 'OUTBOX', 2);
        INSERT INTO MSG_FOLDER_TABLE VALUES (3, 'SENTBOX', 2);
        INSERT INTO MSG_FOLDER_TABLE VALUES (4, 'DRAFT', 3);
        INSERT INTO MSG_FOLDER_TABLE VALUES (5, 'CBMSGBOX', 1);
        INSERT INTO MSG_FOLDER_TABLE VALUES (6, 'SPAMBOX', 4);
        INSERT INTO MSG_FOLDER_TABLE VALUES (7, 'SMS TEMPLATE', 5);
        INSERT INTO MSG_FOLDER_TABLE VALUES (8, 'MMS TEMPLATE', 5);

        INSERT INTO MSG_ADDRESS_TABLE VALUES (0, 0, 0, '', 0, '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, '');"
fi

mkdir -p %{buildroot}%{_libdir}/systemd/user/tizen-middleware.target.wants
install -m 0644 %SOURCE101 %{buildroot}%{_libdir}/systemd/user/
ln -s ../msg-service.service %{buildroot}%{_libdir}/systemd/user/tizen-middleware.target.wants/msg-service.service


%post tools -p /sbin/ldconfig
%post -n sms-plugin -p /sbin/ldconfig
%post -n mms-plugin -p /sbin/ldconfig

%post 
/sbin/ldconfig

########## Setting Config Value ##########
# General Options
vconftool set -t bool db/msg/general/keep_copy 1
vconftool set -t int db/msg/general/alert_tone 0
vconftool set -t bool db/msg/general/auto_erase 0
vconftool set -t bool db/msg/general/block_msg 0

# SMS Send Options
vconftool set -t int db/msg/sms_send/dcs 3
vconftool set -t int db/msg/network_mode 2
vconftool set -t bool db/msg/sms_send/reply_path 0
vconftool set -t bool db/msg/sms_send/delivery_report 0
vconftool set -t int db/msg/sms_send/save_storage 1

# MMS Send Options
vconftool set -t int db/msg/mms_send/msg_class 0
vconftool set -t int db/msg/mms_send/priority 1
vconftool set -t int db/msg/mms_send/expiry_time 0
vconftool set -t int db/msg/mms_send/custom_delivery 0
vconftool set -t bool db/msg/mms_send/sender_visibility 0
vconftool set -t bool db/msg/mms_send/delivery_report 1
vconftool set -t bool db/msg/mms_send/read_reply 1
vconftool set -t bool db/msg/mms_send/keep_copy 0
vconftool set -t bool db/msg/mms_send/body_replying 0
vconftool set -t bool db/msg/mms_send/hide_recipients 0
vconftool set -t bool db/msg/mms_send/report_allowed 1
vconftool set -t int db/msg/mms_send/reply_charging 0
vconftool set -t int db/msg/mms_send/reply_charging_deadline 0
vconftool set -t int db/msg/mms_send/reply_charging_size 0
vconftool set -t int db/msg/mms_send/delivery_time 0
vconftool set -t int db/msg/mms_send/creation_mode 2

# MMS Receive Options
vconftool set -t int db/msg/mms_recv/home_network 0
vconftool set -t int db/msg/mms_recv/abroad_network 0
vconftool set -t bool db/msg/mms_recv/read_receipt 1
vconftool set -t bool db/msg/mms_recv/delivery_receipt 1
vconftool set -t bool db/msg/mms_recv/reject_unknown 0
vconftool set -t bool db/msg/mms_recv/reject_advertisement 0

# MMS Receive Options
vconftool set -t int db/msg/mms_style/font_size 30
vconftool set -t bool db/msg/mms_style/font_style/bold 0
vconftool set -t bool db/msg/mms_style/font_style/italic 0
vconftool set -t bool db/msg/mms_style/font_style/underline 0
vconftool set -t int db/msg/mms_style/font_color/red 0
vconftool set -t int db/msg/mms_style/font_color/green 0
vconftool set -t int db/msg/mms_style/font_color/blue 0
vconftool set -t int db/msg/mms_style/font_color/hue 255
vconftool set -t int db/msg/mms_style/bg_color/red 255
vconftool set -t int db/msg/mms_style/bg_color/green 255
vconftool set -t int db/msg/mms_style/bg_color/blue 255
vconftool set -t int db/msg/mms_style/bg_color/hue 255
vconftool set -t int db/msg/mms_style/page_dur 2
vconftool set -t int db/msg/mms_style/page_custom_dur 0
vconftool set -t int db/msg/mms_style/page_dur_manual 0

# Push Msg Options
vconftool set -t bool db/msg/push_msg/recv_option 1
vconftool set -t int db/msg/push_msg/service_load 1

# CB Msg Options
vconftool set -t bool db/msg/cb_msg/receive 0
vconftool set -t bool db/msg/cb_msg/all_channel 0
vconftool set -t int db/msg/cb_msg/max_sim_count 0
vconftool set -t int db/msg/cb_msg/channel_count 0
vconftool set -t bool db/msg/cb_msg/language/0 0
vconftool set -t bool db/msg/cb_msg/language/1 0
vconftool set -t bool db/msg/cb_msg/language/2 0
vconftool set -t bool db/msg/cb_msg/language/3 0
vconftool set -t bool db/msg/cb_msg/language/4 0
vconftool set -t bool db/msg/cb_msg/language/5 0
vconftool set -t bool db/msg/cb_msg/language/6 0
vconftool set -t bool db/msg/cb_msg/language/7 0
vconftool set -t bool db/msg/cb_msg/language/8 0
vconftool set -t bool db/msg/cb_msg/language/9 0

# SOS Msg Options
vconftool set -t bool db/msg/sos_msg/send_option 0
vconftool set -t int db/msg/sos_msg/recipient_count 0
vconftool set -t int db/msg/sos_msg/repeat_count 0
vconftool set -t string db/msg/sos_msg/msg_text ""
vconftool set -t int db/msg/sos_msg/alert_type 0

# Voice Mail Options
vconftool set -t string db/msg/voice_mail/voice_mail_number "12345678"

# MMS Size Options
vconftool set -t int db/msg/size_opt/msg_size 300

vconftool set -t int db/badge/org.tizen.message 0

# Msg Count
vconftool set -t int db/msg/recv_sms 0 -u 0
vconftool set -t int db/msg/recv_mms 0 -u 0

%postun -p /sbin/ldconfig

%postun tools -p /sbin/ldconfig

%postun -n sms-plugin -p /sbin/ldconfig

%postun -n mms-plugin -p /sbin/ldconfig

%files
%manifest msg-service.manifest
%defattr(-,root,root,-)
%dir /opt/data/msg-service
%{_libdir}/libmsg_plugin_manager.so
%{_libdir}/libmsg_mapi.so.*
%{_libdir}/libmsg_framework_handler.so
%{_libdir}/libmsg_transaction_manager.so
%{_libdir}/libmsg_utils.so
%{_libdir}/libmsg_transaction_proxy.so
%{_sysconfdir}/rc.d/init.d/msg-server
%{_sysconfdir}/rc.d/rc3.d/S70msg-server
%{_sysconfdir}/rc.d/rc5.d/S70msg-server
%config(noreplace) %attr(0660,-,db_msg_service) /opt/dbspace/.msg_service.db
%config(noreplace) %attr(0660,-,db_msg_service) /opt/dbspace/.msg_service.db-journal

%files devel
%manifest msg-service.manifest
%defattr(-,root,root,-)
%{_libdir}/libmsg_mapi.so
%{_libdir}/pkgconfig/msg-service.pc
%{_includedir}/msg-service/*

%files tools
%manifest msg-service.manifest
%defattr(-,root,root,-)
%{_bindir}/msg-helper
%{_bindir}/msg-test-app
%{_bindir}/msg-server
%{_datadir}/media/Sherbet.wav
%attr(0644,root,root)/usr/share/msg-service/plugin.cfg
%attr(0644,root,root)/opt/etc/msg-service/Temp0_2.txt
%attr(0644,root,root)/opt/etc/msg-service/Temp1_0.txt
%attr(0644,root,root)/opt/etc/msg-service/audio.amr
%attr(0644,root,root)/opt/etc/msg-service/P091120_104633.jpg
%attr(0644,root,root)/opt/etc/msg-service/A.smi
%attr(0644,root,root)/opt/etc/msg-service/V091120_104905.3gp
%attr(0644,root,root)/opt/etc/msg-service/alert_on_call.mp3
%{_libdir}/systemd/user/msg-service.service
%{_libdir}/systemd/user/tizen-middleware.target.wants/msg-service.service

%files -n sms-plugin
%manifest msg-service.manifest
%defattr(-,root,root,-)
%{_libdir}/libmsg_sms_plugin.so

%files -n mms-plugin
%manifest msg-service.manifest
%defattr(-,root,root,-)
%{_libdir}/libmsg_mms_plugin.so
%{_libdir}/libmsg_mms_language_pack.so
