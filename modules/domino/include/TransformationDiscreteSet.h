/**
 * \file  TransformationDiscreteSet.h
 * \brief Holds a discrete sampling space of transformations.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#ifndef IMPDOMINO_TRANSFORMATION_DISCRETE_SET_H
#define IMPDOMINO_TRANSFORMATION_DISCRETE_SET_H

#include "IMP/Particle.h"
#include <map>
#include  <sstream>
#include "IMP/base_types.h"
#include "config.h"
#include <IMP/algebra/Transformation3D.h>
#include "DiscreteSet.h"

IMPDOMINO_BEGIN_NAMESPACE

class IMPDOMINOEXPORT TransformationDiscreteSet : public DiscreteSet
{
public:
  TransformationDiscreteSet();//Model *m);
  void add_transformation(const algebra::Transformation3D &t);
  algebra::Transformation3D get_transformation(long state_ind) const;

  void show(std::ostream& out=std::cout) const;
  void set_model(Model *m) {m_=m;}
protected:
  Model *m_;
  Particles states_;
  std::vector<FloatKey> atts_;
  std::vector<algebra::Transformation3D> trans_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_TRANSFORMATION_DISCRETE_SET_H */
