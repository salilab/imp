This modules provides support for RMF file format for storing
hierarchical molecular data (such as atomic or coarse grained
representations of proteins), along with markup, including geometry
and score data.

 IMP.rmf supports I/O of IMP::atom::Hierarchy and associated types as
 well as output of IMP::display::Geometry, IMP::Restraint and
 arbitrary IMP::Particles. For each of these there are methods like:
 - IMP::rmf::add_hierarchies() to add them to an RMF file, note, this does not,
   for various reasons, write the state to frame 0.
 - IMP::rmf::create_hierarchies() to create hierarchies from an RMF file
 - IMP::rmf::link_hierarchies() to link existing hierarchies to a corresponding
   ones in the rmf file

 Once objects are linked/added/created, they are attached to the RMF file.
 IMP::rmf::load_frame() can be used to change the state of the linked objects
 to that of an arbitrary frame and IMP::rmf::save_frame() can be used to save
 the current state of the objects into a frame in the RMF file.

 See the [RMF library](http://salilab.github.com/rmf) for more information.


_Author(s)_: Daniel Russel

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
