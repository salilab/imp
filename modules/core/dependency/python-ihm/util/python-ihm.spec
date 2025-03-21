Name:          python3-ihm
License:       MIT
Group:         Applications/Engineering
Version:       2.3
Release:       1%{?dist}
Summary:       Package for handling IHM mmCIF and BinaryCIF files
Packager:      Ben Webb <ben@salilab.org>
URL:           https://pypi.python.org/pypi/ihm
Source:        ihm-%{version}.tar.gz
BuildRequires: python3-devel, python3-setuptools, gcc
%if 0%{?rhel} != 7
BuildRequires: python3-msgpack
Requires: python3-msgpack
%else
BuildRequires: sed
%endif

%description
This is a Python package to assist in handling mmCIF and BinaryCIF files
compliant with the integrative/hybrid modeling (IHM) extension. It works
with Python 3.6 or later.

%prep
%setup -n ihm-%{version}
%if 0%{?rhel} == 7
sed -i -e "s/install_requires=\['msgpack'\]/#/" setup.py
%endif

%build
%{__python3} setup.py install --root=${RPM_BUILD_ROOT} --record=INSTALLED_FILES

%check
%{__python3} setup.py test

%files -f INSTALLED_FILES
%defattr(-,root,root)

%changelog
* Thu Mar 13 2025 Ben Webb <ben@salilab.org>   2.3-1
- Update to latest upstream.

* Thu Feb 13 2025 Ben Webb <ben@salilab.org>   2.2-1
- Update to latest upstream.

* Wed Feb 12 2025 Ben Webb <ben@salilab.org>   2.1-1
- Update to latest upstream.

* Tue Feb 11 2025 Ben Webb <ben@salilab.org>   2.0-1
- Update to latest upstream.

* Tue Nov 26 2024 Ben Webb <ben@salilab.org>   1.8-1
- Update to latest upstream.

* Tue Oct 22 2024 Ben Webb <ben@salilab.org>   1.7-1
- Update to latest upstream.

* Fri Sep 27 2024 Ben Webb <ben@salilab.org>   1.6-1
- Update to latest upstream.

* Fri Sep 06 2024 Ben Webb <ben@salilab.org>   1.5-1
- Update to latest upstream.

* Fri Aug 30 2024 Ben Webb <ben@salilab.org>   1.4-1
- Update to latest upstream.

* Tue Jul 16 2024 Ben Webb <ben@salilab.org>   1.3-1
- Update to latest upstream.

* Wed Jun 12 2024 Ben Webb <ben@salilab.org>   1.2-1
- Update to latest upstream.

* Thu May 09 2024 Ben Webb <ben@salilab.org>   1.1-1
- Update to latest upstream.

* Tue Feb 13 2024 Ben Webb <ben@salilab.org>   1.0-1
- Update to latest upstream.

* Fri Dec 08 2023 Ben Webb <ben@salilab.org>   0.43-1
- Update to latest upstream.

* Thu Nov 30 2023 Ben Webb <ben@salilab.org>   0.42-1
- Update to latest upstream.

* Mon Oct 02 2023 Ben Webb <ben@salilab.org>   0.41-1
- Update to latest upstream.

* Mon Sep 25 2023 Ben Webb <ben@salilab.org>   0.40-1
- Update to latest upstream.

* Fri Aug 04 2023 Ben Webb <ben@salilab.org>   0.39-1
- Update to latest upstream.

* Mon Jul 31 2023 Ben Webb <ben@salilab.org>   0.38-1
- Update to latest upstream.

* Thu Apr 14 2022 Ben Webb <ben@salilab.org>   0.31-1
- Update to latest upstream.

* Mon Mar 21 2022 Ben Webb <ben@salilab.org>   0.28-1
- Update to latest upstream.

* Thu Jan 27 2022 Ben Webb <ben@salilab.org>   0.27-1
- Update to latest upstream.

* Tue Jan 25 2022 Ben Webb <ben@salilab.org>   0.26-1
- Update to latest upstream.

* Fri Dec 03 2021 Ben Webb <ben@salilab.org>   0.25-1
- Update to latest upstream.

* Fri Jul 09 2021 Ben Webb <ben@salilab.org>   0.20-1
- Initial package.
