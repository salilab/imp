Executables {#executables}
===========

# Overview #

[TOC]

RMF provides a number of useful executables in the `bin` directory.

# rmf_show # {#rmf_show}
`rmf_show` out the hierarchy in the file, optionally showing decorators and data.

    Show an rmf file as text.
    Usage: ./bin/rmf_show rmf
      -d [ --decorators ]   Show what decorators recognize each node.
      -f [ --frame ] arg    Frame to use
      -h [ --help ]         Get help on command line arguments.
      -v [ --verbose ]      Show attribute values for each node.


# rmf_frames # {#rmf_frames}
`rmf_frames` prints out the frame hierarchy in the passed file.

    Print out information about frames
    Usage: ./bin/rmf_frames rmf
      -h [ --help ]         Get help on command line arguments.

# rmf_info # {#rmf_info}

`rmf_info` displays information about the attributes found in the RMF and how
much they are used.

    Print out information about categories and keys.
    Usage: ./bin/rmf_info rmf
      -f [ --frame ] arg    Frame to use
      -h [ --help ]         Get help on command line arguments.


# rmf_slice # {#rmf_slice}

`rmf_slice` creates a new rmf from frames selected from an existing one.

    Grab frames from an rmf file
    Usage: ./bin/rmf_slice input_rmf output_rmf
      -f [ --frame ] arg      First (or only) frame to use
      -s [ --frame_step ] arg The step size for frames. Must be > 0.
      -h [ --help ]           Get help on command line arguments.

# rmf_cat # {#rmf_cat}

`rmf_cat` creates a new rmf by concatenating several. Each file must have the same hierarchical structure.

    Combine two or more rmf files.
    Usage: ./bin/rmf_cat input_1.rmf input_2.rmf ... output.rmf
      -h [ --help ]         Get help on command line arguments.
      -v [ --verbose ]      Produce more output.
      --hdf5-errors         Show hdf5 errors.
      --log-level arg       What log level to use: Trace, Info, Warn, Error, Off


# rmf_xml # {#rmf_xml}
`rmf_xml` prints out the hierarchy and data to an XML files that can be opened in an XML viewer
   (eg Google Chrome or Firefox). These viewers support collapsing of subtrees, which
   makes it much easier to get around large hierarchies.


# rmf_validate # {#rmf_validate}

`rmf_validate` checks for errors in an RMF file, it currently isn't very powerful.

# rmf3_dump # {#rmf3_dump}

`rmf3_dump` shows a low level dump of a 3rd generation RMF file (these are the current, avro-based files). It is primarily there for debugging.
