/**
 *  \file IMP/rmf/atom_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/atom_links.h>

IMPRMF_BEGIN_NAMESPACE

IMP_DEFINE_LINKERS(Hierarchy, hierarchy, hierarchies, atom::Hierarchy,
                   atom::Hierarchies,
                   (RMF::FileConstHandle fh, Model *m), (fh, m));

IMPRMF_END_NAMESPACE
