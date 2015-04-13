Writing new code {#new_code}
================

The easiest way to start writing new functions and classes is to
create a new module using the [make-module.py](\ref dev_tools_make_module)
script.  This creates a new module in the `modules` directory. Alternatively,
you can simply use the `scratch` module.

If, instead, you choose to add code to an existing module, you need to
consult with the maintainer of that module. Their GitHub username
can be found on the [module main page](../ref/namespaces.html).

Either way, we highly recommend doing this in a [git clone](@ref devsetup),
as described earlier, then using [git](@ref faq_git) to keep track of
changes to your code.

When designing the interface for your new code, you should

- think about what exactly is the functionality you want to contribute. Is
  it a single function, a single Restraint, a set of related classes
  and functions?

- search %IMP for similar functionality and, if there is any, adapt
  the existing interface for your purposes. (It might make more sense to
  modify the existing code in cooperation with its author.) For example,
  the existing
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
  IMP::Refiner, a IMP::SingletonContainer or just an arbitrary
  IMP::ParticleIndexes object. Similarly, rather than writing a Restraint,
  it may make more sense to write an IMP::PairScore or IMP::UnaryFunction;
  an IMP::SingletonModifier may be more appropriate than an IMP::Constraint.

You are encouraged to post to the
[imp-dev list](http://integrativemodeling.org/contact.html) to find help
answering these questions as it can be hard to grasp all the various
pieces of functionality already in the repository.

See also the IMP::example module, which contains many examples of writing
new %IMP functionality in C++ or Python.
You may also want to read [the design example](\ref design_example) for
some suggestions on how to go about implementing your functionality
in %IMP.
