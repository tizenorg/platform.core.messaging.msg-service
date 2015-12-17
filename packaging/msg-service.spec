Name:           msg-service
Version:        1.0.2
Release:        2
License:        Apache-2.0
Summary:        Messaging Framework Library
Group:          System/Libraries
Source0:        %{name}-%{version}.tar.gz
Source1:        msg-server.service
Source2:        msg-server.socket

%if "%{?profile}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

Requires(post): /usr/bin/sqlite3
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires(post): systemd
Requires(postun): systemd
BuildRequires: cmake
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(badge)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(capi-media-image-util)
BuildRequires: pkgconfig(capi-media-metadata-extractor)
BuildRequires: pkgconfig(capi-media-thumbnail-util)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-telephony)
BuildRequires: pkgconfig(contacts-service2)
BuildRequires: pkgconfig(cynara-client)
BuildRequires: pkgconfig(cynara-creds-commons)
BuildRequires: pkgconfig(cynara-creds-socket)
BuildRequires: pkgconfig(cynara-session)
BuildRequires: pkgconfig(deviced)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(eventsystem)
BuildRequires: pkgconfig(feedback)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(icu-uc)
BuildRequires: pkgconfig(iniparser)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(lbs-dbus)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(libwbxml2)
BuildRequires: pkgconfig(motion)
BuildRequires: pkgconfig(mm-fileinfo)
BuildRequires: pkgconfig(mm-player)
BuildRequires: pkgconfig(mm-session)
BuildRequires: pkgconfig(mm-sound)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(privacy-manager-client)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(sqlite3)
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
%if "%{?tizen_profile_name}" == "wearable"
-D_MSG_WEARABLE_PROFILE:BOOL=ON \
%else
-D_MSG_WEARABLE_PROFILE:BOOL=OFF \
%endif

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/etc/config

%make_install

mkdir -p %{buildroot}%{_unitdir}/multi-user.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir}/msg-server.service
%install_service multi-user.target.wants msg-server.service

mkdir -p %{buildroot}%{_unitdir}/sockets.target.wants
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir}/msg-server.socket
%install_service sockets.target.wants msg-server.socket

mkdir -p %{buildroot}/usr/dbspace
sqlite3 %{buildroot}/usr/dbspace/.msg_service.db "PRAGMA journal_mode = PERSIST;"
sqlite3 %{buildroot}/usr/dbspace/.msg_service.db < %{buildroot}/usr/share/msg-service/msg-service-db.sql

rm %{buildroot}/usr/share/msg-service/msg-service-db.sql

%post tools
/sbin/ldconfig

chmod 660 /usr/dbspace/.msg_service.db
chmod 660 /usr/dbspace/.msg_service.db-journal

chgrp priv_message_read /usr/dbspace/.msg_service.db
chmod o= /usr/dbspace/.msg_service.db
chsmack -a "*" /usr/dbspace/.msg_service.db

mkdir -p -m 775 /opt/usr/data/msg-service
mkdir -p -m 775 /opt/usr/data/msg-service/msgdata
mkdir -p -m 775 /opt/usr/data/msg-service/smildata
mkdir -p -m 775 /opt/usr/data/msg-service/ipcdata
mkdir -p -m 775 /opt/usr/data/msg-service/msgdata/thumbnails

%post -n sms-plugin -p /sbin/ldconfig
%post -n mms-plugin -p /sbin/ldconfig

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%postun tools -p /sbin/ldconfig
%postun -n sms-plugin -p /sbin/ldconfig
%postun -n mms-plugin -p /sbin/ldconfig

%files
%manifest msg-service.manifest
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
%{_libdir}/libmsg_mapi.so
%{_libdir}/pkgconfig/msg-service.pc
%{_includedir}/msg-service/*

%files tools
%manifest msg-service-tools.manifest
%caps(cap_chown,cap_dac_override,cap_lease=eip) %{_bindir}/msg-server
%config(noreplace) /usr/dbspace/.msg_service.db*
%{_unitdir}/msg-server.service
%{_unitdir}/multi-user.target.wants/msg-server.service
%{_unitdir}/msg-server.socket
%{_unitdir}/sockets.target.wants/msg-server.socket
/usr/share/license/msg-service/LICENSE.APLv2
/etc/config/*

%files -n sms-plugin
%manifest sms-plugin.manifest
%{_libdir}/libmsg_sms_plugin.so
/usr/share/license/msg-service/LICENSE.APLv2

%files -n mms-plugin
%manifest mms-plugin.manifest
%{_libdir}/libmsg_mms_plugin.so
/usr/share/license/msg-service/LICENSE.APLv2

%changelog
