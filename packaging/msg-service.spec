Name:           msg-service
Version:        0.9.3
Release:        1
License:        Flora-1.1
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
License:        Flora License v1.1
Summary:        Messaging Framework Library (development)
Requires:       %{name} = %{version}-%{release}
Group:          Development/Libraries

%description devel
Description: Messaging Framework Library (development)


%package tools
License:        Flora License v1.1
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
License:        Flora License v1.1
Summary:        SMS plugin library
Requires:       %{name} = %{version}-%{release}
Group:          System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description -n sms-plugin
Description: SMS plugin library

%package -n mms-plugin
License:        Flora License v1.1
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
%license LICENSE.Flora
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
%license LICENSE.Flora
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
%license LICENSE.Flora
%defattr(-,root,root,-)
%{_libdir}/libmsg_sms_plugin.so

%files -n mms-plugin
%manifest mms-plugin.manifest
%license LICENSE.Flora
%defattr(-,root,root,-)
%{_libdir}/libmsg_mms_plugin.so

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
