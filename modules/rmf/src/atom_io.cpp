/**
 *  \file IMP/rmf/atom_io.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/atom_links.h>

IMPRMF_BEGIN_NAMESPACE

IMP_DEFINE_LINKERS(Hierarchy, hierarchy, hierarchies, atom::Hierarchy,
                   atom::Hierarchies,
                   (RMF::FileConstHandle fh, kernel::Model *m), (fh, m));

IMPRMF_END_NAMESPACE
