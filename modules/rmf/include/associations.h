/**
 *  \file IMP/rmf/associations.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ASSOCIATIONS_H
#define IMPRMF_ASSOCIATIONS_H

#include <IMP/rmf/rmf_config.h>
#include "links.h"
#include <IMP/base/Object.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/object_macros.h>
#include <IMP/base/log_macros.h>
#include <RMF/names.h>

IMPRMF_BEGIN_NAMESPACE

typedef base::Object *AssociationType;

IMPRMFEXPORT void set_association(RMF::NodeConstHandle nh, base::Object *o,
                                  bool overwrite = false);

template <class T>
inline T *get_association(RMF::NodeConstHandle nh) {
  AssociationType o = nh.get_association<AssociationType>();
  base::Object *op = o;
  if (!op) return nullptr;
  return dynamic_cast<T *>(op);
}

/** \name Associations
    RMF supports tracking associations between an RMF file an native objects.
    In the \imp case these are IMP::base::Object and we provide some special
    methods aid in this process.
    @{
*/
IMPRMFEXPORT RMF::NodeConstHandle get_node_from_association(
    RMF::FileConstHandle nh, base::Object *oi);

IMPRMFEXPORT RMF::NodeHandle get_node_from_association(RMF::FileHandle nh,
                                                       base::Object *oi);
IMPRMFEXPORT bool get_has_associated_node(RMF::FileConstHandle nh,
                                          base::Object *oi);

/** @} */

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ASSOCIATIONS_H */
