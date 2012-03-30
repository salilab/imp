/**
 *  \file IMP/model_object_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_MODEL_OBJECT_MACROS_H
#define IMPKERNEL_MODEL_OBJECT_MACROS_H
#include "kernel_config.h"
#include "dependency_graph.h"
#include "ModelObject.h"


/** Declare:
    - IMP::ModelObject::do_update_dependencies()
*/
#define IMP_MODEL_OBJECT_DEPENDENCIES(Name)                             \
  public:                                                               \
  IMP_IMPLEMENT(void do_update_dependencies(const DependencyGraph &,    \
                                  const DependencyGraphVertexIndex &)); \
//! @}


#endif  /* IMPKERNEL_MODEL_OBJECT_MACROS_H */
