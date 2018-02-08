/**
 *  \file IMP/core/DirectionMover.h
 *  \brief A mover that transforms a Direction.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_DIRECTION_MOVER_H
#define IMPCORE_DIRECTION_MOVER_H

#include <IMP/core/core_config.h>
#include <IMP/core/direction.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/Particle.h>
#include <IMP/Object.h>
#include <IMP/Model.h>


IMPCORE_BEGIN_NAMESPACE

//! Modify a direction.
/** The Direction is transformed via rotation and/or reflection

    \see Direction
    \see MonteCarlo
*/
class IMPCOREEXPORT DirectionMover : public MonteCarloMover {
  private:
    algebra::Vector3D last_direction_;
    Float max_angle_;
    Float reflect_prob_;
    ParticleIndex pi_;

    void initialize(ParticleIndex pi, double max_rotation,
                    double reflect_probability);

  public:
    DirectionMover(Model *m, ParticleIndex pi, Float max_rotation,
                   Float reflect_probability);

    DirectionMover(Direction d, Float max_rotation,
                   Float reflect_probability);
  
    //! Set the maximum rotation in radians.
    void set_maximum_rotation(Float mr);
  
    //! Set the frequency of reflection moves.
    void set_reflect_probability(Float rp);
      
    //! Get the maximum rotation.
    Float get_maximum_rotation() const { return max_angle_; }
   
    //! Get the frequency of reflection moves.
    Float get_reflect_probability() const { return reflect_prob_; }
  
    //! Get the surface being moved.
    Direction get_direction() const { return Direction(get_model(), pi_); }

  protected:
    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
    virtual void do_reject() IMP_OVERRIDE;
    IMP_OBJECT_METHODS(DirectionMover);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DIRECTION_MOVER_H */
