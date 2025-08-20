/**
 *  \file IMP/isd/PMFRestraint.h
 *  \brief Simple restraint based on a Potential of Mean Force lookup table
 *
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_PMF_RESTRAINT_H
#define IMPISD_PMF_RESTRAINT_H

#include <IMP/Particle.h>
#include <IMP/Restraint.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/isd/isd_config.h>

#include <string>
#include <utility>
#include <vector>

IMPISD_BEGIN_NAMESPACE

class IMPISDEXPORT PMFRestraint : public Restraint {
 public:
  //! Constructor with lookup table file and optional distance cutoff
  PMFRestraint(Model* m, std::string pmf_file, double distance_cutoff = 0.0,
               std::string name = "PMFRestraint");

  //! Get PMF value for a specific distance
  double get_pmf_value(double distance) const;

  //! Set containers for dynamic pair management (called from Python)
  void set_containers(IMP::container::ListSingletonContainer* lsc,
                      IMP::container::ClosePairContainer* cpc);

  virtual double unprotected_evaluate(
      DerivativeAccumulator* accum) const override;
  virtual ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(PMFRestraint);

 private:
  void load_pmf_from_file(std::string filename);

  std::vector<std::pair<double, double>> pmf_table_;  // PMF lookup table
  IMP::Pointer<IMP::container::ListSingletonContainer>
      lsc_;                                               // Particle container
  IMP::Pointer<IMP::container::ClosePairContainer> cpc_;  // Pair container
  double min_distance_, max_distance_;                    // PMF table range
  double distance_cutoff_;  // Distance cutoff for pairs
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_PMF_RESTRAINT_H */
