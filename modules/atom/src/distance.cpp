/**
 *  \file distance.cpp  \brief distance measures
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/distance.h>
#include <IMP/atom/Mass.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/geometric_alignment.h>

IMPATOM_BEGIN_NAMESPACE

double get_rmsd_transforming_first(const IMP::algebra::Transformation3D& tr,
                                   const core::XYZs& s0, const core::XYZs& s1) {
  return algebra::get_rmsd_transforming_first(tr, s0, s1);
}

double get_rmsd(const core::XYZs& s0, const core::XYZs& s1) {
  return algebra::get_rmsd(s0, s1);
}

double get_rmsd(const Selection& s0, const Selection& s1) {
  return algebra::get_rmsd(s0.get_selected_particles(),
                           s1.get_selected_particles());
}

double get_rmsd_transforming_first(const IMP::algebra::Transformation3D& tr,
                                   const Selection& s0, const Selection& s1) {
  return algebra::get_rmsd_transforming_first(tr, s0.get_selected_particles(),
                                              s1.get_selected_particles());
}

std::pair<double, double> get_placement_score(const core::XYZs& from,
                                              const core::XYZs& to) {
  // calculate the best fit between the two placements
  algebra::Vector3Ds from_v, to_v;
  for (core::XYZs::const_iterator it = from.begin(); it != from.end(); ++it) {
    from_v.push_back(it->get_coordinates());
  }
  for (core::XYZs::const_iterator it = to.begin(); it != to.end(); ++it) {
    to_v.push_back(it->get_coordinates());
  }
  algebra::Transformation3D t =
      algebra::get_transformation_aligning_first_to_second(from_v, to_v);
  return std::pair<double, double>(
      t.get_translation().get_magnitude(),
      algebra::get_axis_and_angle(t.get_rotation()).second);
}

double get_pairwise_rmsd_score(const core::XYZs& ref1, const core::XYZs& ref2,
                               const core::XYZs& mdl1, const core::XYZs& mdl2) {
  // calculate the best fit between the reference and model
  // of the first component
  /*algebra::Vector3Ds from_v1,to_v1;
  for(core::XYZs::const_iterator it = mdl1.begin(); it != mdl1.end(); ++it) {
    from_v1.push_back(it->get_coordinates());
  }
  for(core::XYZs::const_iterator it = ref1.begin(); it != ref1.end(); ++it) {
    to_v1.push_back(it->get_coordinates());
    }*/
  algebra::Transformation3D t =
      algebra::get_transformation_aligning_first_to_second(mdl1, ref1);
  Float rmsd_score = get_rmsd_transforming_first(t, ref2, mdl2);
  return rmsd_score;
}

std::pair<double, double> get_component_placement_score(
    const core::XYZs& ref1, const core::XYZs& ref2, const core::XYZs& mdl1,
    const core::XYZs& mdl2) {
  // calculate the best fit between the reference and model
  // of the first component
  algebra::Vector3Ds from_v1, to_v1, from_v2, to_v2;
  for (core::XYZs::const_iterator it = mdl1.begin(); it != mdl1.end(); ++it) {
    from_v1.push_back(it->get_coordinates());
  }
  for (core::XYZs::const_iterator it = ref1.begin(); it != ref1.end(); ++it) {
    to_v1.push_back(it->get_coordinates());
  }
  for (core::XYZs::const_iterator it = mdl2.begin(); it != mdl2.end(); ++it) {
    from_v2.push_back(it->get_coordinates());
  }
  for (core::XYZs::const_iterator it = ref2.begin(); it != ref2.end(); ++it) {
    to_v2.push_back(it->get_coordinates());
  }
  algebra::Transformation3D t =
      algebra::get_transformation_aligning_first_to_second(from_v1, to_v1);
  // now transform the model of component 2 according to the transformation
  for (algebra::Vector3Ds::iterator it = from_v2.begin(); it != from_v2.end();
       ++it) {
    *it = t.get_transformed(*it);
  }
  // find the best transformation from the new from_v2 to the reference
  algebra::Transformation3D t2 =
      algebra::get_transformation_aligning_first_to_second(from_v2, to_v2);

  // return the best fit between
  return std::pair<double, double>(
      t.get_translation().get_magnitude(),
      algebra::get_axis_and_angle(t.get_rotation()).second);
}

namespace {
  //! returns mass if mass=true, or radii^3 if radii=true, or 1.0
  double get_weight
  (bool mass, bool radii, Particle* p) {
    if (mass) {
      return Mass(p).get_mass();
    } else if (radii) {
      return cube(core::XYZR(p).get_radius());
    } else {
      return 1.0;
    }
  }
}

double get_radius_of_gyration
(const ParticlesTemp& ps, bool weighted) {
  IMP_USAGE_CHECK(ps.size() > 0, "No particles provided");
  bool mass= Mass::get_is_setup(ps[0]) && weighted;
  bool radii= core::XYZR::get_is_setup(ps[0]) && weighted;
  algebra::Vector3D cm(0, 0, 0);
  double total= 0;
  for (unsigned int i= 0; i < ps.size(); ++i) {
    double weight= get_weight(mass, radii, ps[i]);
    total+= weight;
    cm+= core::XYZ(ps[i]).get_coordinates() * weight;
  }
  cm /= total;
  double ret = 0;
  for (unsigned int i = 0; i < ps.size(); ++i) {
    double c;
    if (radii) {
      c = .6 * square(core::XYZR(ps[i]).get_radius());
    } else {
      c = 0;
    }
    double d = get_squared_distance(core::XYZ(ps[i]).get_coordinates(), cm);
    ret += get_weight(mass, radii, ps[i]) * (d + c);
  }
  return std::sqrt(ret / total);
}

IMPATOM_END_NAMESPACE
