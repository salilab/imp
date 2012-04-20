/**
 *  \file link_macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPRMF_LINK_MACROS_H
#define IMPRMF_LINK_MACROS_H

#define IMP_DEFINE_LINKERS(Name, name, args, cargs,                     \
                           create_args, create_cargs)                   \
  Name##SaveLink *get_##name##_save_link args {                         \
    int index=get_save_linker_index(#name);                             \
    if (!fh.get_has_associated_data(index)) {                           \
      SaveLinkAssociationType psl= new Name##SaveLink create_args;      \
      fh.add_associated_data(index, psl);                               \
    }                                                                   \
    SaveLinkAssociationType ln                                          \
      = fh.get_associated_data<SaveLinkAssociationType>(index);         \
    return dynamic_cast<Name##SaveLink*>(ln.get());                     \
  }                                                                     \
  Name##LoadLink *get_##name##_load_link cargs {                        \
    int index=get_load_linker_index(#name);                             \
    if (!fh.get_has_associated_data(index)) {                           \
      LoadLinkAssociationType psl= new Name##LoadLink create_cargs;     \
      fh.add_associated_data(index, psl);                               \
    }                                                                   \
    LoadLinkAssociationType pt                                          \
      = fh.get_associated_data<LoadLinkAssociationType>(index);         \
    return dynamic_cast<Name##LoadLink*>(pt.get());                     \
  }

#endif /* IMPRMF_LINK_MACROS_H */
