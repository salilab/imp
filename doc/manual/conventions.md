Conventions {#conventions}
===========

[TOC]

%IMP tries to make things simpler to use by adhering to various naming
and interface conventions.

# Names {#conventions_names}

- Names in `CamelCase` are class names, for %example IMP::RestraintSet
- Lower case names separated with underscores (`_`) in them are functions or methods, for example IMP::Model::update() or IMP::Model::add_particle().
- Collections of data of a certain class, e.g. `ClassName` are passed using type `ClassNames`. This type is a `list` in Python and a IMP::base::Vector<ClassName> (which is roughly equivalent to std::vector<ClassName*>) in C++.
- These function names start with a verb, which indicates what the method does. Methods starting with
   - `set_` change some stored value
   - `get_` create or return a \c value object or
     return an existing IMP::base::Object class object
   - `create_`  create a new IMP::base::Object class object
   - `add_`, `remove_` or `clear_` manipulate the contents of a collection of data
   - `show_` print things in a human-readable format
   - `load_` and `save_` or `read_` and `write_` move data between files and memory
   - `link_` create a connection between something and an IMP::base::Object
   - `update_` change the internal state of an IMP::base::Object
   - `do_` is a virtual method as part of a \external{http://en.wikipedia.org/wiki/Non-virtual_interface_pattern,non-virtual interface pattern}
   - `handle_` take action when an event occurs
   - `validate_` check the state of data and print messages and throw exceptions if something is corrupted
   - `setup_` and `teardown_` create or destroy some type of invariant (e.g. the constraints for a rigid body)
   - `apply_` either apply a passed object to each piece of data in some collection or apply the object itself to a particular piece of passed data (this is a bit ambiguous)
- names starting with `IMP_` are preprocessor symbols (C++ only)
- names don't use abbreviations

# Graphs {#conventions_graphs}

Graphs in %IMP are represented in C++ using the \external{http://www.boost.org/doc/libs/release/libs/graph, Boost Graph Library}. All graphs used in IMP are \external{http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/VertexAndEdgeListGraph.html, VertexAndEdgeListGraphs}, have vertex_name properties,
and are \external{http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/BidirectionalGraph.html, BidirectionalGraphs} if they are directed.

The Boost.Graph interface cannot be easily exported to Python so we instead provide a simple wrapper IMP::PythonDirectedGraph. There are methods to translate the graphs into various common Python and other formats (e.g. graphviz).


# Values and Objects (C++ only) {#conventions_values}

As is conventional in C++, IMP classes are divided into broad, exclusive types
- *Object classes*: They inherit from IMP::base::Object and are always passed by pointer. They are reference counted and so should only be stored using IMP::base::Pointer in C++ (in Python everything is reference counted). Never allocate these on the stack as very bad things can happen. Objects cannot be duplicated. Equality on objects is defined as identity (e.g. two different objects are different even if the data they contain is identical).

- *Value classes* which are normal data types. They are passed by value (or `const&`), never by pointer. Equality is defined based on the data stored in the value. Most value types in IMP are always valid, but a few, mostly geometric types (IMP::algebra::Vector3D) are designed for fast, low-level use and are left in an uninitialized state by their default constructor.

- *RAII classes* control some particular resource using the [RAII idiom](http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization).
They grab control of a resource when created and then free it when they are destroyed. As a result, they cannot be copied. Non-IMP examples include things like files in Python, which are automatically closed when the file object is deleted.

All types in %IMP, with a few documented exceptions, can be
- compared to other objects of the same type
- output to a C++ stream or printed in Python
- meaningfully put into Python dictionaries or C++ hash maps
