/**
 *  \file IMP/atom/BrownianDynamics.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_BROWNIAN_DYNAMICS_H
#define IMPATOM_BROWNIAN_DYNAMICS_H

#include <IMP/atom/atom_config.h>
#include "Diffusion.h"
#include "Simulator.h"
#include "atom_macros.h"
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/PairPredicate.h>
#include <IMP/kernel/SingletonPredicate.h>
#include <IMP/core/NeighborsTable.h>
#include <IMP/Optimizer.h>
#include <IMP/kernel/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPATOM_BEGIN_NAMESPACE

// for swig
class SimulationParameters;

//! Simple Brownian dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable "Stokes radius"; this optimizer assumes
    the score to be energy in kcal/mol, the xyz coordinates to be in
    angstroms and the diffusion coefficent be in cm^2/s

    kernel::Particles without optimized x,y,z and nonoptimized D are skipped.

    Rigid bodies are supported.

    BrownianDynamics uses a SimulationParameters particle to store the
    parameters of the simulation. Such a particle must be passed on
    creation. The BrownianDynamics object will at least see updates
    to the SimulationParamters particle which occur before the
    call to BrownianDynamics::optimize() or BrownianDynamics::simulate(),
    changing the the parameters during optimization has undefined
    results.

    The optimizer can either automatically determine which particles
    to use from the model or be passed a SingletonContainer for the
    particles. If such a container is passed, particles added to it
    during optimization state updates are handled properly.

    \see Diffusion
  */
class IMPATOMEXPORT BrownianDynamics : public Simulator {
 public:
  //! Create the optimizer
  /** If sc is not null, that container will be used to find particles
      to move, otherwise the model will be searched.
      @param m model associated with bd
      @param name name of bd object
      @param wave_factor for wave step function, see Simulator object,
                         if >1.001 or so, creates a wave of time steps
                         that are larger by up to wave_factor from
                         formal maximal time step

     @note wave_factor is an advanced feature - if you're not sure, just use
                       its default, see also Simulator::simulate_wave()
  */
  BrownianDynamics(kernel::Model *m, std::string name = "BrownianDynamics%1%",
                   double wave_factor=1.0);
 /** You can add predicates which will be applied to the neighbor list of
      each particle after it has been moved. If any of the predicates returns
      0, then the move will be rejected.
      \note This is in active development and will change. Most likely to a
      kernel::SingletonPredicate based mechanism which wraps up the neighbor
      computations internally.

      \note The neighbors list is updated _before_ any moves are made, for
      efficiency. So make sure that your distance bounds include potential
      moves. */
  void add_move_predicate(kernel::PairPredicate *pp, int reject_value,
                          double distance_bound,
                          double slack);
  /** See the other add_move_predicate */
  void add_move_predicate(kernel::SingletonPredicate *sp, int reject_value);
  void set_maximum_move(double ms) { max_step_ = ms; }
  void set_use_stochastic_runge_kutta(bool tf) { srk_ = tf; }

  IMP_OBJECT_METHODS(BrownianDynamics);

 private:
  virtual void setup(const kernel::ParticleIndexes &ps) IMP_OVERRIDE;
  virtual double do_step(const kernel::ParticleIndexes &sc,
                         double dt) IMP_OVERRIDE;
  virtual bool get_is_simulation_particle(kernel::ParticleIndex p) const
      IMP_OVERRIDE;

 private:
  void advance_chunk(double dtfs, double ikt, const kernel::ParticleIndexes &ps,
                     unsigned int begin, unsigned int end);
  void advance_coordinates_1(kernel::ParticleIndex pi, unsigned int i,
                             double dtfs, double ikT);
  void advance_coordinates_0(kernel::ParticleIndex pi, unsigned int i,
                             double dtfs, double ikT);
  void advance_orientation_0(kernel::ParticleIndex pi, double dtfs, double ikT);
  IMP_NAMED_TUPLE_4(Data, Datas, base::PointerMember<kernel::PairPredicate>,
                    predicate, int, value, double, distance, double, slack,);
  Datas rejection_predicates_;
  IMP_NAMED_TUPLE_2(SingletonData, SingletonDatas,
                   base::PointerMember<kernel::SingletonPredicate>, predicate,
                    int, value, );
  SingletonDatas singleton_rejection_predicates_;
  kernel::ScoreStates rejection_score_states_;
  base::Vector<base::PointerMember<core::NeighborsTable> > rejection_tables_;

  double max_step_;
  bool srk_;
  base::Vector<algebra::Vector3D> forces_;
};

/** Repeatedly run the current model with brownian dynamics at different time
    steps to try to find the maximum time step that can be used without
    the model exploding.
*/
IMPATOMEXPORT double get_maximum_time_step_estimate(BrownianDynamics *bd);

#ifndef IMP_DOXYGEN
IMPATOMEXPORT double get_harmonic_sigma(double D, double f);
#endif
IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BROWNIAN_DYNAMICS_H */
