Change Log {#changelog}
==========

# 1.5.0 - 2023-03-22 # {#changelog_1_5_0}
- Windows builds now require MS Visual Studio 2015 or later (for full C++11
  support). The following macros for pre-C++11 environments are no longer
  needed and are deprecated: `RMF_NOEXCEPT`, `RMF_CANEXCEPT`.
- All RMF binaries now report the full version (including micro version)
  when the --version flag is used (e.g. "1.4.1", not "1.4").
- If built with NumPy, some Python-specific functions are now provided to
  allow direct access to RMF data via NumPy arrays.
- File handles can now be explicitly closed (via a `close` method). Most IO
  operations on a closed handle will now raise an error. In Python file handles
  now support the context manager protocol so can be used in 'with' blocks.

# 1.4.1 - 2022-11-21 # {#changelog_1_4_1}
- Build fixes to work with SWIG 4.1.
- Various internal build scripts now use 'python3' rather than
  unversioned 'python'.

# 1.4 - 2022-03-10 # {#changelog_1_4}
- RMF now requires a C++11 compiler and SWIG 3 (or later) to build. Most
  recent compilers should support C++11, such as gcc, clang
  or MS Visual Studio 2012 or later. Various macro and header workarounds
  for pre-C++11 environments are thus no longer needed and are deprecated:
  `RMF_FINAL`, `RMF_OVERRIDE`, and `RMF_FOREACH`.
- Build fixes for latest log4cxx, in C++17 mode, or with newer versions of gcc.

# 1.3.1 - 2021-12-01 # {#changelog_1_3_1}
- Various build fixes for newer versions of gcc and clang.
- Build fixes for Fedora 33 or later.
- Add support for Python 3.10.
- Minor documentation improvements.

# 1.3 - 2021-04-21 # {#changelog_1_3}
- All RMF binaries now support the --version flag, and the RMF library itself
  is versioned.

# 1.2 - 2021-04-19 # {#changelog_1_2}
- A new category of decorators 'provenance' allows information about how the
  structure was generated to be added to the file.
- The new RMF::decorator::Reference decorator allows for a node to mark
  another node as its reference.
- The new RMF::decorator::ExplicitResolution decorator allows attaching an
  explicitly-set resolution to a node. (This allows such values from IMP to
  be accurately stored in the file.)

# 1.1 - 2014-03-07 # {#changelog_1_1}
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
