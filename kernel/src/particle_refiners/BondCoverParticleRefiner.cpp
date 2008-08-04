/**
 *  \file BondCoverParticleRefiner.cpp
 *  \brief Cover a bond with a constant volume set of spheres
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/particle_refiners/BondCoverParticleRefiner.h"
#include "IMP/decorators/bond_decorators.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/internal/constants.h"

#include <cmath>

namespace IMP
{

BondCoverParticleRefiner::BondCoverParticleRefiner(FloatKey rk,
                                                   FloatKey vk): rk_(rk),
                                                                 vk_(vk)
{
  if (0) {
    BondCoverParticleRefiner t(rk, vk);
  }
}


bool BondCoverParticleRefiner::get_can_refine(Particle *p) const
{
  if (!BondDecorator::is_instance_of(p)) return false;
  return (p->has_attribute(vk_));

}

/* n (4/3) pi (d/(2n))^3 = v
   n^2= (4/3) pi (d/2)^3 / v
 */
Particles BondCoverParticleRefiner::get_refined(Particle *p) const
{
  IMP_assert(get_can_refine(p), "Trying to refine the unrefinable");

  Float v= p->get_value(vk_);
  BondDecorator bd(p);
  BondedDecorator e0= bd.get_bonded(0);
  BondedDecorator e1= bd.get_bonded(1);
  IMP_IF_CHECK(CHEAP) {
    XYZDecorator::cast(e0.get_particle());
    XYZDecorator::cast(e1.get_particle());
  }
  XYZDecorator d0(e0.get_particle());
  XYZDecorator d1(e1.get_particle());
  Float d= distance(d0, d1);

  float nsf=std::sqrt(internal::PI * d*square(d)/(6.0 * v));
  unsigned int ns= static_cast<unsigned int>(std::ceil(nsf) )+1;
  Float r, vt;
  Float last_error=-4*v;
  for(int i=0; i< 5; ++i) {
    r= d/(2.0*ns);
    vt= 4.0/3.0*internal::PI*r*square(r)*ns;
    Float err= vt-v;
    if (err > 0) {
      if (last_error < err) {
        IMP_LOG(VERBOSE, "adding to the number of spheres " << ns 
                << " " << err << " " << last_error << std::endl);
        ++ns;
        r= d/(2.0*ns);
        vt= 4.0/3.0*internal::PI*r*square(r)*ns;
      } else {
        IMP_LOG(VERBOSE, "Leaving the number of spheres " << ns 
                << " " << err << " " << last_error << std::endl);
      }
      break;
    } else {
      IMP_LOG(VERBOSE, "Subtracting from the number of spheres " << ns 
              << " " << err << std::endl);
      if (ns ==1) break;
      --ns;
      last_error=-err;
    }
  }

  IMP_LOG(VERBOSE, "Refining bond with length " << d << " into " 
          << ns << " particles" << std::endl);

  Vector3D vb= d0.get_vector();
  Vector3D ud= d0.get_vector_to(d1).get_unit_vector();
  Particles ret;
  Float f= d/(2.0*nsf);
  IMP_LOG(VERBOSE, "Resulting volume is " << vt
          << " (" << 4.0/3.0*internal::PI*f*square(f)*nsf << ")"
          << " with target of " << v << std::endl);
  IMP_LOG(VERBOSE, "Base coordinate is " << vb << " and unit vector is " 
          << ud << std::endl);
  for (unsigned int i=0; i< ns; ++i) {
    Particle *np= new Particle();
    p->get_model()->add_particle(np);
    XYZDecorator d= XYZDecorator::create(np);
    d.set_coordinates(vb+ (1+2*i)*r* ud);
    np->add_attribute(rk_, r, false);
    ret.push_back(np);
  }
  return ret;
}



void BondCoverParticleRefiner::cleanup_refined(Particle *p,
                                               Particles &ps,
                                               DerivativeAccumulator *da) const
{
  IMP_assert(get_can_refine(p), "Cleanup called with non-refinable particle");
  BondDecorator bd(p);
  BondedDecorator e0= bd.get_bonded(0);
  BondedDecorator e1= bd.get_bonded(1);
  IMP_IF_CHECK(CHEAP) {
    XYZDecorator::cast(e0.get_particle());
    XYZDecorator::cast(e1.get_particle());
  }
  XYZDecorator d0(e0.get_particle());
  XYZDecorator d1(e1.get_particle());

  if (da) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      Float w= (i+.5)/ (ps.size()+1);
      XYZDecorator d(ps[i]);
      DerivativeAccumulator da0(*da, w);
      DerivativeAccumulator da1(*da, 1-w);
      for (unsigned int i=0; i< 3; ++i) {
        d0.add_to_coordinate_derivative(i, d.get_coordinate_derivative(i), da0);
        d1.add_to_coordinate_derivative(i, d.get_coordinate_derivative(i), da1);
      }
    }
  }

  for (unsigned int i=0; i< ps.size(); ++i) {
    // note that the particles get deleted at this point currently
    ps[i]->get_model()->remove_particle(ps[i]->get_index());
  }
  ps.clear();
}

void BondCoverParticleRefiner::show(std::ostream &out) const
{
  out << "BondCoverParticleRefiner:\n"
      << "radius key: " << rk_ 
      << "\nvolume key: " << vk_ << std::endl;
}

} // namespace IMP
