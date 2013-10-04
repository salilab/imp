# Developer Guide #

# Developing with IMP # {#devguide}
[TOC]

This page presents instructions on how to develop code using
IMP. Developers should also read [Getting started as a developer](https://github.com/salilab/imp/wiki/Getting-started-as-a-developer).

# Getting around IMP # {#devguide_getting_around}

The input files in the IMP directory are structured as follows:
- `tools` contains various command line utilities for use by developers. They
  are [documented below](#devguide_scripts).
- `doc` contains inputs for general IMP overview documentation (such as this
  page), as well as configuration scripts for `doxygen`.
- `applications` contains various applications implementing using a variety of
  IMP modules.
- each subdirectory of `module/` defines a module; they all have the same
  structure. The directory for module `name` has
   the following structure
   - `README.md` contains a module overview
   - `include` contains the C++ header files
   - `src` contains the C++ source files
   - `bin` contains C++ source files each of which is built into an executable
   - `pyext` contains files defining the Python interface to the module as well
      as Python source files (in `pyext/src`)
   - `test` contains test files, that can be run with `ctest`.
   - `doc` contains additional documentation that is provided via `.dox` files
   - `examples` contains examples in Python and C++, as well as any data needed
      for examples
   - `data` contains any data files needed by the module

When IMP is built, a number of directories are created in the build directory. They are
 - `include` which includes all the headers. The headers for module `name` are
    placed in `include/IMP/name`
 - `lib` where the C++ and Python libraries are placed. Module `name` is built
    into a C++ library `lib/libimp_name.so` (or `.dylib` on a Mac) and a Python
    library with Python files located in `lib/IMP/name` and the binary part in
    `lib/_IMP_name.so.`
 - `doc` where the html documentation is placed in `doc/html` and the examples
    in `doc/examples` with a subdirectory for each module
 - `data` where each module gets a subdirectory for its data.

When IMP is installed, the structure from the `build` directory is
moved over more or less intact except that the C++ and Python
libraries are put in the (different) appropriate locations.


# Writing new code # {#devguide_new_code}

The easiest way to start writing new functions and classes is to
create a new module using [make-module.py](\ref dev_tools_make_module).
This creates a new module in the `modules` directory or simply use the
`scratch` module.

We highly recommend using a revision control system such as
[git](http://git-scm.com/) or [svn](http://subversion.tigris.org/) to
keep track of changes to your module.

If, instead, you choose to add code to an existing module you need to
consult with the person who people who control that module. Their names
can be found on the module main page.

When designing the interface for your new code, you should

- search IMP for similar functionality and, if there is any, adapt
  the existing interface for your purposes. For example, the existing
  IMP::atom::read_pdb() and IMP::atom::write_pdb() functions provide
  templates that should be used for the design of any functions that
  create particles from a file or write particles to a file. Since
  IMP::atom::Bond, IMP::algebra::Segment3D and
  IMP::display::Geometry all use methods like
  IMP::algebra::Segment3D::get_point() to access the
  endpoints of a segment, any new object which defines similar
  point-based geometry should do likewise.

- think about how other people are likely to use the code. For
  example, not all molecular hierarchies have atoms as their leaves,
  so make sure your code searches for arbitrary
  IMP::core::XYZ particles rather than atoms if you only care
  about the geometry.

- look for easy ways of splitting the functionality into pieces. It
  generally makes sense, for %example, to split selection of the
  particles from the action taken on them, either by accepting a
  IMP::kernel::Refiner, or a IMP::kernel::SingletonContainer or just an arbitrary
  IMP::kernel::ParticleIndexes object.


You may want to read [the design example](\ref designexample) for
some suggestions on how to go about implementing your functionality
in IMP.

## Coding conventions ## {#devguide_conventions}

Make sure you read the [API Conventions](\ref introduction_conventions) page
first.

To ensure code consistency and readability, certain conventions
must be adhered to when writing code for IMP. Some of these
conventions are automatically checked for by source control before
allowing a new commit, and can also be checked yourself in new
code by running [check_standards.py](#devguide_check_standards) files_to_check`.

### Indentation ### {#devguide_indentation}

All C++ headers and code should be indented with 2-space indents. Do not use
tabs. [clang-format](\ref dev_tools_clang_format) can help you do this formatting
automatically.

All Python code should conform to the [Python style
guide](http://www.python.org/dev/peps/pep-0008/).  In essence this
translates to 4-space indents, no tabs, and similar class, method and
variable naming to the C++ code. You can ensure that your Python code
is correctly indented by using the `tools/reindent.py` script,
available as part of the IMP distribution.

### Names ### {#devguide_names}

See the [introduction](\ref introduction_names) first. In addition, developers
should be aware that
- all preprocessor symbols must begin with `IMP`.
- names of files that implement a single class should be named for that
  class; for example the `SpecialVector` class could be implemented in
  `SpecialVector.h` and `SpecialVector.cpp`
- files that provide free functions or macros should be given names
  `separated_by_underscores,` for `example `container_macros.h`
- Functions which take a parameter which has units should have the
  unit as part of the function name, for %example
  IMP::atom::SimulationParameters::set_maximum_time_step_in_femtoseconds().
  Remember the Mars orbiter. The exception to this is distance and
  force numbers which should always be in angstroms and kcal/mol
  angstrom respectively unless otherwise stated.

### Passing and storing data ### {#devguide_passing}

- When a class or function takes a set of particles which are expected to
  be those of a particular type of decorator, it should take a list of
  decorators instead. eg IMP::core::transform() takes a IMP::core::XYZ.
  This makes it clearer what attributes the particle is required to have
  as well as allows functions to be overloaded (so there can be an
  IMP::core::transform() which takes IMP::core::RigidBody particles instead).


- IMP::Restraint and IMP::ScoreState classes should generally use a
  IMP::SingletonContainer (or other type of Container) to store the set of
  IMP::Particle objects that they act on.

- Store collections of IMP::Object-derived
  objects of type `Name` using a `Names.` Declare functions that
  accept them to take a `NamesTemp` (`Names` is a `NamesTemp)`.
  `Names` are reference counted (see IMP::RefCounted for details),
  `NamesTemp` are not. Store collections of particles using a
  `Particles` object, rather than decorators.

### Display ### {#devguide_display}

All values must have a `show` method which takes an optional
`std::ostream` and prints information about the object (see
IMP::base::Array::show() for an example). Add a `write` method if you
want to provide output that can be read back in.

### Errors ### {#devguide_errors}

Classes and methods should use IMP exceptions to report errors. See
IMP::base::Exception for a list of existing exceptions. See
[checks](base_2exception_8h.html) for more information.

### Namespaces ### {#devguide_namespace}

Use the provided `IMPMODULE_BEGIN_NAMESPACE,`
`IMPMODULE_END_NAMESPACE,` `IMPMODULE_BEGIN_INTERNAL_NAMESPACE` and
`IMPMODULE_END_INTERNAL_NAMESPACE` macros to put declarations in a
namespace appropriate for module `MODULE.`

Each module has an internal namespace, eg `IMP::base::internal` and an internal
include directory `IMP/base/internal.` Any function which is
 - not intended to be part of the API,
 - not documented,
 - liable to change without notice,
 - or not tested

should be declared in an internal header and placed in the internal namespace.

The functionality in such internal headers is
 - not exported to Python
 - and not part of of documented API

As a result, such functions do not need to obey all the coding conventions
(but we recommend that they do).


## Documenting your code ## {#devguide_documenting}

IMP is documented using `doxygen`. See
[Documenting your code in doxygen](http://www.doxygen.nl/docblocks.html)
to get started. We use `//!` and `/**` ... * / blocks for documentation.
You are encouraged to use `Doxygen's`
[markdown support](http://www.stack.nl/~dimitri/doxygen/manual/markdown.html) as much as possible.

Python code should provide Python doc strings.

All headers not in internal directories are parsed through
`doxygen`. Any function that you do not want documented (for example,
because it is not well tested), hide by surrounding with

    \#ifndef IMP_DOXYGEN
    void messy_poorly_thought_out_function();
    \#endif

We provide a number of extra Doxygen commands to aid in producing nice
IMP documentation.

- To mark that some part of the API has not yet been well planned at may change
    using `\\unstable{Classname}.` The documentation will include a disclaimer
    and the class or function will be added to a list of unstable classes. It is
    better to simply hide such things from `doxygen`.

- To mark a method as not having been well tested yet, use `\\untested{Classname}.`

- To mark a method as not having been implemented, use `\\untested{Classname}.`

## Debugging and testing your code ## {#devguide_testing}

Ensuring that your code is correct can be very difficult, so IMP
provides a number of tools to help you out.

The first set are assert-style macros:

- IMP_USAGE_CHECK() which should be used to check that arguments to
  functions and methods satisfy the preconditions.

- IMP_INTERNAL_CHECK() which should be used to verify internal state
  and return values to make sure they satisfy pre and post-conditions.

See [checks](base_2exception_8h.html) page for more details. As a
general guideline, any improper usage to produce at least a warning
all return values should be checked by such code.

The second is logging macros such as:

- IMP_LOG() which allows controlled display of messages about what the
  code is doing. See [logging](base_2log_8h.html) for more information.

Finally, each module has a set of unit tests. The
tests are located in the `modules/modulename/test` directory.
These tests should try, as much as possible to provide independent
verification of the correctness of the code. Any
file in that directory or a subdirectory whose name matches `test_*.{py,cpp}`,
`medium_test_*.{py,cpp}` or `expensive_test_*.{py,cpp}` is considered a test.
Normal tests should run in at most a few seconds on a typical machine, medium
tests in 10 seconds or so and expensive tests in a couple of minutes.

Some tests will require input files or temporary files. Input files
should be placed in a directory called `input` in the `test`
directory. The test script should then call
\command{self.get_input_file_name(file_name)} to get the true path to
the file. Likewise, appropriate names for temporary files should be
found by calling
\command{self.get_tmp_file_name(file_name)}. Temporary files will be
located in `build/tmp.` The test should remove temporary files after
using them.

## Writing Examples ## {#devguide_examples}

Writing examples is very important part of being an IMP developer and
one of the best ways to help people use your code. To write a (Python)
example, create a file `myexample.py` in the example directory of an
appropriate module, along with a file `myexample.readme.` The readme
should provide a brief overview of what the code in the module is
trying to accomplish as well as key pieces of IMP functionality that
it uses.

When writing examples, one should try (as appropriate) to do the following:
- begin the example with `import` lines for the IMP modules used
- have parameters describing the process taking place. These include names of
  PDB files, the resolution to perform computations at etc.
- define a function `create_representating` which creates and returns the model
  with the needed particles along with a data structure so that key
  particles can be located. It should define nested functions as
  needed to encapsulate commonly used code
- define a function `create_restraints` which creates the  restraints to score
  conformations of the representation
- define a function `get_conformations` to perform the sampling
- define a function `analyze_conformations` to perform some sort of clustering
  and analysis of the resulting conformations
- finally do the actual work of calling the `create_representation` and
  `create_restraints` functions and performing samping and analysis and
  displaying the solutions.

Obviously, not all examples need all of the above parts.

The example should have enough comments that the reasoning behind each line of code is clear to someone who roughly understands how IMP in general works.

Examples must use methods like IMP::base::get_example_data() to access
data in the example directory. This allows them to be run from
anywhere.


## Exporting code to Python ## {#devguide_swig}

IMP uses SWIG to wrap code C++ code and export it to Python. Since SWIG is
relatively complicated, we provide a number of helper macros and an example
file (see modules/example/pyext/swig.i-in). The key bits are
- the information goes into a file called swig.i-in in the module pyext directory
- the first part should be one `IMP_SWIG_VALUE(),` `IMP_SWIG_OBJECT()` or
  `IMP_SWIG_DECORATOR()` line per value type, object type or decorator object
  the module exports to Python. Each of these lines looks like

      IMP_SWIG_VALUE(IMP::module_namespace, ClassName, ClassNames);

- then there should be a number of `%include` lines, one per header file
  in the module which exports a class or function to Python. The header files
  must be in order such that no class is used before a declaration for it
  is encountered (SWIG does not do recursive inclusion)
- finally, any templates that are to be exported to SWIG must have a
  `%template` call. It should look something like

      namespace IMP {
        namespace module_namespace {
           %template(PythonName) CPPName<Restraint, 3>;
        }
      }



# Managing your own module # {#devguide_module}

When there is a significant group of new functionality, a new set of
authors, or code that is dependent on a new external dependency, it is
probably a good idea to put that code in its own module. To create a
new module, run [make-module.py](\ref dev_tools_make_module) script
from the main IMP source directory, passing the name of your new
module. The module name should consist of lower case characters and
numbers and the name should not start with a number. In addition the
name "local" is special and is reserved to modules that are internal
to code for handling a particular biological system or application. eg

     ./tools/make-module.py mymodule

The next step is to update the information about the module stored in
`modules/mymodule/README.md`. This includes the names of the authors and
descriptions of what the module is supposed to do.

If the module makes use of external libraries. See the files `modules/base/dependencies.py` and `modules/base/dependency/Log4CXX.description`
for examples.

Each module has an auto-generated header called `modulename_config.h.`
This header contains basic definitions needed for the module and
should be included (first) in each header file in the module. In
addition, there is a header `module_version.h` which contains the
version info as preprocessor symbols. This should not be included in
module headers or cpp files as doing so will force frequent
recompilations.




# Contributing code back to the repository # {#devguide_contributing}

In order to be shared with others as part of the IMP distribution,
code needs to be of higher quality and more thoroughly vetted than
typical research code. As a result, it may make sense to keep the
code as part of a private module until you better understand what
capabilities can be cleanly offered to others.

The first set of questions to answer are

- What exactly is the functionality I would like to contribute? Is
  it a single function, a single Restraint, a set of related classes
  and functions?

- Is there similar functionality already in IMP? If so, it might make
  more sense to modify the existing code in cooperation with its
  author. At the very least, the new code needs to respect the
  conventions established by the prior code in order to maintain
  consistency.

- Where should the new functionality go? It can either be added to an
  existing module or as part of a new module. If adding to an existing
  module, you must communicate with the authors of that module to get
  permission and coordinate changes.

- Should the functionality be written in C++ or Python? In general, we
  suggest C++ if you are comfortable programming in that language as
  that makes the functionality available to more people.

You are encouraged to post to the
`imp-dev` list to find help
answering these questions as it can be hard to grasp all the various
pieces of functionality already in the repository.

All code contributed to IMP
- must follow the  [IMP coding conventions](#devguide_conventions)
- should follow general good [C++ programming practices](#devguide_cpp)
- must have unit tests
- must pass all unit tests
- must have documentation
- must build on all supported compilers (roughly, recent versions of `gcc`,
  `clang++` and `Visual C++`) without warnings
- should have examples
- must not have warnings when its doc is built

See [getting started as a developer](https://github.com/salilab/imp/wiki/Getting-started-as-a-developer) for more information on submitting code.

## Once you have submitted code ## {#devguide_supporting}

Once you have submitted code, you should monitor the [Nightly build
status](http://www.salilab.org/imp/nightly/results/) to make sure that
your code builds on all platforms and passes the unit tests. Please
fix all build problems as fast as possible.

In addition to monitoring the `imp-dev` list, developers who have a module or
are committing patches to svn may want to subscribe to the `imp-commits` email
list which receives notices of all changes made to the IMP repository.


## Cross platform compatibility ## {#devguide_cross_platform}

IMP is designed to run on a wide variety of platforms. To detect problems on
other platforms
we provide nightly test runs on the supported
platforms for code that is part of the IMP repository.

In order to make it more likely that your code works on all the supported platforms:
- use the headers and classes in IMP::compatibility when appropriate
- avoid the use of `and` and `or` in C++ code, use `&&` and `||` instead.
- avoid `friend` declarations involving templates, use the preprocessor,
  conditionally on the symbols `SWIG` and `IMP_DOXYGEN` to hide code as
  needed instead.

### C++ 11 ### {#devguide_cxx11}
IMP now turns on C++ 11 support when it can. However, since compilers
are still quite variable in which C++ 11 features they support, it is
not adviseable to use them directly in IMP code at this point. To aid
in their use when practical we provide several helper macros:
- IMP_OVERRIDE inserts the `override` keyword when available
- IMP_FINAL inserts the `final` keyword when available

More will come.

# Good programming practices # {#devguide_cpp}

Two excellent sources for general C++ coding guidelines are

- [C++ Coding Standards](http://www.amazon.com/Coding-Standards-Rules-Guidelines-Practices/dp/0321113586) by Sutter and Alexandrescu

- [Effective C++](http://www.amazon.com/Effective-Specific-Addison-Wesley-Professional-Computing/dp/0201924889) by Meyers

IMP endeavors to follow all the of the guidelines published in those
books. The Sali lab owns copies of both of these books that you
are free to borrow.


# IMP gotchas # {#devguide_gotchas}

Below are a suggestions prompted by bugs found in code submitted to IMP.

- Never use '`using` `namespace'` outside of a function; instead
      explicitly provide the namespace. (This avoids namespace pollution, and
      removes any ambiguity.)

- Never use the preprocessor to define constants. Use `const`
  variables instead. Preprocessor symbols don't have scope or type
  and so can have unexpected effects.

- Don't expect IMP::base::Object::get_name() names to be unique, they
  are there for human viewing. If you need a unique identifier
  associated with an object or non-geometric value, just use the
  object or value itself.

- Pass other objects by value or by `const` & (if the object is
      large) and store copies of them.

- Never expose member variables in an object which has
      methods. All such member variables should be private.

- Don't derive a class from another class simply to reuse some
      code that the base class provides - only do so if your derived
      class could make sense when cast to the base class. As above,
      reuse existing code by pulling it into a function.

- Clearly mark any file that is created by a script so that other
      people know to edit the original file.

- Always return a `const` value or `const` ref if you are not
      providing write access. Returning a `const` copy means the
      compiler will report an error if the caller tries to modify the
      return value without creating a copy of it.

- Include files from the local module first, then files from the
    other IMP modules and kernel and finally outside includes. This
    makes any dependencies in your code obvious, and by including
    standard headers \e after IMP headers, any missing includes in the
    headers themselves show up early (rather than being masked by
    other headers you include).

        #include <IMP/mymodule/mymodule_exports.h>
        #include <IMP/mymodule/MyRestraint.h>
        #include <IMP/Restraint.h>
        #include <vector>

- Use `double` variables for all computational intermediates.

- Avoid using nested classes in the API as SWIG can't wrap them
      properly.  If you must use use nested classes, you will have to
      do more work to provide a Python interface to your code.


- Delay initialization of keys until they are actually needed
  (since all initialized keys take up memory within each particle,
  more or less). The best way to do this is to have them be static
  variables in a static function:

      FloatKey get_my_float_key() {
               static FloatKey k("hello");
               return k;
      }

- One is the almost always the right number:
        - Information should be stored in exactly one
          place. Duplicated information easily gets out of sync.
        - A given piece of code should only appear once. Do not copy,
          paste and modify to create new functionality.  Instead,
          figure out a way to reuse the existing code by pulling it
          into an internal function and adding extra parameters. If
          you don't, when you find bugs, you won't remember to fix
          them in all the copies of the code.
        - There should be exactly one way to represent any particular
          state. If there is more than one way, anyone who writes
          library code which uses that type of state has to handle all
          ways.  For %example, there is only one scheme for
          representing proteins, namely the IMP::atom::Hierarchy.
        - Each class/method should do exactly one thing. The presence
          of arguments which dramatically change the behavior of the
          class/method is a sign that it should be split. Splitting
          it can make the code simpler, expose the common code for
          others to use and make it harder to make mistakes by
          getting the mode flag wrong.
        - Methods should take at most one argument of each type (and
          ideally only one argument). If there are several arguments
          of the same types (eg two different `double` parameters) it is
          easy for a user to mix up the order of arguments and the compiler will
          not complain. `int` and `double` count as
          equivalent types for this rule since the compiler will
          transparently convert an `int` into a `double.`


# Further reading # {#devguide_further_reading}

- [Developer tools](\ref dev_tools)
- [Developer FAQ](http://github.com/salilab/imp/wiki/FAQ-for-developers)
- [Internals](http://github.com/salilab/imp/wiki/Internals).
