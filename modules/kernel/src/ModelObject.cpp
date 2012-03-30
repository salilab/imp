/**
 *  \file ModelObject.cpp  \brief Single variable function.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/ModelObject.h>
#include <IMP/Model.h>

IMP_BEGIN_NAMESPACE
void ModelObject::update_dependencies(const DependencyGraph &dg,
                                      const DependencyGraphVertexIndex &index) {
  return do_update_dependencies(dg, index);
}
ModelObject::ModelObject(Model *m, std::string name):
  Tracked(this, m, name) {
}

ModelObject::ModelObject(std::string name): Tracked(name) {
}

void ModelObject::set_model(Model *m) {
  Tracked::set_tracker(this, m);
}
ModelObjectsTemp ModelObject::get_inputs() const {
  return do_get_inputs();
}
ModelObjectsTemp ModelObject::get_outputs() const {
  return do_get_outputs();
}

IMP_END_NAMESPACE
