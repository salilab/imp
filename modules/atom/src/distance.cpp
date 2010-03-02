/**
 *  \file distance.cpp  \brief distance measures
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/distance.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/geometric_alignment.h>

IMPATOM_BEGIN_NAMESPACE

IMPATOMEXPORT std::pair<double,double> get_placement_score(
  const core::XYZs& from ,const core::XYZs& to) {
  //calculate the best fit bewteen the two placements
  std::vector<algebra::VectorD<3> > from_v,to_v;
  for(core::XYZs::const_iterator it = from.begin(); it != from.end(); it++) {
    from_v.push_back(it->get_coordinates());
  }
  for(core::XYZs::const_iterator it = to.begin(); it != to.end(); it++) {
    to_v.push_back(it->get_coordinates());
  }
  algebra::Transformation3D t =
    algebra::get_transformation_aligning_first_to_second(from_v,to_v);
  return std::pair<double,double>(
    t.get_translation().get_magnitude(),
    algebra::get_angle_and_axis(t.get_rotation()).second);
}

double get_pairwise_rmsd_score(
      const core::XYZs& ref1 ,const core::XYZs& ref2,
      const core::XYZs& mdl1 ,const core::XYZs& mdl2) {
  //calculate the best fit bewteen the reference and model
  //of the first component
  /*std::vector<algebra::VectorD<3> > from_v1,to_v1;
  for(core::XYZs::const_iterator it = mdl1.begin(); it != mdl1.end(); it++) {
    from_v1.push_back(it->get_coordinates());
  }
  for(core::XYZs::const_iterator it = ref1.begin(); it != ref1.end(); it++) {
    to_v1.push_back(it->get_coordinates());
    }*/
  algebra::Transformation3D t =
    algebra::get_transformation_aligning_first_to_second(mdl1,ref1);
  Float rmsd_score=get_rmsd(ref2,mdl2, t);
  return rmsd_score;
}

std::pair<double,double> get_component_placement_score(
      const core::XYZs& ref1 ,const core::XYZs& ref2,
      const core::XYZs& mdl1 ,const core::XYZs& mdl2) {
  //calculate the best fit bewteen the reference and model
  //of the first component
  std::vector<algebra::VectorD<3> > from_v1,to_v1,from_v2,to_v2;
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
    algebra::get_transformation_aligning_first_to_second(from_v1,to_v1);
  //now transform the model of component 2 according to the transformation
  for(std::vector<algebra::VectorD<3> >::iterator it = from_v2.begin();
                                 it != from_v2.end(); it++) {
    *it=t.get_transformed(*it);
  }
  //find the best transformation from the new from_v2 to the reference
  algebra::Transformation3D t2 =
    algebra::get_transformation_aligning_first_to_second(from_v2,to_v2);

  //return the best fit bewteen
  return std::pair<double,double>(
   t.get_translation().get_magnitude(),
   algebra::get_angle_and_axis(t.get_rotation()).second);
}


IMPATOM_END_NAMESPACE
