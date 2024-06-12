/**
 *  \file IMP/isd/ResidueProteinProximityRestraint.h
 *  \brief Restrain a selection of particles (eg. a residue or
 *  segment) to be within a certain distance of a second
 *  selection of particles (eg. a protein).
 *  Use to model data from mutagenesis experiments that disrupt
 *  protein-protein interactions.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_RESIDUE_PROTEIN_PROXIMITY_RESTRAINT_H
#define IMPISD_RESIDUE_PROTEIN_PROXIMITY_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container_macros.h>
#include <IMP/PairContainer.h>
#include <IMP/Restraint.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/utility.h>
#include <boost/unordered_map.hpp>

IMPISD_BEGIN_NAMESPACE

//! Apply harmonic restraint between a residue or fragment and a
//! protein.
class IMPISDEXPORT ResidueProteinProximityRestraint : public Restraint {
public:
  //! Create the restraint.
 ResidueProteinProximityRestraint(Model *m,
                                  double cutoff = 6.0,
                                  double sigma = 3.0,
                                  double xi = 0.5,
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

  void set_sigma(double sigma) { sigma_=sigma; }

  void set_cutoff(double cutoff) { cutoff_=cutoff; }

  void set_max_score(double max_score) { max_score_=max_score; }

  void set_yi(double yi) { yi_=yi; }

  void set_interpolation_factor(double interpolation_factor) { interpolation_factor_=interpolation_factor; }

  void set_part_of_log_score(bool hey) { part_of_log_score_=hey; }

  virtual double unprotected_evaluate(DerivativeAccumulator *accum) const override;

  virtual IMP::ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(ResidueProteinProximityRestraint);
 private:
  double cutoff_;
  double sigma_;
  double xi_;
  double yi_;
  double interpolation_factor_;
  double max_score_;
  bool part_of_log_score_;
  PairContainers contribs_;
  ParticleIndexes ppis_;
  Ints default_range_;
  std::vector<IMP::core::XYZRs> coms1_;
  std::vector<IMP::core::XYZRs> coms2_;
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_RESIDUE_PROTEIN_PROXIMITY_RESTRAINT_H */
