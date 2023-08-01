# On modern Fedora/RHEL, use Python 3 by default (and provide an RMF-python2
# subpackage; on RHEL 9 or later, use Python 3 only).
# On older systems, the RMF package uses Python 2 only.
%if 0%{?fedora} > 12 || 0%{?rhel} >= 8
%define with_python3 1
%define cmake_use_python2 off
%define default_python python3
%else
%define with_python3 0
%define cmake_use_python2 on
%define default_python python2
%endif

%if 0%{?rhel} >= 9
%define with_python2 0
%else
%define with_python2 1
%endif

# Old RHEL only has a python-devel package, but RHEL8 only has python2-devel
%if 0%{?fedora} || 0%{?rhel} >= 8
%define PYTHON2 python2
%else
%define PYTHON2 python
%endif

Name:          RMF
Version:       1.5.1
Release:       1%{?dist}
License:       Apache 2.0
Summary:       Library to support reading and writing of RMF files
Group:         Applications/Engineering
Packager:      Ben Webb <ben@salilab.org>
URL:           https://integrativemodeling.org/rmf/
Source0:       rmf-%{version}.tar.gz
%if 0%{?with_python2}
BuildRequires: %{PYTHON2}-devel >= 2.7
%endif
%if 0%{?with_python3}
BuildRequires: python3-devel, symlinks
%endif
BuildRequires: gcc-c++, hdf5-devel >= 1.8
%if 0%{?rhel} == 7
# The default SWIG package in RHEL7 is SWIG 2, but SWIG 3 is provided
# (in the CentOS Extras repository)
BuildRequires: swig3 >= 3.0
%else
BuildRequires: swig >= 3.0
%endif

BuildRequires: cmake >= 2.8
BuildRequires: boost-devel >= 1.53

# Add numpy support if available (the Python 2 variant is no longer shipped
# with modern Fedora).
%if 0%{?with_python3}
BuildRequires: python3-numpy
Requires: python3-numpy
%if 0%{?with_python2} && (0%{?fedora} < 34 || 0%{?rhel})
BuildRequires: python2-numpy
%endif
%else
BuildRequires: numpy
Requires: numpy
%endif

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
%if 0%{?with_python3}
Requires:      python3-devel
%else
Requires:      %{PYTHON2}-devel
%endif

%description devel
This package contains the include files for building applications that link
against RMF.

%if 0%{?with_python2} && 0%{?with_python3}
%package python2
Group:         Applications/Engineering
Summary:       Python wrappers for Python 2
Requires:      %{name} = %{version}-%{release}
Requires:      python2
%if 0%{?fedora} < 34 || 0%{?rhel}
Requires: python2-numpy
%endif

%description python2
This package contains wrappers for Python 2 (the base package already
includes Python 3 wrappers).
%endif

%prep
%setup -n rmf-%{version}

%build
mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=%{_prefix} \
         -DUSE_PYTHON2=%{cmake_use_python2}
make %{?_smp_mflags}

%install
cd build
make %{?_smp_mflags} DESTDIR=${RPM_BUILD_ROOT} install

%if 0%{?with_python2} && 0%{?with_python3}
# Build Python 2 wrappers
py2_ver=`%{PYTHON2} -c "import sys; print('%d.%d' % sys.version_info[:2])"`
py3_ver=`python3 -c "import sys; print('%d.%d' % sys.version_info[:2])"`
py2_lib=`echo %{_libdir}/libpython2.*.so`
py2_inc=`echo /usr/include/python2.*`
cmake .. \
      -DCMAKE_INSTALL_PREFIX=%{_prefix} \
      -DCMAKE_INSTALL_PYTHONDIR=%{_libdir}/python${py2_ver}/site-packages \
      -DSWIG_PYTHON_LIBRARIES=${py2_lib} \
      -DPYTHON_INCLUDE_DIRS=${py2_inc} \
      -DPYTHON_INCLUDE_PATH=${py2_inc} \
      -DPYTHON_LIBRARIES=${py2_lib} -DUSE_PYTHON2=on
make %{?_smp_mflags} DESTDIR=${RPM_BUILD_ROOT} install

# Replace .py files with symlinks to Python 3 files (since they are the same)
# but not the SWIG-generated __init__.py files (since these contain config
# information which might be different; e.g. on Fedora the Python 3 wrappers
# include numpy support but the Python 2 wrappers do not)
(cd ${RPM_BUILD_ROOT}%{_libdir}/python${py2_ver} \
 && find site-packages -name '*.py' -a ! -name __init__.py \
      -exec ln -sf ${RPM_BUILD_ROOT}%{_libdir}/python${py3_ver}/\{\} \{\} \; \
 && symlinks -rc .)
%endif

%check
# Basic check of installed Python wrappers and command line tools
export LD_LIBRARY_PATH=${RPM_BUILD_ROOT}%{_libdir}
${RPM_BUILD_ROOT}%{_prefix}/bin/rmf3_dump --version

%if 0%{?with_python3}
  py3_ver=`python3 -c "import sys; print('%d.%d' % sys.version_info[:2])"`
  export PYTHONPATH=${RPM_BUILD_ROOT}%{_libdir}/python${py3_ver}/site-packages
  python3 -c "import RMF; assert(RMF.__version__ == '%{version}')"
  python3 -c "import RMF; assert(hasattr(RMF, 'get_all_global_coordinates'))"
%endif

%if 0%{?with_python2}
  py2_ver=`%{PYTHON2} -c "import sys; print('%d.%d' % sys.version_info[:2])"`
  export PYTHONPATH=${RPM_BUILD_ROOT}%{_libdir}/python${py2_ver}/site-packages
  %{PYTHON2} -c "import RMF; assert(RMF.__version__ == '%{version}')"
%endif

%files
%defattr(-,root,root)
%{_prefix}/bin/*
%{_libdir}/libRMF*.so.*
%{_libdir}/%{default_python}*/site-packages/RMF*
%{_libdir}/%{default_python}*/site-packages/_RMF*so
%if 0%{?with_python3}
%{_libdir}/python3*/site-packages/__pycache__
%endif

%if 0%{?with_python2} && 0%{?with_python3}
%files python2
%defattr(-,root,root)
%{_libdir}/python2*/site-packages/RMF*
%{_libdir}/python2*/site-packages/_RMF*so
%endif

%files devel
%defattr(-,root,root)
%{_prefix}/share/RMF/swig
%{_prefix}/include/RMF
%{_prefix}/include/RMF.h
%{_libdir}/libRMF*.so

%changelog
* Mon Jul 31 2023 Ben Webb <ben@salilab.org>   1.5.1-1
- Initial build.
