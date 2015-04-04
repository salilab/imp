RMF {#rmf}
===

The IMP.rmf module acts as a link between RMF files and IMP data structures. Currently, three types of data are linked:
- IMP::kernel::Restraint
- IMP::atom::Hierarchy
- IMP::display::Geometry

For each type of data, there are a set of functions declared in `foo_io.h` and defined in `foo_io.cpp`. These functions support four major operations
- addition of new data to the RMF file: this creates a hierarchy of nodes in the RMF and stores a link between those nodes and the corresponding IMP::base::Objects
- creation of IMP structures from the RMF: IMP data structures are created based on corresponding data contained in the RMF file
- link existing IMP structures to the RMF: existing IMP data structures are linked to data stored it the RMF. The two must correspond exactly (eg the RMF must have been created by adding those IMP data structures to the file).
and

- saving a frame to a file: a new frame is added to the RMF file and all linked data is used to write data to that frame.

The links between IMP data structures are done via IMP::rmf::LoadLink and IMP::rmf::SaveLink. These are data structures associated with the RMF file (the RMF library provides a mechanism for this) that store persistent state that is available as long as the file is open. There is one load and one save link class per type of data.

## Hierarchy ##
atom::Hierarchy data is stored as RMF::REPRESENTATION nodes in the file.

There is exactly one node in the RMF file per IMP::kernel::Particle in the hierarchy. The link classes are implemented in terms of a bunch of helper classes, each of which handle a particular type of data.

The main tricky part is handing rigid bodies. In particular, the particles defining the rigid body are not always part of the IMP hierarchy and handling older RMF files where rigid bodies were written differently.

Rigid bodies that are not part of the IMP::atom::Hierarchy are handle by
- at each node in the hierarchy, checking if all core::XYZ particles in the subtree are part of the same rigid body
- if so, pretend that the current particle is the actual rigid body.
- since this can result in one IMP rigid body having many corresponding nodes in the RMF, a tag (an id integer) is added to such rigid bodies so they can be combined when creating from the file

When writing coordinates of things that are rigid bodies, the internal coordinates are written to the RMF rather than the global coordinates. This makes the files more compact. It works since the rigid bodies are written as RMF::decorator::ReferenceFrame nodes in the file, which effectively provides a transformation to the coordinates of things underneath it.

## Restraints ##
Restraints are stored as RMF::FEATURE nodes in the file.

Restraints are more or less write only, in contrast to the other types. There are create functions, but since the RMF doesn't contain enough data to actually create the correct IMP type, these just create dummy restraints whose score is simply the value stored in the file (and which don't have any derivatives). As a result, it is not clear that they can be used for much beyond verifying scores and some simple analysis.

Each IMP::kernel::Restraint added to the RMF is written as follows:
- a RMF node is created for the restraint itself, containing one score per frame
- IMP::kernel::Restraint::create_current_decomposition() is called on the restraint. If this decomposition contains more than one restraint, then child nodes are added for each restraint and they are stored recursively.
- all leaves have the (static) list of RMF nodes that they involve written

Mapping between the sub-restraints created by create_current_decomposition() and RMF nodes is slightly non-trivial. It is done based on the set up input particles for each restraint returned by create_current_decomposition(). For each such restraint, the link first checks in a map if there is already a node with that set of inputs, if so, it reuses that. If not, it creates a new one.

## Geometry ##
Geometry is stored as RMF::SHAPE nodes in the file.

The functionality provided in IMP::display::Geometry doesn't map entirely naturally on to RMF's geometry support. On the IMP side, things are implemented in terms of an ephemeral decomposition of complex geometric objects into simple ones (eg, a bounding box can be decomposed into edges, but each time you ask for the decomposition, you will get a different set of IMP::base::Objects). The RMF side expects a hierarchy whose structure stays constant across frames. As a result, each type of geometry object has to be special cased in IMP.rmf. Currently there is support for
- segments
- balls
- bounding boxes
- cylinders
- points

## Considerations ##
For each piece of data stored in the RMF, a decision has to be made whether to store it once per file (using the RMF set_static_foo() methods), once per frame (using the RMF set_frame_foo() methods), or automatically (using RMF set_foo() methods). In IMP.rmf, in general things are stored once per file with the exception of coordinates/transformations of things that are not rigid parts of rigid bodies, which are stored once per frame. This could be changed to using automatic for almost everything, but that is
- a bit slower as it has to check each time if the value is different than what is already stored
- not something that other programs expect for things like residue index
In particular, it might make sense for radii to become automatic.

One must be very careful not to store any RMF::NodeHandle, RMF::FileHandle objects in the adpators as these will keep the RMF file open indefinitely (since the handles will keep the file alive and the file keeps the adaptor alive and the adaptor keeps the handle alive).
