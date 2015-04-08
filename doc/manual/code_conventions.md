Coding conventions {#code_conventions}
==================

To ensure code consistency and readability, certain conventions
must be adhered to when writing code for IMP. Some of these
conventions are automatically checked for by source control before
allowing a new commit, and can also be checked yourself in new
code by running [check_standards.py](#devguide_check_standards).

All new code should also conform to the previously-described
[naming and interface conventions](@ref conventions).


# Indentation {#codeconv_indentation}

All C++ headers and code should be indented with 2-space indents. Do not use
tabs. [clang-format](\ref dev_tools_clang_format) can help you do this formatting
automatically.

All Python code should conform to the [Python style
guide](http://www.python.org/dev/peps/pep-0008/).  In essence this
translates to 4-space indents, no tabs, and similar class, method and
variable naming to the C++ code. You can ensure that your Python code
is correctly indented by using the
[cleanup_code.py script](\ref dev_tools_clang_format).

# Names {#codeconv_names}

In addition to the previously-described
[naming and interface conventions](@ref conventions),
developers should be aware that
- all preprocessor symbols must begin with `IMP`.
- names of files that implement a single class should be named for that
  class; for example the `SpecialVector` class could be implemented in
  `SpecialVector.h` and `SpecialVector.cpp`
- files that provide free functions or macros should be given names
  `separated_by_underscores`, for `example `container_macros.h`
- Functions which take a parameter which has [units](@ref units) should have the
  unit as part of the function name, for %example
  IMP::atom::SimulationParameters::set_maximum_time_step_in_femtoseconds().
  Remember the Mars orbiter. The exception to this is distance and
  force numbers which should always be in angstroms and kcal/mol
  angstrom respectively unless otherwise stated.

# Passing and storing data {#codeconv_passing}

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
  objects of type `Name` using a `Names`. Declare functions that
  accept them to take a `NamesTemp` (`Names` is a `NamesTemp)`.
  `Names` are reference counted (see IMP::RefCounted for details);
  `NamesTemp` are not. Store collections of particles using a
  `Particles` object, rather than decorators.

# Display {#codeconv_display}

All values must have a `show` method which takes an optional
`std::ostream` and prints information about the object (see
IMP::base::Array::show() for an example). Add a `write` method if you
want to provide output that can be read back in.

# Errors {#codeconv_errors}

Classes and methods should use %IMP exceptions to report errors. See
IMP::base::Exception for a list of existing exceptions. See
[checks](exception_8h.html) for more information.

# Namespaces {#codeconv_namespace}

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
