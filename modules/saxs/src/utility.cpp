/**
 *  \file IMP/saxs/utility.cpp
 *  \brief Functions to deal with very common saxs operations
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#include <IMP/saxs/utility.h>
#include <IMP/saxs/SolventAccessibleSurface.h>

IMPSAXS_BEGIN_NAMESPACE

Profile* compute_profile(IMP::kernel::Particles particles, float min_q,
                         float max_q, float delta_q, FormFactorTable* ft,
                         FormFactorType ff_type, float water_layer_c2, bool fit,
                         bool reciprocal, bool ab_initio, bool vacuum, std::string beam_profile_file) {
  IMP_NEW(Profile, profile, (min_q, max_q, delta_q));
  if (reciprocal) profile->set_ff_table(ft);
  if (beam_profile_file.size() > 0) profile->set_beam_profile(beam_profile_file);

  // compute surface accessibility and average radius
  IMP::Floats surface_area;
  SolventAccessibleSurface s;
  float average_radius = 0.0;
  if (water_layer_c2 != 0.0) {
    // add radius
    for (unsigned int i = 0; i < particles.size(); i++) {
      float radius = ft->get_radius(particles[i], ff_type);
      IMP::core::XYZR::setup_particle(particles[i], radius);
      average_radius += radius;
    }
    surface_area = s.get_solvent_accessibility(IMP::core::XYZRs(particles));
    average_radius /= particles.size();
    profile->set_average_radius(average_radius);
  }

  // pick profile calculation based on input parameters
  if (!fit) {         // regular profile, no c1/c2 fitting
    if (ab_initio) {  // bead model, constant form factor
      profile->calculate_profile_constant_form_factor(particles);
    } else if (vacuum) {
      profile->calculate_profile_partial(particles, surface_area, ff_type);
      profile->sum_partial_profiles(0.0, 0.0);  // c1 = 0;
    } else {
      profile->calculate_profile(particles, ff_type, reciprocal);
    }
  } else {  // c1/c2 fitting
    if (reciprocal)
      profile->calculate_profile_reciprocal_partial(particles, surface_area,
                                                    ff_type);
    else
      profile->calculate_profile_partial(particles, surface_area, ff_type);
  }
  return profile.release();
}

IMPSAXS_END_NAMESPACE
