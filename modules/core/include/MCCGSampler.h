/**
 *  \file core/MCCGSampler.h
 *  \brief A Monte Carlo/Conjugate Gradients based sampler.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_MCCG_SAMPLER_H
#define IMPCORE_MCCG_SAMPLER_H

#include "config.h"

#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/algebra/BoundingBoxD.h>
#include "internal/CoreListSingletonContainer.h"

IMPCORE_BEGIN_NAMESPACE

class MonteCarlo;

//! A simple sampler.
/** This sampler randomizes the conformation and then uses Monte Carlo
    and conjugate gradient steps to search for good solutions. Each
    Monte Carlo move is followed by the specified number of
    conjugate gradient steps before it is decided whether to accept
    or reject the move.

    At the moment it only support optimization of Cartesian coordinates,
    but this will be fixed when people ask for it
    (and they already have :-). We are also open to supporting a wider
    variety of optimization protocols (eg only do conjugate gradient
    steps occasionally).
*/
class IMPCOREEXPORT MCCGSampler : public Sampler
{
  struct Parameters {
    unsigned int cg_steps_;
    int mc_steps_;
    typedef std::map<FloatKey, double> BallSizes;
    BallSizes ball_sizes_;
    typedef std::map<FloatKey, std::pair<Float, Float> > Bounds;
    Bounds bounds_;
    unsigned int attempts_;
    FloatKeys opt_keys_;
    Parameters();
  };
  Parameters default_parameters_;

  Parameters fill_in_parameters() const;
  void randomize(const Parameters &pms,
                 internal::CoreListSingletonContainer *sc) const;
  internal::CoreListSingletonContainer* set_up_movers(const Parameters &pms,
                                        MonteCarlo *mc) const;
public:
  MCCGSampler(Model *m);

  //! Set the bounding box for randomizing the Cartesian coordinates
  void set_bounding_box(const algebra::BoundingBoxD<3> &bb);

  //! Set the maximum number of attempts to find a solution
  void set_number_of_attempts(unsigned int att);

  //! Set the number of MC steps to take in each optimization run
  void set_number_of_monte_carlo_steps(unsigned int cg);

  //! Set the maximum size of the MC step for all attributes
  void set_max_monte_carlo_step_size(double d);

  //! Set the maximum size of the MC step for an attribute
  /** As was mentioned, at the moment k can be one of
      x,y or z.
  */
  void set_max_monte_carlo_step_size(FloatKey k, double d);

  //! Set the number of CG steps to take after each MC step
  void set_number_of_conjugate_gradient_steps(unsigned int cg);

  IMP_SAMPLER(MCCGSampler)
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MCCG_SAMPLER_H */
