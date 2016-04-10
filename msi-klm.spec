Name: msi-klm
Version: 1.0
Release: 1%{?dist}
Summary: msi keyboard led manager
Group: System Environment/Base
License: GPL
URL: https://github.com/AlexanderKlishin/msi-klm.git
Source0: none
BuildRequires: make
BuildRequires: hidapi-devel

%description

%prep

%build
pwd
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp msi-klm $RPM_BUILD_ROOT/usr/bin

%clean
rm -rf $RPM_BUILD_ROOT


%files
%attr(4755,-,-) /usr/bin/msi-klm

%doc

%changelog

