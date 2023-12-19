/**
 *  \file isd/ResidueProteinProximityRestraint.h
 *  \brief Restrain a selection of particles (eg. a residue or
 *  segment) to be within a certain distance of a second
 *  selection of particles (eg. a protein).
 *  Use to model data from mutagenesis experiments that disrupt
 *  protein-protein interactions.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/ResidueProteinProximityRestraint.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Scale.h>
#include <IMP/container/ListPairContainer.h>
#include <boost/math/special_functions/erf.hpp>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <cmath>

namespace {
  // sqrt(2*PI)
  static const double SQ2PI = 2.5066282746310002;
}

IMPISD_BEGIN_NAMESPACE

ResidueProteinProximityRestraint::ResidueProteinProximityRestraint(Model* m,
                                                                   double cutoff,
                                                                   double sigma,
                                                                   double xi,
                                                                   bool part_of_log_score,
                                                                   std::string name):
  Restraint(m,name),
  cutoff_(cutoff),
  sigma_(sigma),
  xi_(xi),
  part_of_log_score_(part_of_log_score)
  {
}

// add a contribution: general case
void ResidueProteinProximityRestraint::add_pairs_container(PairContainer *pc) {
  contribs_.push_back(pc);
  default_range_.push_back(static_cast<int>(default_range_.size()));
}

void ResidueProteinProximityRestraint::add_contribution_particles(const ParticleIndexes ppis1,
                                                                  const ParticleIndexes ppis2){

  ParticlesTemp pps1;
  ParticlesTemp pps2;
  for (unsigned int i = 0; i < ppis1.size(); i++){
    pps1.push_back(get_model()->get_particle(ppis1[i]));
    ppis_.push_back(ppis1[i]);
  }
  for (unsigned int i = 0; i < ppis2.size(); i++){
    pps2.push_back(get_model()->get_particle(ppis2[i]));
    ppis_.push_back(ppis2[i]);
  }

  IMP::core::XYZRs xx1 = IMP::core::XYZRs(pps1);
  IMP::core::XYZRs xx2 = IMP::core::XYZRs(pps2);

  algebra::Vector3D centroid1 = core::get_centroid(xx1);
  algebra::Vector3D centroid2 = core::get_centroid(xx2);

  coms1_.push_back(xx1);
  coms2_.push_back(xx2);
  //sigmass_.push_back(sigmas);
}

double ResidueProteinProximityRestraint::evaluate_for_contributions(Ints c)
  const{

  double score_tot = 0.0;
  double score = 0.0;
  //! Loop over the contributions and score things
  for (int n : c){

    //! Get COM
    algebra::Vector3D centroid1 = core::get_centroid(coms1_[n]);
    algebra::Vector3D centroid2 = core::get_centroid(coms2_[n]);
    double dist = IMP::algebra::get_distance(centroid1, centroid2);

    //! Compute distances. Loop elements of close pair
    Vector<double> dists_;
    IMP_CONTAINER_FOREACH(PairContainer, contribs_[n], {
	//! Distances
	core::XYZ d0(get_model(), _1[0]);
	core::XYZ d1(get_model(), _1[1]);
	double dist = get_distance(d0,d1);
	dists_.push_back(dist);
	
	//! Get particles indexes
	// ParticleIndex pi1 = get_model()->get_particle(_1[0])->get_index();
	// ParticleIndex pi2 = get_model()->get_particle(_1[1])->get_index();
      });

    if(!dists_.empty()) {
      double lowest_dist = *min_element(dists_.begin(), dists_.end());
      if (lowest_dist <= cutoff_/2.) {
        double prior_prob = std::exp(-xi_*lowest_dist);
        double prior_score = -log(prior_prob);
	score = prior_score;
      }
      else if (lowest_dist > cutoff_/2. && lowest_dist <= cutoff_) {
        double interpolation_prob = std::exp(-yi_*lowest_dist);
        score = -log(interpolation_prob) - interpolation_factor_;
      }
      else {
        double prior_prob = std::exp(-xi_*lowest_dist);
	double sig2 = sigma_*sigma_;
        double ld2 = lowest_dist*lowest_dist;
        double prob = std::exp(-ld2/(2*sig2))/(SQ2PI*sigma_);
        score = -log(prior_prob*prob);
      }
      score_tot +=  score;
    }
    else {
      double prior_prob = std::exp(-xi_*dist);
      double prior_score = -log(prior_prob);
      score_tot +=  max_score_ + prior_score;
    }
  }
  return score_tot;
}

double ResidueProteinProximityRestraint::unprotected_evaluate(DerivativeAccumulator *)
  const {

  double output_score = evaluate_for_contributions(default_range_);
  return output_score;
}

ModelObjectsTemp ResidueProteinProximityRestraint::do_get_inputs() const {
    ParticlesTemp ret;
    for (unsigned int k = 0; k < get_number_of_contributions(); ++k) {
      ret.push_back(get_model()->get_particle(ppis_[k]));
    }
    return ret;
}

IMPISD_END_NAMESPACE
