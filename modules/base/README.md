\brief Low level functionality (logging, error
handling, profiling, command line flags etc) that is used by all of IMP.

# Graphs # {#graphs}

Graphs in IMP are represented in C++ using the
[Boost.Graph](http://www.boost.org/doc/libs/release/libs/graph). All
graphs used in IMP are
[VertexAndEdgeListGraphs](http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/VertexAndEdgeListGraph.html),
have vertex_name properties, are
[BidirectionalGraphs](http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/BidirectionalGraph.html)
if they are directed.

The Boost.Graph interface cannot be easily exported to Python so we instead provide a simple wrapper IMP::PythonDirectedGraph. There are methods to translate the graphs into various common python and other formats (eg graphviz).


# Values and Objects (C++ only) # {#values}

As is conventional in C++, IMP classes are divided into broad, exclusive types
- *Object classes*: They inherit from IMP::base::Object and are always passed by pointer. They are reference counted and so should only be stored using IMP::base::Pointer (in C++, in Python everything is reference counted). Never allocate these on the stack as very bad things can happen. Objects cannot be duplicated. Equality on objects is defined as identity (eg two different objects are different even if the data they contain is identical).

- *Value classes* which are normal data types. They are passed by value (or `const&`), never by pointer. Equality is defined based on the data stored in the value. Most value types in IMP are always valid, but a few, mostly geometric types (IMP::algebra::Vector3D) are designed for fast, low-level use and are left in an uninitialized state by their default constructor

- *RAII classes* control some particular resource. They grab control of a resource when created and then free it when they are destroyed. As a result, they cannot be copied. Non-IMP examples include things like files in python, which are automatically closed when the file object is deleted.

All types in IMP, with a few documented exceptions, can be
- compared to other objects of the same type
- output to a C++ stream or printed in python
- meaningfully put into python dictionaries or C++ hash maps

# Google Perf Tools # {#perf}

The google perf tools can be used for cpu and memory profiling of IMP. They can
be controlled from the command line in many IMP executables.

# Info

_Author(s)_: Daniel Russel

_Maintainer_: `benmwebb`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
