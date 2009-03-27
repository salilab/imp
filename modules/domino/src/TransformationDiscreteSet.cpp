/**
 * \file  TransformationDiscreteSet.cpp
 * \brief Holds a discrete sampling space of transformations.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/domino/TransformationDiscreteSet.h>
IMPDOMINO_BEGIN_NAMESPACE
TransformationDiscreteSet::TransformationDiscreteSet(){
  atts_.push_back(FloatKey("x"));
  atts_.push_back(FloatKey("y"));
  atts_.push_back(FloatKey("z"));
  atts_.push_back(FloatKey("a"));
  atts_.push_back(FloatKey("b"));
  atts_.push_back(FloatKey("c"));
  atts_.push_back(FloatKey("d"));
  IMP_LOG(VERBOSE,"Cosntruct a TransformationDiscreteSet with " <<
          atts_.size() << " attributes " << std::endl);
  m_=NULL;
}


algebra::Transformation3D
  TransformationDiscreteSet::get_transformation(long state_ind) const {
  IMP_assert(static_cast<unsigned int>(state_ind)<trans_.size(),
             "the input index is out of range");
  return trans_[state_ind];
}
void TransformationDiscreteSet::add_transformation(
         const algebra::Transformation3D &t) {
  IMP_assert(m_!=NULL,"need to set the model first");
  trans_.push_back(t);
  Particle *p = new Particle(m_);
  for(int i=0;i<3;i++){
    p->add_attribute(atts_[i],t.get_translation()[i],false);
  }
  for(int i=0;i<4;i++){
    p->add_attribute(atts_[i+3],t.get_rotation().get_quaternion()[i],false);
  }
  add_state(p);
}
void TransformationDiscreteSet::show(std::ostream& out) const {
  //TODO - add
}
IMPDOMINO_END_NAMESPACE
