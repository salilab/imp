/**
 *  \file ProjectionParameters.h
 *  \brief Decorator for projection parameters
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECTION_PARAMETERS_H
#define IMPEM2D_PROJECTION_PARAMETERS_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/Decorator.h"
#include "IMP/ScoreState.h"



IMPEM2D_BEGIN_NAMESPACE

//! Decorator for particles representing the projection parameters
//! of a 2D-EM image
class IMPEM2DEXPORT ProjectionParameters: public Decorator {
public:

  IMP_DECORATOR(ProjectionParameters,Decorator);


  ~ProjectionParameters() {};

  //! Keys of the decorator
  static FloatKey* get_keys();

  //! ranges for the keys. Only adjust the translations, the rotations are
  //! self adjusted
  void set_proper_ranges_for_keys(Model *m,
                                algebra::Vector3D &min_translation_values,
                                algebra::Vector3D &max_translation_values);

  //! Create the proper attributes for a particle
  static ProjectionParameters setup_particle(Particle *p);

  //!Return true of the particle is a set of projection parameters
  static bool particle_is_instance(Particle *p);


  //! Get whether the parameters are optimized
  /** \return true only if all of them are optimized.
    */
  bool get_parameters_optimized() const {
    for(int i=0;i<6;++i) {
      if(get_particle()->get_is_optimized(get_keys()[i])==false) {
        return false;
      }
    }
    return true;
  }

  //! 2 First coordinates of the translation
  algebra::Vector2D get_translation2D() const {
     algebra::Vector2D v(get_translation_x(),get_translation_y());
     return v;
  }

  void set_translation2D(algebra::Vector2D &v) {
    set_translation(v[0],v[1],0.0);
  }

  void set_translation2D(double x,double y) {
     set_translation(x,y,0.0);
  }

  algebra::Vector3D get_translation() const {
    algebra::Vector3D v(
      get_translation_x(),get_translation_y(),get_translation_z());
    return v;
  }

  void set_translation(algebra::Vector3D &v) {
    set_translation(v[0],v[1],v[2]);
  }

  void set_translation(double x,double y,double z) {
    set_translation_x(x); set_translation_y(y); set_translation_z(z);
  }

   algebra::Rotation3D get_rotation() const {
     return algebra::get_rotation_from_fixed_zyz(
                                            get_Phi(),get_Theta(),get_Psi());
   }

  void set_rotation(double phi,double theta,double psi) {
    set_Phi(phi);
    set_Theta(theta);
    set_Psi(psi);
  }

  //! Set whether the parameters are optimized
  inline void set_parameters_optimized(bool tf) const {
    for(int i=0;i<5;++i) {
      get_particle()->set_is_optimized(get_keys()[i], tf);
    }
  }
  //! Get and set functions for the parameters
  IMP_DECORATOR_GET_SET(Phi,get_keys()[0],Float,double);
  IMP_DECORATOR_GET_SET(Theta,get_keys()[1],Float,double);
  IMP_DECORATOR_GET_SET(Psi,get_keys()[2],Float,double);
  // translation_x and translation_y are understood as coordinates,
  // NOT in matricial form
  IMP_DECORATOR_GET_SET(translation_x,get_keys()[3],Float,double);
  IMP_DECORATOR_GET_SET(translation_y,get_keys()[4],Float,double);
  IMP_DECORATOR_GET_SET(translation_z,get_keys()[5],Float,double);
};

IMP_OUTPUT_OPERATOR(ProjectionParameters);

typedef Decorators<ProjectionParameters, Particles> ProjectionParametersList;




//! Score state to keep angles and distances
class IMPEM2DEXPORT ProjectionParametersScoreState : public ScoreState {
public:
  ProjectionParametersScoreState(Particle *p) {
    proj_params_ = p;
  }
  IMP_SCORE_STATE(ProjectionParametersScoreState);

private:
  // Particle to store the projection params
  Pointer<Particle> proj_params_;
};


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_PARAMETERS_H */
