Name:           msg-service
Version:        1.0.1
Release:        1
License:        Apache-2.0
Summary:        Messaging Framework Library
Group:          System/Libraries
Source0:        %{name}-%{version}.tar.gz
Source1:        msg-server.service
Source2:        msg-server.socket

#%if "%{?tizen_profile_name}" == "wearable"
#ExcludeArch: %{arm} %ix86 x86_64
#%endif

Requires(post): /usr/bin/sqlite3
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires(post): systemd
Requires(postun): systemd
BuildRequires: cmake
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(badge)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-system-info)
#BuildRequires: pkgconfig(capi-telephony)
BuildRequires: pkgconfig(contacts-service2)
BuildRequires: pkgconfig(cynara-client)
BuildRequires: pkgconfig(cynara-creds-commons)
BuildRequires: pkgconfig(cynara-creds-socket)
BuildRequires: pkgconfig(cynara-session)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(deviced)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(feedback)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(iniparser)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(lbs-dbus)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(libsystemd-daemon)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(libwbxml2)
BuildRequires: pkgconfig(capi-media-thumbnail-util)
BuildRequires: pkgconfig(capi-media-image-util)
BuildRequires: pkgconfig(mm-fileinfo)
BuildRequires: pkgconfig(mm-player)
BuildRequires: pkgconfig(mm-session)
BuildRequires: pkgconfig(mm-sound)
BuildRequires: pkgconfig(mmutil-imgp)
BuildRequires: pkgconfig(mmutil-jpeg)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(privacy-manager-client)
BuildRequires: pkgconfig(sensor)
BuildRequires: pkgconfig(storage)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(vconf)

%description
Description: Messaging Framework Library


%package devel
License:        Apache-2.0
Summary:        Messaging Framework Library (development)
Requires:       %{name} = %{version}-%{release}
Group:          Development/Libraries

%description devel
Description: Messaging Framework Library (development)


%package tools
License:        Apache-2.0
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
License:        Apache-2.0
Summary:        SMS plugin library
Requires:       %{name} = %{version}-%{release}
Group:          System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description -n sms-plugin
Description: SMS plugin library

%package -n mms-plugin
License:        Apache-2.0
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
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} \
        -DLIB_INSTALL_DIR=%{_libdir} \
%ifarch i586
-D_TIZEN_I586_ENABLED:BOOL=ON \
%else
-D_TIZEN_I586_ENABLED:BOOL=OFF \
%endif
%if 0%{?sec_product_feature_container_enable} == 1
-D_FEATURE_CONTAINER_ENABLE:BOOL=ON \
%endif
%if "%{?tizen_profile_name}" == "wearable"
-D_MSG_WEARABLE_PROFILE:BOOL=ON
%else
-D_MSG_WEARABLE_PROFILE:BOOL=OFF
%endif

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/etc/config

%make_install

mkdir -p %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_libdir}/systemd/system/msg-server.service
ln -s ../msg-server.service %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants/msg-server.service

mkdir -p %{buildroot}%{_libdir}/systemd/system/sockets.target.wants
install -m 0644 %SOURCE2 %{buildroot}%{_libdir}/systemd/system/msg-server.socket
ln -s ../msg-server.socket %{buildroot}%{_libdir}/systemd/system/sockets.target.wants/msg-server.socket

mkdir -p %{buildroot}/opt/usr/dbspace
sqlite3 %{buildroot}/opt/usr/dbspace/.msg_service.db "PRAGMA journal_mode = PERSIST;"
sqlite3 %{buildroot}/opt/usr/dbspace/.msg_service.db < %{buildroot}/usr/share/msg-service/msg-service-db.sql

rm %{buildroot}/usr/share/msg-service/msg-service-db.sql

%post tools
/sbin/ldconfig

chown 200:5000 /opt/usr/dbspace/.msg_service.db
chown 200:5000 /opt/usr/dbspace/.msg_service.db-journal
chown 200:200 %{_libdir}/systemd/system/msg-server.socket

chmod 660 /opt/usr/dbspace/.msg_service.db
chmod 660 /opt/usr/dbspace/.msg_service.db-journal

if [ -f %{_libdir}/rpm-plugins/msm.so ]
then
	chsmack -a 'msg-service::db' /opt/usr/dbspace/.msg_service.db*
fi

%post -n sms-plugin -p /sbin/ldconfig
%post -n mms-plugin -p /sbin/ldconfig

%post
/sbin/ldconfig

mkdir -p /opt/usr/data/msg-service
chgrp db_msg_service /opt/usr/data/msg-service


########## Setting Config Value (Internal keys) ##########
# Message Server Status
vconftool set -t bool memory/msg/ready 0 -i -u 200 -g 5000 -s system::vconf_inhouse

# SMS Send Options
vconftool set -t int db/msg/network_mode 2 -u 200 -g 5000 -s system::vconf_inhouse

# New Message Count
vconftool set -t int db/msg/recv_sms 0 -u 200 -g 5000 -s system::vconf_inhouse
vconftool set -t int db/msg/recv_mms 0 -u 200 -g 5000 -s system::vconf_inhouse

########## Setting Config Value (Private keys) ##########
# General Options
vconftool set -t bool db/private/msg-service/general/keep_copy 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/general/auto_erase 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/general/block_msg 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/contact_sync_time 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/alert_rep_type 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/search_tags 31 -s msg-service::vconf -u 200 -g 5000

vconftool set -t int db/private/msg-service/general/page_limit 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/sms_mms_threshold 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/max_recipient 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/sms_storage 0 -s msg-service::vconf -u 200 -g 5000

vconftool set -t bool db/private/msg-service/general/block_unknown_msg 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/sms_limit 1000 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/mms_limit 100 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/general/notification 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/general/vibration 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/general/preview 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/general/ringtone_type 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t string db/private/msg-service/general/ringtone_path "" -s msg-service::vconf -u 200 -g 5000

vconftool set -t string db/private/msg-service/general/ua_profile "" -s msg-service::vconf -u 200 -g 5000

vconftool set -t int db/private/msg-service/general/msg_id_counter 0 -s msg-service::vconf -u 200 -g 5000

# SMS Send Options
vconftool set -t int db/private/msg-service/sms_send/dcs 3 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/sms_send/reply_path 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/sms_send/delivery_report 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sms_send/save_storage 1 -s msg-service::vconf -u 200 -g 5000

# MMS Send Options
vconftool set -t int db/private/msg-service/mms_send/msg_class 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_send/priority 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_send/expiry_time 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_send/custom_delivery 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_send/sender_visibility 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_send/delivery_report 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_send/read_reply 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_send/keep_copy 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_send/body_replying 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_send/hide_recipients 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_send/report_allowed 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_send/reply_charging 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_send/reply_charging_deadline 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_send/reply_charging_size 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_send/delivery_time 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_send/creation_mode 2 -s msg-service::vconf -u 200 -g 5000

# MMS Receive Options
vconftool set -t int db/private/msg-service/mms_recv/home_network 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_recv/abroad_network 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_recv/read_receipt 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_recv/delivery_receipt 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_recv/reject_unknown 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_recv/reject_advertisement 0 -s msg-service::vconf -u 200 -g 5000

# MMS Style Options
vconftool set -t int db/private/msg-service/mms_style/font_size 30 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_style/font_style/bold 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_style/font_style/italic 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/mms_style/font_style/underline 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/font_color/red 255 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/font_color/green 255 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/font_color/blue 255 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/font_color/hue 255 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/bg_color/red 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/bg_color/green 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/bg_color/blue 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/bg_color/hue 255 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/page_dur 2 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/page_custom_dur 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/mms_style/page_dur_manual 0 -s msg-service::vconf -u 200 -g 5000

# Push Msg Options
vconftool set -t bool db/private/msg-service/push_msg/recv_option 1 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/push_msg/service_load 1 -s msg-service::vconf -u 200 -g 5000

# CB Msg Options
vconftool set -t bool db/private/msg-service/cb_msg/receive/1 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/receive/2 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/save 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/cb_msg/max_sim_count/1 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/cb_msg/max_sim_count/2 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/cb_msg/max_sim_count/3 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/0 1 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/1 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/2 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/3 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/4 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/5 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/6 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/7 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/8 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool db/private/msg-service/cb_msg/language/9 0 -s msg-service::vconf -u 200 -g 5000

# Voice Mail Options
vconftool set -t string db/private/msg-service/voice_mail/voice_mail_number/1 "" -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/voice_mail/voice_mail_count/1 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string db/private/msg-service/voice_mail/voice_mail_alphaid/1 "" -f -s msg-service::vconf -u 200 -g 5000

vconftool set -t string db/private/msg-service/voice_mail/voice_mail_number/2 "" -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/voice_mail/voice_mail_count/2 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string db/private/msg-service/voice_mail/voice_mail_alphaid/2 "" -f -s msg-service::vconf -u 200 -g 5000

vconftool set -t string db/private/msg-service/voice_mail/voice_mail_number/3 "" -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/voice_mail/voice_mail_count/3 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string db/private/msg-service/voice_mail/voice_mail_alphaid/3 "" -f -s msg-service::vconf -u 200 -g 5000

# MMS Size Options
vconftool set -t int db/private/msg-service/size_opt/msg_size 300 -s msg-service::vconf -u 200 -g 5000

# SIM message count
vconftool set -t int db/private/msg-service/sim_count/used_cnt/1 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sim_count/total_cnt/1 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sim_count/used_cnt/2 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sim_count/total_cnt/2 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sim_count/used_cnt/3 0 -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sim_count/total_cnt/3 0 -f -s msg-service::vconf -u 200 -g 5000

# SIM information
vconftool set -t int memory/private/msg-service/sim_changed/1 0 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/sim_subs_id/1 "" -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool memory/private/msg-service/national_sim/1 0 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/msisdn/1 "" -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/iccid/1 "" -i -f -s msg-service::vconf -u 200 -g 5000

vconftool set -t int memory/private/msg-service/sim_changed/2 0 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/sim_subs_id/2 "" -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool memory/private/msg-service/national_sim/2 0 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/msisdn/2 "" -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/iccid/2 "" -i -f -s msg-service::vconf -u 200 -g 5000

vconftool set -t int memory/private/msg-service/sim_changed/3 0 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/sim_subs_id/3 "" -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool memory/private/msg-service/national_sim/3 0 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/msisdn/3 "" -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t string memory/private/msg-service/iccid/3 "" -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t int  memory/private/msg-service/default_network_sim 0 -i -f -s msg-service::vconf -u 200 -g 5000

# SST information
vconftool set -t bool memory/private/msg-service/sim_st/1 1 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool memory/private/msg-service/sim_mo_ctrl/1 0 -i -f -s msg-service::vconf -u 200 -g 5000

vconftool set -t bool memory/private/msg-service/sim_st/2 1 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool memory/private/msg-service/sim_mo_ctrl/2 0 -i -f -s msg-service::vconf -u 200 -g 5000

vconftool set -t bool memory/private/msg-service/sim_st/3 1 -i -f -s msg-service::vconf -u 200 -g 5000
vconftool set -t bool memory/private/msg-service/sim_mo_ctrl/3 0 -i -f -s msg-service::vconf -u 200 -g 5000

# Notification
vconftool set -t int db/private/msg-service/notification_priv_id 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/voice_noti_id1 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/voice_noti_id2 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/cb_noti_priv_id 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sim_msg_noti_priv_id 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/emergency_noti_id 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sentfail_noti_id 0 -s msg-service::vconf -u 200 -g 5000
vconftool set -t int db/private/msg-service/sim_full_noti_id 0 -s msg-service::vconf -u 200 -g 5000


%postun -p /sbin/ldconfig

%postun tools -p /sbin/ldconfig
%postun -n sms-plugin -p /sbin/ldconfig
%postun -n mms-plugin -p /sbin/ldconfig

%files
%manifest msg-service.manifest
%defattr(-,system,system,-)
%{_libdir}/libmsg_plugin_manager.so
%{_libdir}/libmsg_mapi.so.*
%{_libdir}/libmsg_framework_handler.so
%{_libdir}/libmsg_transaction_manager.so
%{_libdir}/libmsg_utils.so
%{_libdir}/libmsg_externals.so
%{_libdir}/libmsg_transaction_proxy.so
%{_libdir}/libmsg_vobject.so
/usr/share/license/msg-service/LICENSE.APLv2

%files devel
%defattr(-,system,system,-)
%{_libdir}/libmsg_mapi.so
%{_libdir}/pkgconfig/msg-service.pc
%{_includedir}/msg-service/*

%files tools
%manifest msg-service-tools.manifest
%defattr(-,system,system,-)
%{_bindir}/msg-server
%config(noreplace) /opt/usr/dbspace/.msg_service.db*
%{_libdir}/systemd/system/msg-server.service
%{_libdir}/systemd/system/multi-user.target.wants/msg-server.service
%{_libdir}/systemd/system/sockets.target.wants/msg-server.socket
%{_libdir}/systemd/system/msg-server.socket
/usr/share/license/msg-service/LICENSE.APLv2
/etc/smack/accesses.d/msg-service.rule
/etc/config/*

%files -n sms-plugin
%manifest sms-plugin.manifest
%defattr(-,system,system,-)
%{_libdir}/libmsg_sms_plugin.so
/usr/share/license/msg-service/LICENSE.APLv2

%files -n mms-plugin
%manifest mms-plugin.manifest
%defattr(-,system,system,-)
%{_libdir}/libmsg_mms_plugin.so
/usr/share/license/msg-service/LICENSE.APLv2

%changelog
