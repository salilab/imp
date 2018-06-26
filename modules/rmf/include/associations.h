/**
 *  \file IMP/rmf/associations.h
 *  \brief Track associations between an RMF file and native objects.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ASSOCIATIONS_H
#define IMPRMF_ASSOCIATIONS_H

#include <IMP/rmf/rmf_config.h>
#include "links.h"
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/object_macros.h>
#include <IMP/log_macros.h>
#include <RMF/names.h>

IMPRMF_BEGIN_NAMESPACE

typedef Object *AssociationType;

IMPRMFEXPORT void set_association(RMF::NodeConstHandle nh, Object *o,
                                  bool overwrite = false);

template <class T>
inline T *get_association(RMF::NodeConstHandle nh) {
  AssociationType o = nh.get_association<AssociationType>();
  Object *op = o;
  if (!op) return nullptr;
  return dynamic_cast<T *>(op);
}

/** \name Associations
    RMF supports tracking associations between an RMF file and native objects.
    In the \imp case these are IMP::Object and we provide some special
    methods to aid in this process.
    @{
*/
IMPRMFEXPORT RMF::NodeConstHandle get_node_from_association(
    RMF::FileConstHandle nh, Object *oi);

IMPRMFEXPORT RMF::NodeHandle get_node_from_association(RMF::FileHandle nh,
                                                       Object *oi);
IMPRMFEXPORT bool get_has_associated_node(RMF::FileConstHandle nh,
                                          Object *oi);

/** @} */

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ASSOCIATIONS_H */
