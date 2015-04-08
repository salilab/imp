Developer Guide {#devguide}
===============

# Developing with IMP #
[TOC]

This page presents instructions on how to develop code using
IMP. Developers should also read [Getting started as a developer](https://github.com/salilab/imp/wiki/Getting-started-as-a-developer).

# Writing new code # {#devguide_new_code}

The easiest way to start writing new functions and classes is to
create a new module using [make-module.py](\ref dev_tools_make_module).
This creates a new module in the `modules` directory. Alternatively, you can
simply use the `scratch` module.

We highly recommend using a revision control system such as
[git](http://git-scm.com/) or [svn](http://subversion.tigris.org/) to
keep track of changes to your module.

If, instead, you choose to add code to an existing module, you need to
consult with the person or people who control that module. Their names
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
  IMP::kernel::Refiner, a IMP::kernel::SingletonContainer or just an arbitrary
  IMP::kernel::ParticleIndexes object.


You may want to read [the design example](\ref designexample) for
some suggestions on how to go about implementing your functionality
in IMP.

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
status](http://integrativemodeling.org/nightly/results/) to make sure that
your code builds on all platforms and passes the unit tests. Please
fix all build problems as fast as possible.

In addition to monitoring the `imp-dev` list, developers who have a module or
are committing patches to svn may want to subscribe to the `imp-commits` email
list which receives notices of all changes made to the IMP repository.


# Further reading # {#devguide_further_reading}

- [Developer tools](\ref dev_tools)
- [Developer FAQ](http://github.com/salilab/imp/wiki/FAQ-for-developers)
- [Internals](http://github.com/salilab/imp/wiki/Internals).
