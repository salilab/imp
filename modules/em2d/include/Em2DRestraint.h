/**
 *  \file IMP/em2d/Em2DRestraint.h
  *  \brief A restraint to score the fitness of a model to a set of EM images
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_EM_2DRESTRAINT_H
#define IMPEM2D_EM_2DRESTRAINT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/ProjectionFinder.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/RegistrationResult.h"
#include <IMP/kernel/Restraint.h>
#include <IMP/base/log.h>
#include <IMP/SingletonContainer.h>

IMPEM2D_BEGIN_NAMESPACE

/**
 * This restraint ensures that a set of particles are similar to a set
 * of EM images. The restraint generates projections of the model that
 * are then compared with the EM images. The projections are generated from
 * the radius of its particles. In the case of atoms, the radius is generated
 * automatically. For other particles the radius has to be provided.
*/
class IMPEM2DEXPORT Em2DRestraint : public kernel::Restraint {
  //! SingletonContainer to store the particles that are restrained
  base::Pointer<SingletonContainer> particles_container_;
  // mutable because it has to change to get projections while evaluating
  //  mutable ProjectionFinder finder_;
  mutable base::Pointer<ProjectionFinder> finder_;

  //! Projection Masks to fast model projection
  em2d::Images em_images_;
  em2d::Images em_variances_;
  unsigned int number_of_optimized_projections_;
  bool fast_optimization_mode_;
  bool only_coarse_registration_;
  Em2DRestraintParameters params_;

 public:
  /**
   * Creates the restraint. You are not done yet by creating the restraint.
   * After creating it, you need to call the setup() function
   */
  Em2DRestraint(kernel::Model *m);

  /**
   * Initializes the restraint
   * @param score_function Scoring function to use to evaluate the similarity
   * between particles and projections
   * @param params All the parameters required for the restraint
   */
  void setup(ScoreFunction *score_function,
             const Em2DRestraintParameters &params) {
    params_ = params;
    finder_ = new ProjectionFinder;
    finder_->set_was_used(true);
    finder_->setup(score_function, params);
    fast_optimization_mode_ = false;
    only_coarse_registration_ = false;
  }

  /**
   * Sets the particles used to compute projections.
   * @param particles_container All the particles must be in the container
   */
  void set_particles(SingletonContainer *particles_container);

  /**
   * Sets the EM images to use as restraints
   * @param em_images The images
   */
  void set_images(const em2d::Images em_images);

  /*! Sets the variance images of EM images. This is useful if the
      image is a class average
      Note: Not all the scoring function use this
      data
  */
  /**
   * Sets the variance images for class averages. If class averages are used
   * as restraints, it is possible to use the variance images to assign a
   * standard deviation (error) for each of the pixels in a class average.
   * @param variance_images
   * @note Not all the scoring functions use the variance images
   * @note This function is functional as is tested, but has not been tested
   * in a real application yet.
   */
  void set_variance_images(const em2d::Images variance_images);

  /**
   * Sets fast mode for computing the restraint. This mode only makes sense
   * it the set_coarse_registration_mode option is false. This option only
   * optimizes some coarse results (those given by the argument)
   * to get the refined value.
   * This option is very fast compared to a full optimization, and almost
   * always is a good idea to use it with 1-5 results.
   * This mode is still significantly slow compared to
   * set_coarse_registration_mode(), but the values optimized are optimum.
   * @param n The number of images to optimize
   */
  void set_fast_mode(unsigned int n);

  /**
   * If the value provided to this function is true, restraint operates
   * only using a coarse registration scheme (maximizing the cross correlation
   * coefficient).
   * This option (which is set to false by default) can speed the evaluation
   * of the restraint by a order of magnitude, but the values obtained are
   * not going to be less accurate. You could want to activate it at the
   * beginning of an optimization.
   * @param opt true if you want to use the coarse mode
   */
  void set_coarse_registration_mode(bool opt) {
    if (opt) only_coarse_registration_ = true;
  }

  /**
   * Get the registration results for each of the images after finishing the
   * optimization. Obviously, requesting the results before optimizing is an
   * error
   * @return A list of registration results
   */
  RegistrationResults get_registration_results() const {
    return finder_->get_registration_results();
  }

  virtual double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(Em2DRestraint);
};

IMP_OBJECTS(Em2DRestraint, Em2DRestraints);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_EM_2DRESTRAINT_H */
