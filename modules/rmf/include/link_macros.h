/**
 *  \file IMP/rmf/link_macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPRMF_LINK_MACROS_H
#define IMPRMF_LINK_MACROS_H
#include "links.h"
#include "associations.h"
#include "internal/link_helpers.h"
#include <IMP/base/Pointer.h>
#include <RMF/FileHandle.h>

// TODO: is there any good reason for the difference between InType(s)
//       and OutType(s) - probably not, need to consolidate
/**
   declares functions for adding, creating and linking
   RMF file objects with base calsses of type InType
 */
#define IMP_DECLARE_LINKERS(Name, name, names, Type, Types, cargs,    \
                            link_custom_documentation)                \
  /** Add objects to the file.                                        \
      \note This does not save a configuration, make sure you use     \
      save_frame() to do that. */                                     \
  IMPRMFEXPORT void add_##names(RMF::FileHandle fh, const Types& hs); \
  /** Add objects to the file under the specified node.               \
      \note This does not save a configuration, make sure you         \
      use                                                             \
      save_frame() to do that. */                                     \
  IMPRMFEXPORT void add_##names(RMF::NodeHandle fh, const Types& hs); \
  /** Add a single Name object to the RMF file.                       \
      \note This does not save a configuration, make                  \
      sure you use                                                    \
      save_frame() to do that. */                                     \
  IMPRMFEXPORT void add_##name(RMF::FileHandle fh, Type hs);          \
  /** Create Name objects from the RMF file.                          \
      \note This does not load a frame. Make sure you call            \
      IMP::rmf::load_frame() before using.*/                          \
  IMPRMFEXPORT Types create_##names cargs;                            \
  /** Link Name objects with the RMF file,                            \
      possibly overwriting an                                         \
      existing link for loading from the                              \
      file. This does not alter the                                   \
      object, but will affect the behaviour                           \
      of functions like load_frame()                                  \
      and save_frame().                                               \
                                                                      \
      link_custom_documentation                                       \
  */                                                                  \
  IMPRMFEXPORT void link_##names(RMF::FileConstHandle fh, const Types& hs)

// TODO: is there any good reason for the difference between InType(s)
//       and OutType(s) - probably not, need to consolidate
/**
   Define functions for adding, creating and linking
   RMF file objects with base calsses of type InType

   \see IMP_DECLARE_LINKERS()
 */
#define IMP_DEFINE_LINKERS(Name, name, names, Type, Types, create_args,     \
                           create_args_pass)                                \
  void add_##names(RMF::FileHandle fh, const Types& hs) {                   \
    internal::add_helper<Name##SaveLink>(fh, hs);                           \
  }                                                                         \
  void add_##names(RMF::NodeHandle fh, const Types& hs) {                   \
    internal::add_helper<Name##SaveLink>(fh.get_file(), hs);                \
  }                                                                         \
  void add_##name(RMF::FileHandle fh, Type hs) {                            \
    add_##names(fh, Types(1, hs));                                          \
  }                                                                         \
  Types create_##names create_args {                                        \
    return internal::create_helper<Name##LoadLink, Types> create_args_pass; \
  }                                                                         \
  void link_##names(RMF::FileConstHandle fh, const Types& hs) {             \
    internal::link_helper<Name##LoadLink>(fh, hs);                      \
  }

#endif /* IMPRMF_LINK_MACROS_H */
