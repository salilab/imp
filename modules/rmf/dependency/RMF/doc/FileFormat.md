File Format {#format}
===========

# Overview #

[TOC]

The %RMF file format (short for Rich Molecular Format) stores
hierarchical data about a molecular structure in a file or buffer in
memory. This data can include
- molecular structures stored hierarchically. These structures need
not be atomic resolution.
- feature information about parts of the structures, such as how well it
fits a particular measurement.
- geometric markup such as segments, surfaces, balls, colors which can
be used to improve visualization

For example, a protein can be stored as a hierarchy where the root is
the whole molecule. The root has one node per chain, each chain has
one node per residue and each residue one node per atom. Each node in
the hierarchy has the appropriate data stored along with it: a chain
node has the chain identifier, and a residue node has the type of the
residue stored and atom nodes have coordinates, atom type and
elements. Bonds between atoms or coarser elements are stored
explicitly as dealing with external databases to generate bonds is the
source of much of the difficulty of dealing with other formats such as
PDB.

The file might also include a node for storing the r-value for a FRET
measurement between two residues (with links to the residues) as well
as extra markers to highlight key parts of the molecule.

Multiple conformations on the hierarchy are stored as frames.  Each
frame has the same hierarchical structure, but some aspects of the
data (eg coordinates) can have different values for each frame (or no
value for a particular frame if they happen to not be applicable then).

A hierarchical storage format was chosen since
- most biological molecules have a natural hierarchical structure
- it reduces redundancy (eg the residue type is only stored once, as is
the residue index)
- most software uses a hierarchy of some sort to represent structures at
runtime, so less translation is needed
- low resolution and multiresolution structures are then more natural
as they are just truncations of a full, atomic hierarchy.

In addition to structural information, the file can also store
information about
- bonds
- how different parts of the structure relate to different experimental data
- different organization schemes on the structure
- arbitrary extra data needed by other programs
- associated authorship and publication information

# Examples # {#example_files}

Simple example:
- [simple pdb](simple.pdb.txt)
- [simple hierarchy](simple.txt) for the hierarchy and data stored
- [simple on disk](simple.rmf3.txt) for how the data is stored in an RMF3 file.

Larger pdb:
- [3U7W pdb](3U7W.pdb.txt)
- [3U7W hierarchy](3U7W.txt) for the hierarchy and data stored
- [3U7W on disk](3U7W.rmf.txt) for how the data is stored in an RMF3 file.


# The RMF Hierarchy # {#hierarchy}

More technically, each node in the %RMF hierarchy has

- a type (RMF::NodeType)

- a human readable name (RMF::NodeHandle::get_name())

- an ID that is unique within the file (RMF::NodeHandle::get_id())

- and associated attributes.

One accesses nodes in the hierarchy using handles, RMF::NodeHandle and
RMF::NodeConstHandle. The root handle can be fetched from the
RMF::FileHandle using RMF::FileHandle::get_root_node().

 # Attributes # {#attribute}

Each attribute is identified by a key (eg RMF::IntKey) and is defined by a
unique combination of

- a \ref decoratorsattributes "category" such as \ref physics "physics",
identified by an RMF::Category.

- a name string

On a per %RMF basis, the data associated with a given key can either
have one value for each node which has that attribute, or one value
per frame per node with the attribute. The methods in RMF::NodeHandle
to get and set the attributes take an optional frame number.

The library provides decorators to group together and provide easier
manipulation of standard attributes. Examples include RMF::Particle,
RMF::decorator::Colored, RMF::decorator::Ball etc. See [Decorators](\ref decoratorsattributes) for
more information.

The data types that can currently be stored in an RMF file are
| Name    |  Description | C++ type | Python type |
|--------:|--------------|:--------:|:-----------:|
| RMF::Float   | a floating point value | `float` | `float` |
| RMF::String  | a utf8 string | `std::string` | `str` |
| RMF::Int | an 64 bit integer | `int` | `int`|
| RMF::Vector3 | three Float values | RMF::Vector3 | RMF.Vector3 |
| RMF::Vector4 | four Float values | RMF::Vector4 | RMF.Vector4 |

RMF::String can be used either to store arbitrary text, or paths to files.
Paths are stored as relative paths, relative to the directory containing
the RMF file. This ensures that if the entire directory structure containing
the RMF file is archived, the paths are still correct. The convention is that
string attributes containing paths are named ending in "filename" or
"filenames". Special handling is done on such attributes (e.g. if an RMF
is moved to a different directory with `rmf_slice` or `rmf_cat`, the relative
paths of the static frame are updated accordingly).

In addition, an arbitrary length list of any of the above can be stored. The type for that
is the type for the single data with an \c s on the end, eg \c Floats for a list of \c Float
values. These are passed as \c std::vector like lists in \c C++ and \c lists in \c Python.

Each data type has associated typedefs such as
| Name    | Type  |Role                                       |
|--------:|:-----:|--------------------------------------|
|RMF::Float | `float` | the type used to pass a floating point value |
|RMF::Floats | std::vector<RMF::Float> | the type used to pass a list of floating point values. It looks like an \c std::vector in \c C++ and a \c list in \c Python |
|RMF::FloatKey | RMF::ID<RMF::FloatTraits> | a RMF::Key used to identify a floating point value associated with a node in the RMF hierarchy |
|RMF::FloatsKey | std::vector<RMF::FloatKey> | a RMF::Key used to identify a list of floating points value associated with a node in the RMF hierarchy |
|RMF::FloatTraits | RMF::FloatTraits | a traits classes to tell HDF5 how to read and write one or more floating point values |
|RMF::FloatsTraits | RMF::FloatsTraits | a traits classes to tell HDF5 how to read and write one or more lists of floating point values |


# Inheritance of properties # {#inheritance}

Certain nodes modify how their children should behave. This
modification can be either through inheritance (eg all descendants are
assumed to have the property unless they explicitly override it) or
composition (the descendant's property is the ancestors composed with
theirs). Note that since a given node can be reached through multiple
path in the hierarchy, a given view of the file might have to have
multiple objects (eg graphics) for a single node.

Current examples are
- RMF::decorator::Colored is inherited. That is, a node that is not an
RMF::decorator::Colored node, has the color of its closest
RMF::decorator::Colored ancestor.
- The RMF::decorator::Particle and RMF::decorator::RigidParticle coordinates
are transforms of any RMF::decorator::ReferenceFrame(s) that occur on the
particle itself or its ancestors. That is, a node that is a
RMF::decorator::Particle or RMF::decorator::Ball with an ancestor that
is an RMF::decorator::ReferenceFrame has global coordinates at the
RMF::decorator::ReferenceFrame's transformation applied to its coordinates.
See the example reference_frames.py.

# Frames # {#frames}

Each RMF file stores one or more frames (conformations). The
attributes of a node in a given conformation are the union of
conformation-specific attributes as well as static attributes (values
that hold for all frames).

As with nodes, frames have a hierarchical relationship to one
another. This hierarchy supports natural representation of clustering
results (eg you have a frame for the cluster center with a child frame
for each conformation that is in the cluster). By convention,
sequential frames in a simulation should be stored as with the
successor frame as a child of the predecessor.

Frames also have arbitrary attributes associated with them, like
nodes.


# Adding custom data to an RMF # {#adding}

When adding data to an %RMF file that is just to be used for internal
consumption, one should create a new category. For example,
[IMP](https://integrativemodeling.org) defines an ''imp'' category
when arbitrary particle data are stored.

If, instead, the data is likely to be a general interest, it probably
makes sense to add it to the documentation of this library so that the
names used can be standardized.

# On disk format # {#on_disk}

The RMF library has supported various on-disk formats. Currently 3 output
methods are supported: files with suffix `.rmf` and `.rmfz` and buffers
in memory.

The current format stores the structure in an [Avro Object
Container](https://avro.apache.org/docs/1.7.5/spec.html#Object+Container+Files). If
the `.rmfz` suffix is used, the contents are compressed. The structure
is stored as a series of records, each containing either a frame or
static data (there can be multiple static data frames - they are
implicitly merged).  Upon opening, the file is scanned once; after
that, frames can be accessed in a random access fashion. See
[Frame.json](Frame.json) for the schema.

The format is robust to corruption (all on disk data are safe if
garbage data is written or the process is killed).

There are several ways that the files can be made more compact
(without breaking forwards compatibility of existing files). They can be investigated
further if there is sufficient demand.

If [HDF5](https://support.hdfgroup.org/HDF5/) is available when RMF is built,
wrappers for it
will be built and support for older HDF5-based RMF formats will be compiled.

# Benchmarks # {#benchmarks}

A quick comparison of the various options (taken from benchmark/benchmark_rmf.cpp).

| type | create | write frame | open  | traverse | read frame | size |
|-----:|-------:|------------:|------:|---------:|-----------:|------|
|rmf   | 0.11   | 0.05        | 0.2   | 0.03     | 0.04       | 14M  |
|rmfz  | 0.09   | 0.10        | 0.5   | 0.03     | 0.05       | 10M  |
|buffer| 0.09   | 0.10        | 0.5   | 0.03     | 0.05       | 10M  |

The operations are:
- `create`: create a RMF file with a hierarchy with 45000 particles
- `write frame`: save coordinates for those particles
- `open`: open the file
- `traverse`: traverse the loaded hierarchy, touching atom, residue and chain data
- `read frame`: load the coordinates from a frame
- `size`: the size of the file. The raw data saved is 11M.

Note that the file stayed in RAM for these operations (hence the identical buffer and to disk times).
