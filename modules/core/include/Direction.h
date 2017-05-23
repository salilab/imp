/**
 *  \file IMP/core/Direction.h
 *  \brief Simple direction decorator.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_DIRECTION_H
#define IMPCORE_DIRECTION_H

#include <IMP/core/core_config.h>
#include <IMP/decorator_macros.h>
#include <IMP/Decorator.h>
#include <IMP/Constraint.h>
#include <IMP/algebra/VectorD.h>

IMPCORE_BEGIN_NAMESPACE

//! A decorator for a particle that represents a direction in 3D.
/** \ingroup decorators
*/
class IMPCOREEXPORT Direction : public Decorator {
  public:
    static void do_setup_particle(Model *m, ParticleIndex pi,
                                  const algebra::Vector3D &v);

    IMP_DECORATOR_METHODS(Direction, Decorator);
    IMP_DECORATOR_SETUP_1(Direction, algebra::Vector3D, v);

    //! Check if particle is setup as a direction.
    static bool get_is_setup(Model *m, ParticleIndex pi);
  
    //! Get key for direction attribute.
    static FloatKey get_direction_key(unsigned int i);
  
    //! Get the vector of derivatives added to the direction.
    Float get_direction_derivative(int i) const {
      return get_particle()->get_derivative(get_direction_key(i));
    }
  
    //! Get the vector of derivatives added to the direction.
    algebra::Vector3D get_direction_derivatives() const;
  
    //! Add v to the derivative of the ith coordinate of the direction.
    void add_to_direction_derivative(int i, Float v, DerivativeAccumulator &d) {
      get_particle()->add_to_derivative(get_direction_key(i), v, d);
    }
  
    //! Add v to the derivatives of the x,y,z coordinates of the direction.
    void add_to_direction_derivatives(const algebra::Vector3D &v,
                                      DerivativeAccumulator &d);
  
    //! Get whether direction is optimized.
    bool get_direction_is_optimized() const;
  
    //! Set whether direction is optimized.
    void set_direction_is_optimized(bool tf) const;
  
    //! Get direction.
    algebra::Vector3D get_direction() const;
  
    //! Set direction.
    void set_direction(const algebra::Vector3D &v);
  
    //! Flip direction so direction is reversed.
    void reflect() { set_direction(-get_direction()); }

  private:
    static ObjectKey get_constraint_key();
};

IMP_DECORATORS(Direction, Directions, Decorators);


#if !defined(IMP_DOXYGEN) && !defined(SWIG)
//! Constrain a direction as unit and its derivative as tangent.
class IMPCOREEXPORT DirectionUnitConstraint : public IMP::Constraint {
  private:
    ParticleIndex pi_;

  private:
    DirectionUnitConstraint(Particle *p)
        : IMP::Constraint(p->get_model(), "DirectionUnitConstraint%1%")
        , pi_(p->get_index()) {}

  public:
    friend class Direction;
    virtual void do_update_attributes() IMP_OVERRIDE;
    virtual void do_update_derivatives(DerivativeAccumulator *da) IMP_OVERRIDE;
    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
    IMP_OBJECT_METHODS(DirectionUnitConstraint);
};
#endif

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DIRECTION_H */
