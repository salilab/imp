# Code Coverage#

# Code coverage analysis # {#coverage}

To assist in testing your code, we report the coverage of all IMP modules
and applications as part of the
[nightly builds](http://www.salilab.org/imp/nightly/results/).
Coverage is basically a report of which lines of code were executed by your
tests; it is then straightforward to see which parts of the code have not
been exercised by any test, so that you can write new tests to test those
parts. (Of course, lines of code that are never executed
have no guarantee of working correctly.)

Both the C++ and Python code coverage is reported. For C++ code, only the
lines of code that were exercised are reported; for Python code, which
conditional branches were taken are also shown (for example, whether both
branches from an 'if' statement are followed).

Ideally, coverage reflects the lines of code in a module or application
that were exercised only by running its own tests, rather than the tests of the
entire IMP package, and generally speaking you should try to test a module
using its own tests.

If you have code that for some reason you wish to exclude from coverage,
you can add specially formatted comments to the code. For Python code,
[add a "pragma: no cover"](http://nedbatchelder.com/code/coverage/excluding.html)
comment to the line to exclude. For C++ code, an individual line can be excluded
by adding `LCOV_EXCL_LINE` somewhere on that line, or a block can be excluded
by surrounding it with lines containing `LCOV_EXCL_START` and `LCOV_EXCL_STOP`.
