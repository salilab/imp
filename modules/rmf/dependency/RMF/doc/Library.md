Library {#library}
=======

# Overview #

[TOC]

The RMF library provides an intermediate level interface to facilitate I/O of
RMF data into and out of programs. The primary classes of interest are
RMF::RootHandle representing the root of an RMF hierarchy and RMF::NodeHandle
representing a node in the hierarchy.

The file is automatically closed when the last handle to it is destroyed.

# Basics # {#basics}

The library defines many classes, some of which are implemented using C++
templates. Every class supports output to a `std::ostream` in C++ and
conversion to `str` in Python. In addition, every class can be compared
to other instances of the same class and can be inserted into hash tables both
in C++ and Python. The methods necessary to support these things are
omitted for brevity.

In addition, there is a typedef for each type for managing lists of the objects.
For example, a list of RMF::Category objects is passed using a RMF::Categories type.
It looks like a `std::vector` in `C++` and is a `list` in Python.

# Associations # {#associations}

The RMF wrapper has the concept of an association between nodes in
its hierarchy and objects in the program accessing. The methods
RMF::FileConstHandle::get_node_handle_from_association(),
RMF::NodeConstHandle::set_association() and
RMF::NodeConstHandle::get_assocation() can be used to take advantage of
this. The idea is that one can store pointers to the programmatic
data structures corresponding to the nodes and so avoid maintaining
ones own lookup table. Any function used in an association must support
a `get_uint()` function. An implementation is provided for pointers.

# Parallel I/O # {#parallel}

If RMF::FileHandle::flush() has been called since the last change, it is safe
to read the file from another process. Writing from more than one process is not
supported. Nor is reading or writing from more than one thread of the same
program.

# Invariants # {#invariants}

Currently, there is little explicit checking of invariants between attributes
in the RMF file. An extensible framework for checking invariants on file
close and open will be added.

# Decorators and factories # {#rmfdecorators}

The nicest way to interact with data stored in nodes is through decorators and
factories (see the RMF::decorator). A factory (eg RMF::decorator::ParticleFactory or RMF::decorator::ParticleConstFactory) is
constructed from a file (RMF::FileHandle or RMF::FileConstHandle). It can be used
to check if given nodes have the requested data at a given frame (eg if the RMF::NodeConstHandle has
mass, coordinates and radius and so is an RMF::decorator::Particle) and construct a decorator
for that node at that frame. The decorator (eg RMF::decorator::Particle or RMF::decorator::ParticleConst)
can then be used to access the attributes and set them (for non-const variants). The
accessing is done through appropriately named functions (eg RMF::decorator::Particle::set_radius()).

# Language bindings # {#bindings}

The RMF library currently supports C++ and Python. The API is
written so that [SWIG](http://www.swig.org) can be used to
easily generate bindings for most languages. The two main
exceptions are C and Fortran. Until the SWIG %C target support is
finished, these can be supported by writing a simple %C %API
manually, probably a week's work.
