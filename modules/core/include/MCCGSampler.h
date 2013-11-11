/**
 *  \file IMP/core/MCCGSampler.h
 *  \brief A Monte Carlo/Conjugate Gradients based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MCCG_SAMPLER_H
#define IMPCORE_MCCG_SAMPLER_H

#include <IMP/core/core_config.h>

#include <IMP/Sampler.h>
#include <IMP/container_macros.h>
#include <IMP/Optimizer.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/kernel/internal/InternalListSingletonContainer.h>

IMPCORE_BEGIN_NAMESPACE

class MonteCarlo;

//! A simple sampler.
/** This sampler randomizes the conformation and then uses Monte Carlo
    and conjugate gradient steps to search for good solutions. Each
    Monte Carlo move is followed by the specified number of
    conjugate gradient steps before it is decided whether to accept
    or reject the move. When the sampler encounters a solution that
    passes all of the restraint score cutoffs, it will accept the solution
    and move on to generating another one.

    At the moment it only support optimization of Cartesian coordinates,
    but this will be fixed when people ask for it
    (and they already have :-). We are also open to supporting a wider
    variety of optimization protocols (eg only do conjugate gradient
    steps occasionally).
*/
class IMPCOREEXPORT MCCGSampler : public Sampler {
  struct Parameters {
    unsigned int cg_steps_;
    int mc_steps_;
    typedef std::map<FloatKey, double> BallSizes;
    BallSizes ball_sizes_;
    typedef std::map<FloatKey, std::pair<Float, Float> > Bounds;
    Bounds bounds_;
    unsigned int attempts_;
    FloatKeys opt_keys_;
    IMP::base::PointerMember<Optimizer> local_opt_;
    Parameters();
  };
  Parameters default_parameters_;
  bool is_refining_;
  base::Pointer<ConfigurationSet> rejected_;

  Parameters fill_in_parameters() const;
  void randomize(const Parameters &pms,
                 IMP::internal::InternalListSingletonContainer *sc) const;
  IMP::internal::InternalListSingletonContainer *set_up_movers(
      const Parameters &pms, MonteCarlo *mc) const;

 public:
  MCCGSampler(kernel::Model *m, std::string name = "MCCG Sampler %1%");

  //! Set the bounding box for randomizing the Cartesian coordinates
  void set_bounding_box(const algebra::BoundingBoxD<3> &bb);

  /** \name Number of steps
      A sampling run proceeds as 3 nested loops
      -# number of attempts
      -# number of Monte Carlo steps
      -# number of CG steps
      @{
  */
  //! Set the maximum number of attempts to find a solution
  void set_number_of_attempts(unsigned int att);

  //! Set the number of MC steps to take in each optimization run
  void set_number_of_monte_carlo_steps(unsigned int cg);

  //! Set the number of CG steps to take after each MC step
  void set_number_of_conjugate_gradient_steps(unsigned int cg);
  /** @} */

  //! Set the maximum size of the MC step for all attributes
  void set_max_monte_carlo_step_size(double d);

  //! Set the maximum size of the MC step for an attribute
  /** As was mentioned, at the moment k can be one of
      x,y or z.
  */
  void set_max_monte_carlo_step_size(FloatKey k, double d);

  //! if set to true, then do not randomize the configuration before
  //! sampling.
  void set_is_refining(bool tf);

  //! Set a local optimizer to use instead of ConjugateGradients
  void set_local_optimizer(Optimizer *opt);

  //! Whether or not to save rejected conformations
  /**
     Saving these can be useful if the sampling is not finding
     any good conformations.
   */
  void set_save_rejected_configurations(bool tf);

  ConfigurationSet *get_rejected_configurations() const;

  /** \name Optimizer states
      The optimizer states will be added to the MonteCarlo optimizer
      used.
  */
  IMP_LIST(public, OptimizerState, optimizer_state, OptimizerState *,
           OptimizerStates);

  virtual ConfigurationSet *do_sample() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MCCGSampler);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MCCG_SAMPLER_H */
