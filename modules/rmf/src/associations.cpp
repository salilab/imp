/**
 *  \file IMP/rmf/associations.cpp
 *  \brief Track associations between an RMF file and native objects.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/associations.h>
IMPRMF_BEGIN_NAMESPACE

void set_association(RMF::NodeConstHandle nh, Object *o, bool overwrite) {
  nh.set_association(AssociationType(o), overwrite);
}

RMF::NodeConstHandle get_node_from_association(RMF::FileConstHandle nh,
                                               Object *oi) {
  AssociationType o(oi);
  return nh.get_node_from_association(o);
}

RMF::NodeHandle get_node_from_association(RMF::FileHandle nh,
                                          Object *oi) {
  AssociationType o(oi);
  return nh.get_node_from_association(o);
}

bool get_has_associated_node(RMF::FileConstHandle nh, Object *oi) {
  return nh.get_node_from_association(oi) != RMF::NodeConstHandle();
}

IMPRMF_END_NAMESPACE
