# norootforbuild

Name:           asterixInspector
Version:        0.12.2
Release:        1
Summary:        A viewer for Eurocontrol Asterix files

Group:          Productivity/File utilities
License:        BSD-3-Clause
URL:            http://asterix.sourceforge.net/
Source0:        %{name}-0.12.1.tar.bz2   
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%if 0%{?suse_version}
BuildRequires:  libqt5-qtbase-devel libQt5WebKit5-devel libQt5WebKitWidgets-devel
BuildRequires:  update-desktop-files
%endif

%if 0%{?fedora_version} || 0%{?rhel_version} || 0%{?centos_version}
BuildRequires: gcc-c++ qt5-qtbase-devel qt5-qtwebkit-devel
%endif

%description
AsterixInspector - displays contents of files which are in Eurocontrol Asterix format.
Asterix is a binary data exchange format in aviation, standardized by Eurocontrol.

%prep
%setup -q -n %{name}-%{version}


%build
qmake-qt5 QMAKE_CXXFLAGS+="$RPM_OPT_FLAGS"
%__make %{?_smp_mflags}


%install
%__install -dm 755 %{buildroot}%{_bindir}
%__install -m 755 %{name} %{buildroot}%{_bindir}
%__install -dm 755 %{buildroot}%{_datadir}/pixmaps/
%__install -m 644 images/radar.svg %{buildroot}%{_datadir}/pixmaps/%{name}.svg
#%__install -dm 755 %{buildroot}%{_datadir}/icons
#%__install -m 644 images/radar.svg %{buildroot}%{_datadir}/icons/%{name}.svg
%__install -dm 755 %{buildroot}%{_datadir}/applications/
%__install -m 644 %{name}.desktop %{buildroot}%{_datadir}/applications/
%__install -dm 755 %{buildroot}%{_datadir}/doc/packages/asterixInspector/examples/
%__install -m 644 examples/* %{buildroot}%{_datadir}/doc/packages/asterixInspector/examples/
%__install -dm 755 %{buildroot}%{_datadir}/asterixInspector/asterixSpecification/
%__install -m 644 asterixSpecification/* %{buildroot}%{_datadir}/asterixInspector/asterixSpecification/

%if 0%{?suse_version}
%suse_update_desktop_file -r %{name} Utility Editor Qt Education Engineering
%endif

%clean
%__rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_datadir}/pixmaps/%{name}.svg
# %{_datadir}/icons/%{name}.svg
%{_datadir}/applications/%{name}.desktop
%{_datadir}/doc/packages/asterixInspector
%{_datadir}/doc/packages/asterixInspector/examples
%{_datadir}/asterixInspector
%{_datadir}/asterixInspector/asterixSpecification

%doc


%changelog
