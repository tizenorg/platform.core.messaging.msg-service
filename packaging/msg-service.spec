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
BuildRequires: libacl-devel
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(appcore-agent)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(badge)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(capi-media-image-util)
BuildRequires: pkgconfig(capi-media-metadata-extractor)
BuildRequires: pkgconfig(capi-media-player)
BuildRequires: pkgconfig(capi-media-sound-manager)
BuildRequires: pkgconfig(capi-media-thumbnail-util)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-telephony)
BuildRequires: pkgconfig(csr-framework)
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
BuildRequires: pkgconfig(libsystemd-login)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(libwbxml2)
BuildRequires: pkgconfig(motion)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(storage)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(vconf)
%if "%{?profile}" != "wearable"
BuildRequires: pkgconfig(contacts-service2)
%endif

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

%package -n msg-manager
License:        Apache-2.0
Summary:        Message manager application
Requires:       %{name} = %{version}-%{release}
Group:          Applications

%description -n msg-manager
Description: Message manager application

%if "%{?profile}" != "wearable"
%define APP_PKGNAME	org.tizen.msg-manager
%define APP_PREFIX	%{TZ_SYS_RO_APP}/%{APP_PKGNAME}
%define APP_BINDIR	%{APP_PREFIX}/bin
%define APP_MANIFESTDIR	%{TZ_SYS_RO_PACKAGES}
%endif

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} \
		-DLIB_INSTALL_DIR=%{_libdir} \
%if "%{?profile}" != "wearable"
		-DAPP_MANIFESTDIR=%{APP_MANIFESTDIR}   \
		-DAPP_BINDIR=%{APP_BINDIR}   \
%endif
		-DTZ_SYS_RO_APP=%TZ_SYS_RO_APP \
		-DTZ_SYS_DATA=%TZ_SYS_DATA \
		-DTZ_SYS_DB=%TZ_SYS_DB \
%ifarch i586
-D_TIZEN_I586_ENABLED:BOOL=ON \
%else
-D_TIZEN_I586_ENABLED:BOOL=OFF \
%endif
%if "%{?profile}" == "wearable"
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

mkdir -p %{buildroot}%{TZ_SYS_DB}
sqlite3 %{buildroot}%{TZ_SYS_DB}/.msg_service.db "PRAGMA journal_mode = PERSIST;"
sqlite3 %{buildroot}%{TZ_SYS_DB}/.msg_service.db < %{buildroot}/usr/share/msg-service/msg-service-db.sql

rm %{buildroot}/usr/share/msg-service/msg-service-db.sql

%post tools
/sbin/ldconfig

chmod 640 %{TZ_SYS_DB}/.msg_service.db
chmod 660 %{TZ_SYS_DB}/.msg_service.db-journal

mkdir -p -m 775 %{TZ_SYS_DATA}/msg-service
mkdir -p -m 770 %{TZ_SYS_DATA}/msg-service/msgdata
mkdir -p -m 770 %{TZ_SYS_DATA}/msg-service/smildata
mkdir -p -m 770 %{TZ_SYS_DATA}/msg-service/ipcdata
mkdir -p -m 770 %{TZ_SYS_DATA}/msg-service/msgdata/thumbnails


chgrp priv_message_read %{TZ_SYS_DB}/.msg_service.db
chgrp priv_message_read %{TZ_SYS_DATA}/msg-service/msgdata
chgrp priv_message_read %{TZ_SYS_DATA}/msg-service/smildata
chgrp priv_message_write %{TZ_SYS_DATA}/msg-service/ipcdata
chgrp priv_message_read %{TZ_SYS_DATA}/msg-service/msgdata/thumbnails


chsmack -a "*" %{TZ_SYS_DB}/.msg_service.db
chsmack -a "System::Shared" %{TZ_SYS_DATA}/msg-service/msgdata -t
chsmack -a "System::Shared" %{TZ_SYS_DATA}/msg-service/smildata -t
chsmack -a "System::Run" %{TZ_SYS_DATA}/msg-service/ipcdata -t
chsmack -a "System::Shared" %{TZ_SYS_DATA}/msg-service/msgdata/thumbnails -t

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
%license LICENSE.APLv2
%{_libdir}/libmsg_plugin_manager.so
%{_libdir}/libmsg_mapi.so.*
%{_libdir}/libmsg_framework_handler.so
%{_libdir}/libmsg_transaction_manager.so
%{_libdir}/libmsg_utils.so
%{_libdir}/libmsg_externals.so
%{_libdir}/libmsg_transaction_proxy.so
%{_libdir}/libmsg_vobject.so

%files devel
%license LICENSE.APLv2
%{_libdir}/libmsg_mapi.so
%{_libdir}/pkgconfig/msg-service.pc
%{_includedir}/msg-service/*

%files tools
%manifest msg-service-tools.manifest
%license LICENSE.APLv2
%caps(cap_chown,cap_dac_override,cap_lease=eip) %{_bindir}/msg-server
%config(noreplace) %{TZ_SYS_DB}/.msg_service.db*
%{_unitdir}/msg-server.service
%{_unitdir}/multi-user.target.wants/msg-server.service
%{_unitdir}/msg-server.socket
%{_unitdir}/sockets.target.wants/msg-server.socket
/etc/config/*

%files -n sms-plugin
%manifest sms-plugin.manifest
%license LICENSE.APLv2
%{_libdir}/libmsg_sms_plugin.so

%files -n mms-plugin
%manifest mms-plugin.manifest
%license LICENSE.APLv2
%{_libdir}/libmsg_mms_plugin.so

%if "%{?profile}" != "wearable"
%files -n msg-manager
%manifest msg-manager.manifest
%license LICENSE.APLv2
%{APP_BINDIR}/msg-manager
%{APP_MANIFESTDIR}/*.xml
%endif

%changelog
