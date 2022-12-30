Name:           smokeview
Version:        0.2.4
Release:        1%{?dist}
Summary:        Smokeview

License:        AllRightsReserved
Source0:        smokeview-%{version}.tar.gz
Url:            https://smokecloud.io

BuildRequires:  systemd, systemd-rpm-macros
BuildRequires:  cmake
Requires:       bash
Requires:       lua-filesystem
Requires:       lua

%description
A post-processor for FDS (Fire Dynamics Simulator).

%prep
%setup -n smokeview-%{version}

%global debug_package %{nil}
%build
%cmake -DLUA=ON -DGLUI=off
%cmake_build

%install
%cmake_install

%files
%{_bindir}/smokeview
%{_bindir}/*.lua


%changelog
* Sat Dec 18 2021 admin
-
