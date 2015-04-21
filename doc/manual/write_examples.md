Writing examples {#write_examples}
================

Writing examples is a very important part of being an %IMP developer and
one of the best ways to help people use your code. To write a (Python)
example, create a file `myexample.py` in the example directory of an
appropriate module. The first few lines of the script should
should provide a brief overview of what the code in the module is
trying to accomplish as well as key pieces of %IMP functionality that
it uses. (This is a comment parsed by Doxygen, so should start with
the `\\example` command.)

\note Examples are run by the build system in the same way as unit tests, and so
should run to completion in *under 2 minutes*. If your example takes longer
than this, it is not a very good example! (For one, users will tire of waiting
for it to finish.) The build system passes each example the `--run_quick_test`
argument, so this can be used to run a faster subset (e.g. fewer iterations
of an optimization). If you are trying to demonstrate the application of your
method to a real biological system, you should write a
[biological system](http://integrativemodeling.org/systems/) instead of an
example.

The example should have enough comments that the reasoning behind each line of code is clear to someone who roughly understands how %IMP in general works.

Examples must use methods like IMP::get_example_path() to access
data in the example directory. This allows them to be run from
anywhere (when they are run by `ctest` the working directory is *not* the
directory the example is in).
