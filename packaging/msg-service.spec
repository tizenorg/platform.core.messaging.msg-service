Name:           msg-service
Version:        0.9.4
Release:        1
License:        Apache-2.0
Summary:        Messaging Framework Library
Group:          System/Libraries
Source0:        %{name}-%{version}.tar.gz
Source101:      msg-service.service
Source1001:	%{name}.manifest
Source1002:	%{name}-devel.manifest
Source1003:	%{name}-tools.manifest
Source1004:	sms-plugin.manifest
Source1005:	mms-plugin.manifest

Requires(post): /usr/bin/sqlite3
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires(post): systemd
Requires(postun): systemd
BuildRequires: cmake
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(appsvc)
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
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(libtzplatform-config)

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
cp %{SOURCE1001} %{SOURCE1002} %{SOURCE1003} %{SOURCE1004} %{SOURCE1005} .


%build
%cmake . \
	-DTZ_SYS_SHARE=%TZ_SYS_SHARE \
	-DUNITDIR_USER=%{_unitdir_user} \
	-DTZ_SYS_SMACK=%TZ_SYS_SMACK

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/etc/config

mkdir -p %{buildroot}/var/log/msgfw

%make_install

mkdir -p %{buildroot}%{_unitdir_user}/tizen-middleware.target.wants
#install -m 0644 %SOURCE101 %{buildroot}%{_unitdir_user}/
ln -s ../msg-service.service %{buildroot}%{_unitdir_user}/tizen-middleware.target.wants/msg-service.service
ln -s ../msg-service-log.service %{buildroot}%{_unitdir_user}/tizen-middleware.target.wants/msg-service-log.service

mkdir -p  %{buildroot}%{_sysconfdir}/rc.d/rc3.d
ln -s %{_sysconfdir}/rc.d/init.d/msg-server  %{buildroot}%{_sysconfdir}/rc.d/rc3.d/S70msg-server
mkdir -p  %{buildroot}%{_sysconfdir}/rc.d/rc5.d
ln -s %{_sysconfdir}/rc.d/init.d/msg-server  %{buildroot}%{_sysconfdir}/rc.d/rc5.d/S70msg-server

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

/bin/systemctl daemon-reload
if [ "$1" = "1" ]; then
    systemctl stop msg-service.service
fi

%postun -p /sbin/ldconfig

%postun tools -p /sbin/ldconfig
%postun -n sms-plugin -p /sbin/ldconfig
%postun -n mms-plugin -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%license LICENSE.APLv2
%defattr(-,root,root,-)
%{_libdir}/libmsg_plugin_manager.so
%{_libdir}/libmsg_mapi.so.*
%{_libdir}/libmsg_framework_handler.so
%{_libdir}/libmsg_transaction_manager.so
%{_libdir}/libmsg_utils.so
%{_libdir}/libmsg_transaction_proxy.so
%{_libdir}/libmsg_vobject.so
%TZ_SYS_SHARE/%{name}/msg_service-init-DB.sh

%files devel
%manifest %{name}-devel.manifest
%defattr(-,root,root,-)
%{_libdir}/libmsg_mapi.so
%{_libdir}/pkgconfig/msg-service.pc
%{_includedir}/msg-service/*

%files tools
%manifest %{name}-tools.manifest
%license LICENSE.APLv2
%defattr(-,root,root,-)
%{_bindir}/msg-helper
%{_bindir}/msg-server
%{_datadir}/media/Sherbet.wav
%attr(0644,root,root)%{_datadir}/msg-service/plugin.cfg
%{_sysconfdir}/rc.d/init.d/msg-server
%{_sysconfdir}/rc.d/rc3.d/S70msg-server
%{_sysconfdir}/rc.d/rc5.d/S70msg-server
%{_unitdir_user}/msg-service.service
%{_unitdir_user}/msg-service-log.service
%{_unitdir_user}/tizen-middleware.target.wants/msg-service.service
%{_unitdir_user}/tizen-middleware.target.wants/msg-service-log.service
%{_sysconfdir}/config/sysinfo-message.xml
%attr(0755,root,%TZ_SYS_USER_GROUP) /var/log/msgfw

%files -n sms-plugin
%manifest sms-plugin.manifest
%license LICENSE.APLv2
%defattr(-,root,root,-)
%{_libdir}/libmsg_sms_plugin.so

%files -n mms-plugin
%manifest mms-plugin.manifest
%license LICENSE.APLv2
%defattr(-,root,root,-)
%{_libdir}/libmsg_mms_plugin.so
