Name:           cohdi-cicd-helper
Version:        0.2
Release:        1%{?dist}
Summary:        CoHDI CI/CD Helper for Worker Nodes

License:        GPL-2.0+ or later
URL:            https://github.com/cohdi/cohdi-cicd-helper
Source0:        cohdi-cicd-helper-%{version}.tar.gz

BuildArch:      noarch
BuildRequires:  dkms
BuildRequires:  kernel-devel
BuildRequires:  kernel-headers

%description
This package contains a kernel module and a shell script to emulate CDI
(Composable Disaggregated Infrastructure) for the NVIDIA DRA Driver for
GPUs.

%prep
%setup -n %{name}-%{version}

%build

%install
mkdir -p %{buildroot}/usr/src/restrict_kmod-%{version}
cp -r %{_topdir}/BUILD/%{name}-%{version}/module/* %{buildroot}/usr/src/restrict_kmod-%{version}/

mkdir -p %{buildroot}%{_modprobedir}
cp -p %{_topdir}/BUILD/%{name}-%{version}/blacklist-nouveau.conf %{buildroot}%{_modprobedir}/

mkdir -p %{buildroot}%{_sbindir}
cp -p %{_topdir}/BUILD/%{name}-%{version}/hidegpu %{buildroot}%{_sbindir}/
chmod +x %{buildroot}%{_sbindir}/hidegpu

mkdir -p %{buildroot}%{_unitdir}
cp -p %{_topdir}/BUILD/%{name}-%{version}/hidegpu.service %{buildroot}%{_unitdir}/

mkdir -p %{buildroot}%{_modulesloaddir}
cp -p %{_topdir}/BUILD/%{name}-%{version}/rk.conf %{buildroot}%{_modulesloaddir}/

%post
if [ $1 -eq 1 ]; then
    systemctl enable hidegpu.service
fi
dkms add /usr/src/restrict_kmod-%{version}
dkms install -m restrict_kmod/%{version}

%preun
if [ $1 -eq 0 ]; then
    systemctl disable hidegpu.service
fi
dkms uninstall -m restrict_kmod/%{version}
dkms remove  -m restrict_kmod/%{version}

%files
%doc README.md LICENSE
/usr/src/restrict_kmod-%{version}
%{_modprobedir}/blacklist-nouveau.conf
%{_sbindir}/hidegpu
%{_unitdir}/hidegpu.service
%{_modulesloaddir}/rk.conf

%changelog
* Mon Nov 17 2025 minoura makoto <minoura@valinux.co.jp> - 0.2-1
- Initial package for cohdi-cicd-helper version 0.2
