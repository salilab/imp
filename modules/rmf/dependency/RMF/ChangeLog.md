Change Log {#changelog}
==========

- The new RMF::decorator::Reference decorator allows for a node to mark
  another node as its reference.
- The new RMF::decorator::ExplicitResolution decorator allows attaching an
  explicitly-set resolution to a node. (This allows such values from IMP to
  be accurately stored in the file.)
- To be more consisent, RMF::decorator::Domain and
	RMF::decorator::Fragment had their access methods modified to
	include `residue` in the name.
- A Pymol plugin was added
- RMF now uses the new avro-based format with the `.rmf` suffix, you
	should use this and `.rmfz` as your suffixes.
- RMF::decorator::Bond and RMF::decorator::BondConst now return
	RMF::NodeHandle and RMF::NodeConstHandle instead of ints.
- RMF::decorator::Representation and RMF::decorator::RepresentationConst now return
	RMF::NodeHandles and RMF::NodeConstHandles instead of ints.
- RMF::decorator::Domain takes two ints in its set method rather than a list
- RMF::NodeConstHandle::get_index() went away as it was a duplicate of
	RMF::NodeConstHandle::get_id()
- RMF::NodeType and RMF::FrameType were replaced with RMF::Enum
	instances. This increases type safety in python and makes
	converting to and from strings easier (you can now use `str()` in python).
- RMF::decorator::Residue::get_residue_index() and
	RMF::decorator::Residue::get_residue_type() replace the versions
	without `residue` in the name
- Decorators and factories have been moved into the RMF::decorator namespace in C++.
- You can now only either create a new RMF or open an existing one read
	only. Most backends only supported those two anyway.
- Managing of data buffers has been revamped and wrapped in the
	RMF::BufferHandle class which tracks a shared buffer.
- You now get keys by passing the appropriate traits object instead of
	having the type in the function name: eg `f.get_key(cat, "name", RMF.IntTraits())`
- Appropriate decorators now return RMF::Vector3, RMF::Vector4 or
	RMF::Vector3s instead of RMF::Floats or RMF::FloatsList.
- There are now static and non-static versions of the decorators.
- You now have to explicitly add a frame or set the current frame before
	manipulating frame data.
- RMF::NodeConstHandle::get_value() subsumes get_value_always() which was
	removed. If there is no value, then either a RMF::Nullable for
	which RMF::Nullable::get_is_null() is true is returned (C++) or
	`None` is returned (python).
- RMF::FileHandle::get_node_from_id() was renamed to
	RMF::FileHandle::get_node()
- Various methods that took ints to identify frames now expect RMF::FrameID.
- A VMD plugin was added
