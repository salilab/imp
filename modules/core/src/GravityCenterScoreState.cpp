/**
 *  \file GravityCenterScoreState.cpp
 *  \brief Set particle to match the gravity center of one or more particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/core/GravityCenterScoreState.h>
#include <IMP/core/XYZDecorator.h>

#include <IMP/algebra/Vector3D.h>

IMPCORE_BEGIN_NAMESPACE

GravityCenterScoreState::GravityCenterScoreState(Particle *center,
                                                 FloatKey weightkey,
                                                 const Particles &ps)
  : center_(center), weightkey_(weightkey)
{
  XYZDecorator::cast(center);
  add_particles(ps);
}

void GravityCenterScoreState::do_before_evaluate()
{
  update_position();
}

// check that the particle is an xyz particle
IMP_LIST_IMPL(GravityCenterScoreState, Particle, particle, Particle*,
              XYZDecorator::cast(obj), update_position(),);

void GravityCenterScoreState::update_position()
{
  algebra::Vector3D cvect(0.0, 0.0, 0.0);
  bool do_weighting = (weightkey_ != FloatKey());

  Float total_weight = 0.;
  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;
    XYZDecorator d(p);
    Float weight = do_weighting ? p->get_value(weightkey_) : 1.0;
    total_weight += weight;
    for (int i = 0; i < 3; ++i) {
      cvect[i] += d.get_coordinate(i) * weight;
    }
  }

  if (total_weight != 0.0) {
    for (int i = 0; i < 3; ++i) {
      cvect[i] /= total_weight;
    }
  }
  XYZDecorator d(center_);
  d.set_coordinates_are_optimized(false);
  for (int i = 0; i < 3; ++i) {
    d.set_coordinate(i, cvect[i]);
  }
}

void GravityCenterScoreState::
transform_derivatives(DerivativeAccumulator *accpt)
{
  size_t nchildren = get_number_of_particles();
  if (nchildren > 0) {
    // we know center is OK since update was called
    XYZDecorator d(center_);
    algebra::Vector3D deriv;
    // divide derivatives equally between all children
    for (int i = 0; i < 3; ++i) {
      deriv[i] = d.get_coordinate_derivative(i) / nchildren;
    }

    for (ParticleIterator iter = particles_begin();
         iter != particles_end(); ++iter) {
      XYZDecorator d = XYZDecorator::cast(*iter);
      for (int i = 0; i < 3; ++i) {
        d.add_to_coordinate_derivative(i, deriv[i], *accpt);
      }
    }
  }
}

void GravityCenterScoreState::show(std::ostream &out) const
{
  out << "GravityCenter" << std::endl;
}

IMPCORE_END_NAMESPACE
