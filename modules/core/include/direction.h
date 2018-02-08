/**
 *  \file IMP/core/direction.h
 *  \brief Decorators for directions and the angle between them.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
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
  
    //! Get the derivative added to the ith coordinate of the direction.
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

//! Get angle between directions.
IMPCOREEXPORT double get_angle(Direction a, Direction b);

//! A decorator for an angle between two directions.
/** \ingroup decorators
    \see Direction
    \note A Constraint is used to maintain the angle and propagate its
          derivatives to its Directions.
*/
class IMPCOREEXPORT DirectionAngle : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const ParticleIndexPair &ds);
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                ParticleIndexAdaptor d0,
                                ParticleIndexAdaptor d1);
  void create_constraint();
  static ObjectKey get_constraint_key();

  public:
    IMP_DECORATOR_METHODS(DirectionAngle, Decorator);
    IMP_DECORATOR_SETUP_1(DirectionAngle, ParticleIndexPair, ds);
    IMP_DECORATOR_SETUP_2(DirectionAngle, ParticleIndexAdaptor, d0,
                          ParticleIndexAdaptor, d1);

    //! Check if particle is setup as an angle.
    static bool get_is_setup(Model *m, ParticleIndex pi);
  
    //! Get key for angle attribute.
    static FloatKey get_angle_key();

    //! Get key for ith direction particle.
    static ParticleIndexKey get_particle_key(unsigned int i);

    //! Get ith direction particle.
    Particle* get_particle(unsigned int i) const;

    Particle* get_particle() const {
      return Decorator::get_particle();
    }

    //! Get ith direction particle index.
    ParticleIndex get_particle_index(unsigned int i) const;

    ParticleIndex get_particle_index() const {
      return Decorator::get_particle_index();
    }

    //! Get angle.
    Float get_angle() const;

    //! Get the derivative added to the angle.
    Float get_angle_derivative() const;

    //! Add to the derivative of the angle.
    void add_to_angle_derivative(Float v, DerivativeAccumulator &d);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
    //! Recalculate and store the angle.
    void update_angle();
#endif

};

IMP_DECORATORS(DirectionAngle, DirectionAngles, Decorators);


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

//! Constrain an angle between two directions.
class IMPCOREEXPORT DirectionAngleConstraint : public IMP::Constraint {
  private:
    ParticleIndex pi_;

  private:
    DirectionAngleConstraint(Particle *p)
        : IMP::Constraint(p->get_model(), "DirectionAngleConstraint%1%")
        , pi_(p->get_index()) {}

  public:
    friend class DirectionAngle;
    virtual void do_update_attributes() IMP_OVERRIDE;
    virtual void do_update_derivatives(DerivativeAccumulator *da) IMP_OVERRIDE;
    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
    IMP_OBJECT_METHODS(DirectionAngleConstraint);
};

#endif

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DIRECTION_H */
