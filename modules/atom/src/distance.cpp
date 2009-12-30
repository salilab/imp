/**
 *  \file distance.cpp  \brief distance measures
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/distance.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/geometric_alignment.h>

IMPATOM_BEGIN_NAMESPACE

double rmsd(const core::XYZs& m1 ,const core::XYZs& m2) {
  IMP_USAGE_CHECK(m1.size()==m2.size(),
            "The input sets of XYZ points "
            <<"should be of the same size", ValueException);
  float rmsd=0.0;
  for(unsigned int i=0;i<m1.size();i++) {
    rmsd += algebra::squared_distance(m1[i].get_coordinates()
                                     ,m2[i].get_coordinates());
  }
  return std::sqrt(rmsd / m1.size());
}
IMPATOMEXPORT std::pair<double,double> placement_score(
  const core::XYZs& from ,const core::XYZs& to) {
  //calculate the best fit bewteen the two placements
  std::vector<algebra::Vector3D> from_v,to_v;
  for(core::XYZs::const_iterator it = from.begin(); it != from.end(); it++) {
    from_v.push_back(it->get_coordinates());
  }
  for(core::XYZs::const_iterator it = to.begin(); it != to.end(); it++) {
    to_v.push_back(it->get_coordinates());
  }
  algebra::Transformation3D t =
    algebra::rigid_align_first_to_second(from_v,to_v);
  return std::pair<double,double>(
    t.get_translation().get_magnitude(),
    algebra::decompose_rotation_into_axis_angle(t.get_rotation()).second);
}

std::pair<double,double> component_placement_score(
      const core::XYZs& ref1 ,const core::XYZs& ref2,
      const core::XYZs& mdl1 ,const core::XYZs& mdl2) {
  //calculate the best fit bewteen the reference and model
  //of the first component
  algebra::Vector3Ds from_v1,to_v1,from_v2,to_v2;
  for(core::XYZs::const_iterator it = mdl1.begin(); it != mdl1.end(); it++) {
    from_v1.push_back(it->get_coordinates());
  }
  for(core::XYZs::const_iterator it = ref1.begin(); it != ref1.end(); it++) {
    to_v1.push_back(it->get_coordinates());
  }
  for(core::XYZs::const_iterator it = mdl2.begin(); it != mdl2.end(); it++) {
    from_v2.push_back(it->get_coordinates());
  }
  for(core::XYZs::const_iterator it = ref2.begin(); it != ref2.end(); it++) {
    to_v2.push_back(it->get_coordinates());
  }
  algebra::Transformation3D t =
    algebra::rigid_align_first_to_second(from_v1,to_v1);
  //now transform the model of component 2 according to the transformation
  for(algebra::Vector3Ds::iterator it = from_v2.begin();
                                 it != from_v2.end(); it++) {
    *it=t.transform(*it);
  }
  //find the best transformation from the new from_v2 to the reference
  algebra::Transformation3D t2 =
    algebra::rigid_align_first_to_second(from_v2,to_v2);

  //transform the model of component 2 back
  algebra::Transformation3D t_back = t.get_inverse();
  for(algebra::Vector3Ds::iterator it = from_v2.begin();
                                 it != from_v2.end(); it++) {
    *it=t_back.transform(*it);
  }


  //return the best fit bewteen
  return std::pair<double,double>(
   t.get_translation().get_magnitude(),
   algebra::decompose_rotation_into_axis_angle(t.get_rotation()).second);
}


IMPATOM_END_NAMESPACE
