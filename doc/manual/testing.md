Debugging and testing your code {#testing}
===============================

Ensuring that your code is correct can be very difficult, so %IMP
provides a number of tools to help you out.

The first set are assert-style macros to use in the C++ code:

- IMP_USAGE_CHECK() which should be used to check that arguments to
  functions and methods satisfy the preconditions.

- IMP_INTERNAL_CHECK() which should be used to verify internal state
  and return values to make sure they satisfy pre and post-conditions.

See the [checks](../ref/exception_8h.html) page for more details. As a
general guideline, any improper usage should produce at least a warning
and all return values should be checked by such code.

The second is logging macros such as:

- IMP_LOG() which allows controlled display of messages about what the
  code is doing. See [logging](../ref/log_8h.html) for more information.

Finally, each module has a set of unit tests. The
tests are located in the `modules/modulename/test` directory.
These tests should try, as much as possible, to provide independent
verification of the correctness of the code. Any
file in that directory or a subdirectory whose name matches `test_*.{py,cpp}`,
`medium_test_*.{py,cpp}` or `expensive_test_*.{py,cpp}` is considered a test.
Normal tests should run in at most a few seconds on a typical machine, medium
tests in 10 seconds or so and expensive tests in a couple of minutes.

Python tests can, in principle, be any Python script that exits without error
if the test passes. However, in most cases it makes sense to use the Python
[unittest framework](https://docs.python.org/2/library/unittest.html). We
provide a custom test case subclass that provides some useful IMP-specific
functions (IMP::test::TestCase). The convention in %IMP is for tests to
make a subclass of this class called `Tests` (this makes it easier to run
individual tests from the command line). Some tests will require input files
or temporary files. Input files
should be placed in a directory called `input` in the `test`
directory. The test script should then call
\command{self.get_input_file_name(file_name)} to get the true path to
the file. Likewise, appropriate names for temporary files should be
found by calling
\command{self.get_tmp_file_name(file_name)}.
