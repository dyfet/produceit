Name:    produceit
epoch:   1
Summary: chroot package production and testing environment
Version: 0.1.0
Release: 1

License: GPLv3+
URL:     https://gitlab.com/tychosoft/produceit
Source0: https://pub.cherokeesofidaho.org/tarballs/%{name}-%{version}.tar.gz
Group:   system/tools

BuildRequires: cmake gcc-c++
Requires:      rpm-build ruby gnupg2
Suggests:	   qemu-user-static

%description
This package includes tools are used for managing chroot and package build
environments.  Both shellit and buildit offers a fast way to execute commands and
build packages in managed chroot's from multiple distributions.  There are some
additional cross-distro packaging utilities to help constructing full archive
build environments.

%prep
%setup -q

%build
%cmake \
        -DCMAKE_INSTALL_SYSCONFDIR=%{_sysconfdir} \
        -DCMAKE_INSTALL_LOCALSTATEDIR=%{_localstatedir}

%{__make} %{?_smp_mflags}

%install
%make_install

%files
%defattr(-,root,root,-)
%license LICENSE
%doc README.md CHANGELOG
%attr(4755,root,root) %{_bindir}/buildit
%attr(4755,root,root) %{_bindir}/shellit
%attr(0755,root,root) %{_bindir}/archiveit

