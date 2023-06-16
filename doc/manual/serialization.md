Serialization {#serialization}
=============

Most %IMP types can be serialized - that is, the internal state of an object,
such as the values of its member variables, can be written to or read in from
a file, string or stream. This allows for individual objects or an entire %IMP
run to be saved and later restored, or to be sent from one machine to another.
In Python, the objects can be loaded or saved using the `pickle` module.

Serialization uses a compact binary format. However, it is not heavily
optimized for size. If a smaller file size is desired, the Python `pickle`
file can be compressed with a general-purpose compression tool such as `gzip`.

Note that IMP::Model is handled specially due to its large size. Objects that
refer to the model (such as restraints or particles) will only include the ID
of the model in the serialization stream, not the model itself.
On deserialization these objects will be reassociated with the model by matching
the ID. This requires that the model be deserialized before any of these other
objects. To ensure this, `pickle` a tuple, list, or other ordered Python
container containing the model before any other object.

Serialization relies on the excellent
[cereal](https://uscilab.github.io/cereal/) library, which is required to
build %IMP. 

To add serialization to a new class, simply add a suitable serialization
function, as per the
[cereal docs](https://uscilab.github.io/cereal/serialization_functions.html).
Most %IMP classes define a `serialize` private method. The class will also
need a default constructor (i.e. one that takes no arguments) if it does not
already have one. If the class is visible to Python, make sure pickle support
is enabled by using the `IMP_SWIG_OBJECT_SERIALIZE` or
`IMP_SWIG_VALUE_SERIALIZE` macros in the SWIG interface.

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
