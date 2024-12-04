Name:          RMF
Version:       1.7.0
Release:       1%{?dist}
License:       Apache 2.0
Summary:       Library to support reading and writing of RMF files
Group:         Applications/Engineering
Packager:      Ben Webb <ben@salilab.org>
URL:           https://integrativemodeling.org/rmf/
Source0:       rmf-%{version}.tar.gz
BuildRequires: python3-devel, symlinks
BuildRequires: gcc-c++, hdf5-devel >= 1.8
BuildRequires: swig >= 3.0
BuildRequires: cmake >= 3.14
BuildRequires: boost-devel >= 1.53

BuildRequires: python3-numpy
Requires: python3-numpy

# Use user-visible (not "platform") Python on RHEL8
%if 0%{?rhel} >= 8
%define __python3 /usr/bin/python3
%endif

# Don't build debug source packages; they cause the build to fail with
# error: Empty %files file [...]/debugsourcefiles.list
%if 0%{?fedora} > 26 || 0%{?rhel} >= 8
%undefine _debugsource_packages
%endif

%description
The library provides support for the RMF file format for storing hierarchical
molecular data (such as atomic or coarse grained representations of proteins),
along with markup, including geometry and score data.

%package devel
Group:         Applications/Engineering
Summary:       Development package for RMF developers.
Requires:      %{name} = %{version}-%{release}
Requires:      boost-devel, hdf5-devel
Requires:      python3-devel

%description devel
This package contains the include files for building applications that link
against RMF.

%prep
%setup -n rmf-%{version}

%build
mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?_smp_mflags}

%install
cd build
make %{?_smp_mflags} DESTDIR=${RPM_BUILD_ROOT} install

%check
# Basic check of installed Python wrappers and command line tools
export LD_LIBRARY_PATH=${RPM_BUILD_ROOT}%{_libdir}
${RPM_BUILD_ROOT}%{_prefix}/bin/rmf3_dump --version

py3_ver=`python3 -c "import sys; print('%d.%d' % sys.version_info[:2])"`
export PYTHONPATH=${RPM_BUILD_ROOT}%{_libdir}/python${py3_ver}/site-packages
python3 -c "import RMF; assert(RMF.__version__ == '%{version}')"
python3 -c "import RMF; assert(hasattr(RMF, 'get_all_global_coordinates'))"

%files
%defattr(-,root,root)
%{_prefix}/bin/*
%{_libdir}/libRMF*.so.*
%{_libdir}/python3*/site-packages/RMF*
%{_libdir}/python3*/site-packages/_RMF*so
%{_libdir}/python3*/site-packages/__pycache__

%files devel
%defattr(-,root,root)
%{_prefix}/share/RMF/swig
%{_prefix}/include/RMF
%{_prefix}/include/RMF.h
%{_libdir}/libRMF*.so

%changelog
* Wed Dec 04 2024 Ben Webb <ben@salilab.org>   1.7.0-1
- Update for 1.7.0 release.

* Mon May 13 2024 Ben Webb <ben@salilab.org>   1.6.1-1
- Update for 1.6.1 release.

* Thu Dec 14 2023 Ben Webb <ben@salilab.org>   1.6.0-1
- Update for 1.6.0 release.

* Mon Jul 31 2023 Ben Webb <ben@salilab.org>   1.5.1-1
- Initial build.
