/**
 *  \file IMP/isd/HybridMonteCarlo.h
 *  \brief A hybrid monte carlo implementation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_HYBRID_MONTE_CARLO_H
#define IMPISD_HYBRID_MONTE_CARLO_H

#include <IMP/isd/isd_config.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/isd/MolecularDynamics.h>
#include <IMP/isd/MolecularDynamicsMover.h>

IMPISD_BEGIN_NAMESPACE

//! Hybrid Monte Carlo optimizer

class IMPISDEXPORT HybridMonteCarlo : public core::MonteCarlo
{

public:
  HybridMonteCarlo(Model *m, Float kT=1.0, unsigned steps=100,
          Float timestep=1.0, unsigned persistence=1);

  Float get_kinetic_energy() const;

  Float get_potential_energy() const;

  Float get_total_energy() const;

  //set md timestep
  void set_timestep(Float ts);
  double get_timestep() const;

  //set number of md steps per mc step
  void set_number_of_md_steps(unsigned nsteps);
  unsigned get_number_of_md_steps() const;

  //set how many mc steps happen until you redraw the momenta
  void set_persistence(unsigned persistence=1);
  unsigned get_persistence() const;

  //return pointer to isd::MolecularDynamics instance
  //useful if you want to set other stuff that is not exposed here
  MolecularDynamics* get_md() const;

  //evaluate should return the total energy
  double do_evaluate(const ParticleIndexes &) const
{
    double ekin = md_->get_kinetic_energy();
    double epot;
    if (get_maximum_difference() < std::numeric_limits<double>::max()) {
        epot = get_scoring_function()->evaluate_if_below(false,
                get_last_accepted_energy()+get_maximum_difference());
    } else {
        epot = get_scoring_function()->evaluate(false);
    }
    return ekin + epot;
}

  virtual void do_step();
  IMP_OBJECT_METHODS(HybridMonteCarlo);
private:
  unsigned num_md_steps_,persistence_;
  unsigned persistence_counter_;
  IMP::internal::OwnerPointer<MolecularDynamicsMover> mv_;
  IMP::Pointer<MolecularDynamics> md_;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_HYBRID_MONTE_CARLO_H */
