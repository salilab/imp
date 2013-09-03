/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/associations.h>
IMPRMF_BEGIN_NAMESPACE

void set_association(RMF::NodeConstHandle nh, base::Object *o, bool overwrite) {
  nh.set_association(AssociationType(o), overwrite);
}

RMF::NodeConstHandle get_node_from_association(RMF::FileConstHandle nh,
                                               base::Object *oi) {
  AssociationType o(oi);
  return nh.get_node_from_association(o);
}

RMF::NodeHandle get_node_from_association(RMF::FileHandle nh,
                                          base::Object *oi) {
  AssociationType o(oi);
  return nh.get_node_from_association(o);
}

bool get_has_associated_node(RMF::FileConstHandle nh, base::Object *oi) {
  return nh.get_node_from_association(oi) != RMF::NodeConstHandle();
}
bool get_has_linker(RMF::FileConstHandle fh, unsigned int id) {
  return fh.get_has_associated_data(id);
}

void set_linker(RMF::FileConstHandle fh, unsigned int id,
                SaveLinkAssociationType l) {
  fh.add_associated_data(id, l);
}

void set_linker(RMF::FileConstHandle fh, unsigned int id,
                LoadLinkAssociationType l) {
  fh.add_associated_data(id, l);
}

SaveLinkAssociationType get_save_linker(RMF::FileConstHandle fh,
                                        unsigned int id) {
  return fh.get_associated_data<IMP::rmf::SaveLinkAssociationType>(id);
}

LoadLinkAssociationType get_load_linker(RMF::FileConstHandle fh,
                                        unsigned int id) {
  return fh.get_associated_data<IMP::rmf::LoadLinkAssociationType>(id);
}

IMPRMF_END_NAMESPACE
