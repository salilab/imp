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

IMPCORE_BEGIN_NAMESPACE

class MonteCarlo;
class ListSingletonContainer;

//! A simple sampler.
/** This sampler randomizes the conformation and then uses Monte Carlo
    and Conjugate Gradient steps to search for good solutions.

    At the moment it only support optimization of cartesian coordinates,
    but this will be fixed.
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
  void randomize(const Parameters &pms, ListSingletonContainer *sc) const;
  ListSingletonContainer* set_up_movers(const Parameters &pms,
                                        MonteCarlo *mc) const;
public:
  MCCGSampler(Model *m);

  //! Set the bounding box for randomizing the cartesian coordinates
  void set_bounding_box(const algebra::BoundingBox3D &bb);

  //! Set the maximum number of attempts
  void set_number_of_attempts(unsigned int att);

  //! Set the number of MC steps to take
  void set_number_of_monte_carlo_steps(unsigned int cg);

  //! Set the maximum size of the MC step
  void set_max_monte_carlo_step_size(double d);

  //! Set the maximum size of the MC step for an attribute
  void set_max_monte_carlo_step_size(FloatKey k, double d);

  //! Set the number of CG steps to take after each MC step
  void set_number_of_conjugate_gradient_steps(unsigned int cg);

  IMP_SAMPLER(MCCGSampler, get_module_version_info())
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MCCG_SAMPLER_H */
