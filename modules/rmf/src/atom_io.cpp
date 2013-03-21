/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/atom_links.h>

IMPRMF_BEGIN_NAMESPACE

IMP_DEFINE_LINKERS(Hierarchy, hierarchy, hierarchies,
                   atom::Hierarchy,atom::Hierarchies,
                   atom::Hierarchy,atom::Hierarchies,
                   (RMF::FileHandle fh),
                   (RMF::FileConstHandle fh,
                    Model *m), (fh), (fh, m),
                   (fh, IMP::internal::get_model(hs)));

void set_save_forces(RMF::FileHandle fh, bool tf) {
  HierarchySaveLink *link= get_hierarchy_save_link(fh);
  link->set_save_forces(tf);
}


IMPRMF_END_NAMESPACE
