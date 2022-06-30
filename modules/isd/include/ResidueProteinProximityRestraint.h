/**
 *  \file IMP/isd/ResidueProteinProximityRestraint.h
 *  \brief Restraint a selection of particles (eg. a residue or 
 *  segment) to be within  a certain distance of a second 
 *  selection of particles (eg. a protein). 
 *  Use to model data from mutagenesis experiments that disrupt
 *  protein-protein interactions.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_RESIDUE_BINDING_RESTRAINT_H
#define IMPISD_RESIDUE_BINDING_RESTRAINT_H
#include "isd_config.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container_macros.h>
#include <IMP/PairContainer.h>
#include <IMP/Restraint.h>
#include <boost/unordered_map.hpp>

IMPISD_BEGIN_NAMESPACE

//! Apply harmonic restraint between a residue or fragment and a
//! protein.

class IMPISDEXPORT ResidueProteinProximityRestraint : public Restraint {
public:
  //! Create the restraint.
 ResidueProteinProximityRestraint(IMP::Model *m,
                                  Float cutoff = 6.0,
                                  Float sigma = 3.0,
                                  Float xi = 0.5,
                                  bool part_of_log_score=false,
                                  std::string name = "ResidueProteinProximityRestraint_%1%"); 
 
  // add a contribution: general case
  void add_pairs_container(PairContainer *pc);

  // add particles list (for COM calculations)
  void add_contribution_particles(const ParticleIndexes ppis1,
                                  const ParticleIndexes ppis2);
 
  //! Evaluate the restraint just for a subset of contribution indexes
  double evaluate_for_contributions(Ints c) const;

  //! Get number of contributions added to the restraint
  unsigned get_number_of_contributions() const { return ppis_.size(); }

  void set_sigma(Float sigma) { sigma_=sigma; }
  
  void set_cutoff(Float cutoff) { cutoff_=cutoff; }

  void set_max_score(Float max_score) { max_score_=max_score; }
  void set_yi(Float yi) { yi_=yi; }
  void set_interpolation_factor(Float interpolation_factor) { interpolation_factor_=interpolation_factor; }
 
  void set_part_of_log_score(bool hey) { part_of_log_score_=hey; }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  
  IMP_OBJECT_METHODS(ResidueProteinProximityRestraint);
 private:
  Float cutoff_;
  Float sigma_;
  Float xi_;
  Float yi_;
  Float interpolation_factor_;
  Float max_score_;
  bool part_of_log_score_;
  PairContainers contribs_;
  ParticleIndexes ppis_;
  Ints default_range_;
  std::vector<IMP::core::XYZRs> coms1_;
  std::vector<IMP::core::XYZRs> coms2_;
  
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_RESIDUE_PROTEIN_PROXIMITY_RESTRAINT_H */

