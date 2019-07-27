Name:    produceit
epoch:   1
Summary: A chroot package production and testing environment
Version: 0.3.0
Release: 1

License: GPL-3.0+
URL:     https://gitlab.com/tychosoft/produceit
Source0: https://cloud.tychosoft.com/package/source/%{name}-%{version}.tar.gz
Group:   system/tools
Vendor:  Tycho Softworks

BuildRequires: cmake >= 3.1.0
BuildRequires: gcc-c++
Requires(post): permissions
Requires:      rpm-build
Recommends:	   produceit-debutils
Suggests:	   qemu-user-static
PreReq:        permissions

%package debutils
Requires: ruby, gnupg2
Summary: Utilities used with produceit or inside a debian chroot

%description
This package includes tools are used for managing chroot and package build
environments.  Both shellit and buildit offers a fast way to execute commands
and build packages in a managed chroot for multiple distributions.  There
are some additional cross-distro packaging utilities to help constructing
full archive build environments.

%description debutils
This provides some stand-alone utilities used to produce debian archives.
These can be used either with produceit, or installed into a produceit managed
debian based chroot.

%prep
%setup -q

%build
%cmake \
        -DCMAKE_INSTALL_SYSCONFDIR=%{_sysconfdir} \
        -DCMAKE_INSTALL_LOCALSTATEDIR=%{_localstatedir}

%{__make} %{?_smp_mflags}

%install
%cmake_install

%post
%set_permissions /usr/bin/shellit /usr/bin/buildit

%verifyscript
%verify_permissions -e /usr/bin/buildit
%verify_permissions -e /usr/bin/shellit

%files
%defattr(-,root,root)
%license LICENSE
%doc README.md CHANGES
%config(noreplace) %{_sysconfdir}/produceit.conf
%verify(not user group mode) %attr(4755,root,root) %{_bindir}/buildit
%verify(not user group mode) %attr(4755,root,root) %{_bindir}/shellit
%{_mandir}/man1/shellit.1*
%{_mandir}/man1/buildit.1*

%files debutils
%defattr(-,root,root)
%{_bindir}/deb-*
%{_mandir}/man1/deb-*