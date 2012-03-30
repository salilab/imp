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
#include <IMP/base/doxygen_macros.h>
#include "ModelObject.h"


/** Declare:
    - IMP::ModelObject::do_update_dependencies()
    - IMP::ModelObject::do_get_inputs()
    - IMP::ModelObject::do_get_outputs()
*/
#define IMP_MODEL_OBJECT(Name)                                          \
  public:                                                               \
  IMP_IMPLEMENT(void do_update_dependencies(const IMP::DependencyGraph &, \
                               const IMP::DependencyGraphVertexIndex &)); \
  IMP_IMPLEMENT(ModelObjectsTemp do_get_inputs() const);                \
  IMP_IMPLEMENT(ModelObjectsTemp do_get_outputs() const)

#endif  /* IMPKERNEL_MODEL_OBJECT_MACROS_H */
