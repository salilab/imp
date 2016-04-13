Automated builds and checks {#autobuild}
===========================

The %IMP code is automatically tested for consistency and stability in a number
of ways.

## Nightly builds

%IMP is built nightly from source code on a variety of different machines
in the Sali lab, and in several different configurations, and all the test
cases, examples, and benchmarks run. This aims to detect problems with new
code relatively quickly. See the
[nightly build status page](https://integrativemodeling.org/nightly/results/)
for latest results. This is done for the head of the `develop` branch of the
code in the [GitHub](https://github.com/salilab/imp) repository. When a new
stable release is in preparation, this is also done for the corresponding
`release/x.y.z` branch and also for the `master` branch once the new release
is merged there. Results for these branches can also be viewed at the nightly
build state page by selecting the branch from the dropdown at the top left
of the page.

## Travis CI

A subset of the %IMP code is built using
[Travis CI](https://travis-ci.org/salilab/imp) every time code is pushed to
the `develop` branch on [GitHub](https://github.com/salilab/imp). (Only a subset
is built because building all of %IMP would exceed Travis CI's time limits.)
This is helpful to quickly detect compilation errors.

## Coverity

The entire %IMP code is periodically subjected to static code analysis
using [Coverity](https://scan.coverity.com/projects/salilab-imp). This is
useful to detect some types of coding errors, such as buffer overflows.

The Coverity scan is not currently completely automated, since the compilation
is quite expensive and Coverity imposes strict limits on the frequency of
builds. It is currently run manually on an older Mac running OS X 10.6 (the
newer Macs in the Sali lab use the `clang` compiler, which Coverity doesn't
support, and the build takes a really long time on a Linux box).

Note that Coverity currently reports a rather high defect density for %IMP.
Many of these are false positives - for example, Coverity complains about
all of the \ref geometricprimitives not being initialized, but this is
intentional.
