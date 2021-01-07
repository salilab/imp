/**
 *  \file IMP/core/SurfaceMover.h
 *  \brief A mover that transforms a Surface.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SURFACE_MOVER_H
#define IMPCORE_SURFACE_MOVER_H

#include <IMP/core/core_config.h>
#include <IMP/core/Surface.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/Particle.h>
#include <IMP/Object.h>
#include <IMP/Model.h>


IMPCORE_BEGIN_NAMESPACE

//! Modify a surface orientation.
/** The Surface is transformed via translation, rotation, and/or
    reflection.

    If combined with a `LateralSurfaceConstraint`, then only
    the normal component of the translation is kept.

    \see Surface
    \see LateralSurfaceConstraint
    \see MonteCarlo
*/
class IMPCOREEXPORT SurfaceMover : public MonteCarloMover {
  private:
    algebra::Transformation3D last_transform_;
    Float max_translation_;
    Float max_angle_;
    Float reflect_prob_;
    ParticleIndex pi_;

    void initialize(ParticleIndex pi, double max_translation,
                    double max_rotation, double reflect_probability);

  public:
    SurfaceMover(Model *m, ParticleIndex pi, Float max_translation,
                 Float max_rotation, Float reflect_probability);

    SurfaceMover(Surface s, Float max_translation, Float max_rotation,
                 Float reflect_probability);

    //! Set the maximum translation in angstroms.
    void set_maximum_translation(Float mt);
  
    //! Set the maximum rotation in radians.
    void set_maximum_rotation(Float mr);
  
    //! Set the frequency of reflection moves.
    void set_reflect_probability(Float rp);
   
    //! Get the maximum translation.
    Float get_maximum_translation() const { return max_translation_; }
   
    //! Get the maximum rotation.
    Float get_maximum_rotation() const { return max_angle_; }
   
    //! Get the frequency of reflection moves.
    Float get_reflect_probability() const { return reflect_prob_; }
  
    //! Get the surface being moved.
    Surface get_surface() const { return Surface(get_model(), pi_); }

  protected:
    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
    virtual void do_reject() IMP_OVERRIDE;
    IMP_OBJECT_METHODS(SurfaceMover);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_SURFACE_MOVER_H */
