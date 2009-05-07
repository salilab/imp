Name:          imp
Version:       SVN
Release:       1
License:       To be determined
Summary:       The Integrative Modeling Platform
Group:         Applications/Engineering
Packager:      Ben Webb <ben@salilab.org>
URL:           http://www.integrativemodeling.org/
Source0:       %{name}-%{version}.tar.gz
BuildRoot:     %{_tmppath}/%{name}-%{version}-root
BuildRequires: scons >= 0.98, boost-devel, swig, python-devel, /usr/bin/dot

%description
IMP's broad goal is to contribute to a comprehensive structural
characterization of biomolecules ranging in size and complexity from small
peptides to large macromolecular assemblies. Detailed structural
characterization of assemblies is generally impossible by any single existing
experimental or computational method. This barrier can be overcome by hybrid
approaches that integrate data from diverse biochemical and biophysical
experiments (eg, x-ray crystallography, NMR spectroscopy, electron microscopy,
immuno-electron microscopy, footprinting, chemical cross-linking, FRET
spectroscopy, small angle X-ray scattering, immunoprecipitation, genetic
interactions, etc...).

We formulate the hybrid approach to structure determination as an optimization
problem, the solution of which requires three main components:
    * the representation of the assembly,
    * the scoring function and
    * the optimization method.

The ensemble of solutions to the optimization problem embodies the most
accurate structural characterization given the available information.

We created IMP, the Integrative Modeling Platform, to make it easier to
implement such an integrative approach to structural and dynamics problems.
IMP is designed to allow mixing and matching of existing modeling components
as well as easy addition of new functionality.

%package devel
Group:         Applications/Engineering
Summary:       Development package for IMP developers.
Requires:      %{name} = %{version}-%{release}

%description devel
This package contains the include files for building applications that link
against IMP.

%package python
Group:         Applications/Engineering
Summary:       Python bindings for IMP.
Requires:      %{name} = %{version}-%{release}

%description python
This package contains Python bindings for IMP, to allow IMP functionality to
be used from Python.

%prep
%setup

%build
scons && scons doc

%install
scons destdir=${RPM_BUILD_ROOT} docdir=/usr/share/doc/%{name}-%{version} \
      install docinstall

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc /usr/share/doc/%{name}-%{version}
%{_libdir}/libimp*.so

%files devel
%defattr(-,root,root)
/usr/include/IMP
/usr/include/IMP.h

%files python
%defattr(-,root,root)
%{_libdir}/python*/site-packages/IMP
%{_libdir}/python*/site-packages/_IMP*so

%changelog
* Thu May 07 2009 Ben Webb <ben@salilab.org>
- Initial build.
