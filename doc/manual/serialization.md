Serialization {#serialization}
=============

Most %IMP types can be serialized - that is, the internal state of an object,
such as the values of its member variables, can be written to or read in from
a file or string. This allows for individual objects or an entire %IMP run
to be saved and later restored. In Python, the objects can be loaded or
saved using the `pickle` module.

Serialization relies on the excellent
[cereal](https://uscilab.github.io/cereal/) library, which is required to
build %IMP. 

To add serialization to a new class, simply add a suitable serialization
function, as per the
[cereal docs](https://uscilab.github.io/cereal/serialization_functions.html).
Most %IMP classes define a `serialize` private method. If the class is visible
to Python, make sure pickle support is enabled by using the
`IMP_SWIG_OBJECT_SERIALIZE` or `IMP_SWIG_VALUE_SERIALIZE` macros in the SWIG
interface.

If a class is polymorphic - i.e. it is a subclass that is referenced somewhere
by a base class pointer, such as a `Restraint` subclass - then the
serialization subsystem will need to store the name of the subclass in the
serialized output and will need to know how to access derived class
information. This is done by adding the `IMP_OBJECT_SERIALIZE_DECL` and
`IMP_OBJECT_SERIALIZE_IMPL` macros to the `.h` file and `.cpp` file
respectively. (These macros are used instead of cereal's `CEREAL_REGISTER_TYPE`
macro.)

The serialization format is subject to change. It should not be considered
portable between different machines (e.g. 32-bit and 64-bit, or Linux and
Windows) or between different %IMP versions, although this could be addressed
in future if necessary.
