Writing new code {#new_code}
================

The easiest way to start writing new functions and classes is to
create a new module using the [make-module.py](\ref dev_tools_make_module)
script.  This creates a new module in the `modules` directory. Alternatively,
you can simply use the `scratch` module.

We highly recommend using a revision control system such as
[git](http://git-scm.com/) to keep track of changes to your module.

If, instead, you choose to add code to an existing module, you need to
consult with the maintainer of that module. Their GitHub username
can be found on the [module main page](../ref/namespaces.html).

When designing the interface for your new code, you should

- search %IMP for similar functionality and, if there is any, adapt
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
in %IMP.
