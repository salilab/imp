/**
 *  \file RigidBodyPackingScore.cpp
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/RigidBodyPackingScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/membrane/HelixDecorator.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/shortest_segment.h>
#include <IMP/core/internal/dihedral_helpers.h>

IMPMEMBRANE_BEGIN_NAMESPACE

RigidBodyPackingScore::RigidBodyPackingScore(Floats x0, Floats sigma,
                                          unsigned int nsig,  unsigned int ncl):
                               x0_(x0), sigma_(sigma), nsig_(nsig), ncl_(ncl) {}

Float RigidBodyPackingScore::evaluate(const ParticlePair &p,
                                 DerivativeAccumulator *da) const
{
  // turn on logging for this method
  IMP_OBJECT_LOG;
  // assume they have an helix decorator
  membrane::HelixDecorator d0(p[0]);
  membrane::HelixDecorator d1(p[1]);
  double omega;
  core::XYZ x0,x1,x2,x3;
  algebra::VectorD<3> b0,e0,b1,e1,t0,t1;
  algebra::Transformation3D tr0,tr1;
  algebra::Segment3D segment;

  // begin and end point
  b0=algebra::VectorD<3>(0.,0.,d0.get_begin());
  e0=algebra::VectorD<3>(0.,0.,d0.get_end());
  b1=algebra::VectorD<3>(0.,0.,d1.get_begin());
  e1=algebra::VectorD<3>(0.,0.,d1.get_end());

  // get rigid body transformation
  tr0=(p[0]->get_reference_frame()).get_transformation_to();
  tr1=(p[1]->get_reference_frame()).get_transformation_to();

  // and apply it to vectors
  b0=tr0.get_transformed(b0);
  e0=tr0.get_transformed(e0);
  b1=tr1.get_transformed(b1);
  e1=tr1.get_transformed(e1);

  // get shortest segment
  segment=algebra::get_shortest_segment(algebra::Segment3D(b0,e0),
                                        algebra::Segment3D(b1,e1));
  t0=segment.get_point(0);
  t1=segment.get_point(1);

  x0.set_coordinates(b0);
  x1.set_coordinates(t0);
  x2.set_coordinates(t1);
  x3.set_coordinates(b1);

  omega=core::internal::dihedral(x0, x1, x2, x3, NULL, NULL, NULL, NULL);

  // log something
  IMP_LOG(VERBOSE, "The crossing angle is" << omega << std::endl);
  double score=1.;
  // calculate score
  for(unsigned int i=0;i<ncl_;i++)
   if((omega > x0_[i]-nsig_*sigma_[i]) && (omega < x0_[i]+nsig_*sigma_[i]))
     score=0.;
  // check if derivatives are requested
  IMP_USAGE_CHECK(da, "Derivatives not available");
  return score;
}


bool RigidBodyPackingScore::get_is_changed(const ParticlePair &pp) const {
  // return whether the score may have changed since last invocation
  return pp[0]->get_is_changed() || pp[1]->get_is_changed();
}
ParticlesTemp RigidBodyPackingScore::get_input_particles(Particle *p) const {
  // return any particles that would be read if p is one of the particles
  // being scored. Don't worry about returning duplicates.
  return ParticlesTemp(1,p);
}
ContainersTemp RigidBodyPackingScore::get_input_containers(Particle *p) const {
  // return any containers that would be read if p is one of the particles
  // being scored. Don't worry about returning duplicates.
  return ContainersTemp();
}


void RigidBodyPackingScore::do_show(std::ostream &out) const {
  out << "ncl=" << ncl_ << std::endl;
}

IMPMEMBRANE_END_NAMESPACE
