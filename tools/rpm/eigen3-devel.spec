Name:           eigen3-devel
Version:        3.0.0
Release:        1%{?dist}
Summary:        A lightweight C++ template library for vector and matrix math
Group:          Development/Libraries
License:        LGPLv3+ or GPLv2+
URL:            http://eigen.tuxfamily.org/index.php?title=Main_Page
# Source file is at: http://bitbucket.org/eigen/eigen/get/3.0.0.tar.bz2
# Renamed source file so it's not just a version number
Source0:        eigen-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      noarch
BuildRequires:  sed

# Fixes build by adding the cstdef to a source file.
# Not yet submitted upstream
Patch0:         eigen-3.0.0.ptrdiff.patch

%description
Eigen is a C++ template library for linear algebra: matrices, vectors,
numerical solvers, and related algorithms.

%prep
%setup -q -n eigen-eigen-65ee2328342f
%patch0 -p0

%install
incdir=${RPM_BUILD_ROOT}%{_includedir}/eigen3
pcdir=${RPM_BUILD_ROOT}%{_datadir}/pkgconfig
mkdir -p ${incdir} ${pcdir}
cp -pR Eigen ${incdir}
sed -e 's#${EIGEN_VERSION_NUMBER}#%{version}#' \
    -e 's#${INCLUDE_INSTALL_DIR}#%{_includedir}/eigen3#' \
    < eigen3.pc.in > ${pcdir}/eigen3.pc

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc COPYING.GPL COPYING.LGPL
%{_includedir}/eigen3
%{_datadir}/pkgconfig/*

%changelog
* Tue Oct 18 2011 Ben Webb <ben@salilab.org> - 3.0.0-1
- Initial build for RHEL5 based on the Fedora package by Rich Mattes.
