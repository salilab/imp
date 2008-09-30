/**
 *  \file Vector3D.cpp   \brief Simple 3D vector class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Vector3D.h"
#include "IMP/random.h"
#include "IMP/internal/constants.h"
#include "IMP/utility.h"

#include <boost/random/uniform_real.hpp>

IMP_BEGIN_NAMESPACE

Vector3D random_vector_in_box(const Vector3D &min, const Vector3D &max)
{
  Vector3D ret;
  for (unsigned int i=0; i< 3; ++i) {
    IMP_check(min[i] < max[i], "Box for randomize must be non-empty",
              ValueException);
    ::boost::uniform_real<> rand(min[i], max[i]);
    ret[i]=rand(random_number_generator);
  }
  return ret;
}


Vector3D random_vector_in_sphere(const Vector3D &center, Float radius)
{
  IMP_check(radius > 0, "Radius in randomize must be postive",
            ValueException);
  Vector3D min(center[0]-radius, center[1]-radius, center[2]-radius);
  Vector3D max(center[0]+radius, center[1]+radius, center[2]+radius);
  float norm;
  Vector3D ret;
  // \todo This algorithm could be more efficient.
  do {
    ret=random_vector_in_box(min, max);
    norm= (center- ret).get_magnitude();
  } while (norm > radius);
  return ret;
}

Vector3D random_vector_on_sphere(const Vector3D &center, Float radius)
{
  IMP_check(radius > 0, "Radius in randomize must be postive",
            ValueException);
  ::boost::uniform_real<> rand(-1,1);
  Vector3D up;
  up[2]= rand(random_number_generator);
  ::boost::uniform_real<> trand(0, 2*internal::PI);
  Float theta= trand(random_number_generator);
  // radius of circle
  Float r= std::sqrt(1-square(up[2]));
  up[0]= std::sin(theta)*r;
  up[1]= std::cos(theta)*r;
  IMP_assert(std::abs(up.get_magnitude() -1) < .1,
             "Error generating unit vector on sphere");
  IMP_LOG(VERBOSE, "Random vector on sphere is " << up << std::endl);
  return center+ up*radius;
}

IMP_END_NAMESPACE
