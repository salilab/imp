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
#include <IMP/base/Pointer.h>
#include <RMF/FileHandle.h>

#define IMP_DEFINE_INTERNAL_LINKERS(Name, name,args, cargs,          \
                                    create_args, create_cargs           \
                                    )                                   \
  namespace {                                                           \
  Name##SaveLink *get_##name##_save_link args {                         \
    int index=IMP::rmf::get_save_linker_index(#name);                   \
    if (!fh.get_has_associated_data(index)) {                           \
      RMF::SetCurrentFrame scf(fh, RMF::ALL_FRAMES);                    \
      IMP::rmf::SaveLinkAssociationType psl                             \
        = new Name##SaveLink create_args;                               \
      IMP::rmf::set_linker(fh, index, psl);                             \
    }                                                                   \
    IMP::rmf::SaveLinkAssociationType ln                                \
      = IMP::rmf::get_save_linker(fh, index);                           \
    return dynamic_cast<Name##SaveLink*>(ln.get());                     \
  }                                                                     \
  Name##LoadLink *get_##name##_load_link cargs {                        \
    int index=IMP::rmf::get_load_linker_index(#name);                   \
    if (!fh.get_has_associated_data(index)) {                           \
      RMF::SetCurrentFrame scf(fh, RMF::ALL_FRAMES);                    \
      IMP::rmf::LoadLinkAssociationType psl                             \
        = new Name##LoadLink create_cargs;                              \
      IMP::rmf::set_linker(fh, index, psl);                             \
    }                                                                   \
    IMP::rmf::LoadLinkAssociationType pt                                \
      = IMP::rmf::get_load_linker(fh, index);                           \
    return dynamic_cast<Name##LoadLink*>(pt.get());                     \
  }                                                                     \
  }                                                                     \

// TODO: is there any good reason for the difference between InType(s)
//       and OutType(s) - probably not, need to consolidate
/**
   declares functions for adding, creating and linking
   RMF file objects with base calsses of type InType

   @param Name camelcase nice name to use for all function
   @param name lowercase nice name
   @param names plural nice name
   @param InType type of base class
   @param InTypes type of base class list
   @param OutType type of base class, probably same as InType and will
                  be consolidated in the future
   @param OutTypes type of base class, probably same as OutTypes and will
                   be consolidated in the future
   @param cargs arguments to create_\#\#name
   @param link_custom_documentation
 */
#define IMP_DECLARE_LINKERS(Name, name, names, InType, InTypes,         \
                            OutType, OutTypes, cargs,                   \
                            link_custom_documentation)                  \
  /** Add objects to the file.
   \note This does not save a configuration, make sure you use
   save_frame() to do that. */                                          \
  IMPRMFEXPORT void add_##names(RMF::FileHandle fh,                     \
                                const OutTypes& hs);                    \
  /** Add objects to the file under the specified node.
   \note This does not save a configuration, make sure you use
   save_frame() to do that. */                                          \
IMPRMFEXPORT void add_##names(RMF::NodeHandle fh,                       \
                                const OutTypes& hs);                    \
/** Add a single Name object to the RMF file.
    \note This does not save a configuration, make sure you use
    save_frame() to do that. */                                         \
  IMPRMFEXPORT void add_##name(RMF::FileHandle fh, OutType hs);         \
  /** Create Name objects from the RMF file.
      \note This does not load a frame. Make sure you call
      IMP::rmf::load_frame() before using.*/                            \
IMPRMFEXPORT InTypes create_##names cargs;                              \
/** Link Name objects with the RMF file, possibly overwriting an
    existing link for loading from the file. This does not alter the
    object, but will affect the behaviour of functions like load_frame()
    and save_frame().

    link_custom_documentation
*/                                                                      \
  IMPRMFEXPORT void link_##names(RMF::FileConstHandle fh,               \
                                 const InTypes &hs)


// TODO: is there any good reason for the difference between InType(s)
//       and OutType(s) - probably not, need to consolidate
/**
   Define functions for adding, creating and linking
   RMF file objects with base calsses of type InType

   \see IMP_DECLARE_LINKERS()

   @param Name camelcase nice name to use for all function
   @param name lowercase nice name
   @param names plural nice name
   @param InType type of base class
   @param InTypes type of base class list
   @param OutType type of base class, probably same as InType and will
                  be consolidated in the future
   @param OutTypes type of base class, probably same as OutTypes and will
                   be consolidated in the future
   @param cargs arguments to create_\#\#name
 */
#define IMP_DEFINE_LINKERS(Name, name, names, InType, InTypes,          \
                           OutType, OutTypes,args, cargs,               \
                           create_args, create_cargs, create_cargs_from) \
  IMP_DEFINE_INTERNAL_LINKERS(Name, name, args, cargs,                  \
                              create_args, create_cargs);               \
  void add_##names(RMF::FileHandle fh,                                  \
                   const OutTypes& hs) {                                \
    if (hs.empty()) return;                                             \
    Name##SaveLink* hsl= get_##name##_save_link(fh);                    \
    RMF::SetCurrentFrame scf(fh, RMF::ALL_FRAMES);                      \
    hsl->add(fh.get_root_node(), hs);                                   \
  }                                                                     \
  void add_##names(RMF::NodeHandle fh,                                  \
                   const OutTypes& hs) {                                \
    if (hs.empty()) return;                                             \
    Name##SaveLink* hsl= get_##name##_save_link(fh.get_file());         \
    RMF::SetCurrentFrame scf(fh.get_file(), RMF::ALL_FRAMES);           \
    hsl->add(fh, hs);                                                   \
  }                                                                     \
  void add_##name(RMF::FileHandle fh, OutType hs) {                     \
    add_##names(fh, OutTypes(1, hs));                                   \
  }                                                                     \
  InTypes create_##names cargs {                                        \
    Name##LoadLink* rsl= get_##name##_load_link create_cargs;           \
    InTypes ret;                                                        \
    {                                                                   \
      /* to catch the type of everything.*/                             \
      RMF::SetCurrentFrame scf(fh, 0);                                  \
      ret= rsl->create(fh.get_root_node());                             \
      rsl->load(fh);                                                    \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  void link_##names(RMF::FileConstHandle fh,                            \
                        const InTypes &hs) {                            \
    if(hs.empty()) return;                                              \
    base::Pointer<Name##LoadLink> pll                                   \
        = get_##name##_load_link create_cargs_from;                     \
    RMF::SetCurrentFrame scf(fh, RMF::ALL_FRAMES);                      \
    pll->link(fh.get_root_node(), hs);                                  \
  }

#endif /* IMPRMF_LINK_MACROS_H */
