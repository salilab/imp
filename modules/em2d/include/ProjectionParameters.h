/**
 *  \file ProjectionParameters.h
 *  \brief Decorator for projection parameters
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECTION_PARAMETERS_H
#define IMPEM2D_PROJECTION_PARAMETERS_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/Decorator.h"
#include "IMP/decorator_macros.h"
#include "IMP/ScoreState.h"
#include "IMP/score_state_macros.h"



IMPEM2D_BEGIN_NAMESPACE

//! Decorator for particles representing the parameters
//! Rotation and tranlation. In this cases, the translation is the translation
//! to apply to the model in 3D, in order to perform the registration
class IMPEM2DEXPORT ProjectionParameters: public Decorator {
public:

  IMP_DECORATOR(ProjectionParameters,Decorator);


  ~ProjectionParameters() {};

  //! Keys of the decorator
  // FloatKeys get_keys();
  static const FloatKeys& get_keys();

  //! ranges for the keys. Only adjust the translations, the rotations are
  //! self adjusted
  void set_proper_ranges_for_keys(Model *m,
                  const algebra::Vector3D &min_translation_values,
                  const algebra::Vector3D &max_translation_values);

  //! Create the proper attributes for a particle
  static ProjectionParameters setup_particle(Particle *p);

  //!Return true of the particle is a set of projection parameters
  static bool particle_is_instance(Particle *p);


  //! Get whether the parameters are optimized
  /*! \return true only if all of them are optimized.
    */
  bool get_parameters_optimized() const {
    for(int i=0;i<6;++i) {
      if(get_particle()->get_is_optimized(get_keys()[i])==false) {
        return false;
      }
    }
    return true;
  }

  algebra::Vector3D get_translation() const {
    return algebra::Vector3D(get_translation_x(),
                             get_translation_y(),
                             0.0);

  }

  void set_translation(const algebra::Vector3D &v) {
    set_translation_x(v[0]);
    set_translation_y(v[1]);
  }

  algebra::Rotation3D get_rotation() const {
    algebra::Vector4D v(get_quaternion_1(),
                         get_quaternion_2(),
                         get_quaternion_3(),
                         get_quaternion_4());
    // normalize the vector. Given that the numbers generated with
    // simplex are not necessarily a unit vector.
    algebra::Vector4D unit=v.get_unit_vector();
    return algebra::Rotation3D(unit[0],
                               unit[1],
                               unit[2],
                               unit[3]);
  }

  void set_rotation(const algebra::Rotation3D &R) {
    set_quaternion_1(R.get_quaternion()[0]);
    set_quaternion_2(R.get_quaternion()[1]);
    set_quaternion_3(R.get_quaternion()[2]);
    set_quaternion_4(R.get_quaternion()[3]);
  }

  //! Set whether the parameters are optimized
  inline void set_parameters_optimized(bool tf) const {
    for(int i=0;i<6;++i) {
      get_particle()->set_is_optimized(get_keys()[i], tf);
    }
  }



  //! Get and set functions for the parameters
  IMP_DECORATOR_GET_SET(quaternion_1,get_keys()[0],Float,double);
  IMP_DECORATOR_GET_SET(quaternion_2,get_keys()[1],Float,double);
  IMP_DECORATOR_GET_SET(quaternion_3,get_keys()[2],Float,double);
  IMP_DECORATOR_GET_SET(quaternion_4,get_keys()[3],Float,double);
  IMP_DECORATOR_GET_SET(translation_x,get_keys()[4],Float,double);
  IMP_DECORATOR_GET_SET(translation_y,get_keys()[5],Float,double);

private:
  algebra::Rotation3D rotation_;
  algebra::Vector3D translation_;

};

IMP_DECORATORS(ProjectionParameters, ProjectionParametersList, Particles);




//! Score state to give information about the values of a ProjectionParameters
//! Decorator
class IMPEM2DEXPORT ProjectionParametersScoreState : public ScoreState {
public:
  ProjectionParametersScoreState(Particle *p):
      ScoreState(p->get_model(),
                 "ProjectionParametersScoreState%1%") {
    proj_params_ = p;
  }
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ProjectionParametersScoreState);

private:
  // Particle to store the projection params
  base::Pointer<Particle> proj_params_;
};



IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_PARAMETERS_H */
