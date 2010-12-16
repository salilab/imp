/**
 *  \file RigidBodyPackingScore.cpp
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/RigidBodyPackingScore.h>
#include <IMP/membrane/HelixDecorator.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/shortest_segment.h>

IMPMEMBRANE_BEGIN_NAMESPACE

RigidBodyPackingScore::RigidBodyPackingScore(Floats omb, Floats ome,
                                             Floats ddb, Floats dde):
                                                omb_(omb), ome_(ome),
                                                ddb_(ddb), dde_(dde) {}

Float RigidBodyPackingScore::evaluate(const ParticlePair &p,
                                 DerivativeAccumulator *da) const
{
  // turn on logging for this method
  IMP_OBJECT_LOG;
  // assume they have an helix decorator
  membrane::HelixDecorator d0(p[0]);
  membrane::HelixDecorator d1(p[1]);
  double omega, dist, max_dist, min_dist;
  core::XYZ x0,x1,x2,x3;
  algebra::VectorD<3> b0,e0,b1,e1,t0,t1;
  algebra::Transformation3D tr0,tr1;
  algebra::Segment3D segment;

  // check if derivatives are requested
  IMP_USAGE_CHECK(!da, "Derivatives not available");

  // begin and end point

  b0=algebra::VectorD<3>(d0.get_begin(),0.0,0.0);
  e0=algebra::VectorD<3>(d0.get_end(),0.0,0.0);
  b1=algebra::VectorD<3>(d1.get_begin(),0.0,0.0);
  e1=algebra::VectorD<3>(d1.get_end(),0.0,0.0);

/**
  IMP_LOG(VERBOSE, "** BEFORE b0[0] " << b0[0] << " b0[1] "<<
                    b0[1] << " b0[2] " << b0[2] << std::endl);
  IMP_LOG(VERBOSE, "** BEFORE e0[0] " << e0[0] << " e0[1] "<<
                    e0[1] << " e0[2] " << e0[2] << std::endl);
  IMP_LOG(VERBOSE, "** BEFORE b1[0] " << b1[0] << " b1[1] "<<
                    b1[1] << " b1[2] " << b1[2] << std::endl);
  IMP_LOG(VERBOSE, "** BEFORE e1[0] " << e1[0] << " e1[1] "<<
                    e1[1] << " e1[2] " << e1[2] << std::endl);
**/
  // check if rigid body
  IMP_USAGE_CHECK(core::RigidBody::particle_is_instance(p[0]),
                  "Particle is not a rigid body");
  IMP_USAGE_CHECK(core::RigidBody::particle_is_instance(p[1]),
                  "Particle is not a rigid body");

  // get rigid body transformation
  tr0=core::RigidBody(p[0]).get_transformation();
  tr1=core::RigidBody(p[1]).get_transformation();

  // and apply it to vectors
  b0=tr0.get_transformed(b0);
  e0=tr0.get_transformed(e0);
  b1=tr1.get_transformed(b1);
  e1=tr1.get_transformed(e1);
/**
  IMP_LOG(VERBOSE, "** AFTER b0[0] " << b0[0] << " b0[1] "<<
                    b0[1] << " b0[2] " << b0[2] << std::endl);
  IMP_LOG(VERBOSE, "** AFTER e0[0] " << e0[0] << " e0[1] "<<
                    e0[1] << " e0[2] " << e0[2] << std::endl);
  IMP_LOG(VERBOSE, "** AFTER b1[0] " << b1[0] << " b1[1] "<<
                    b1[1] << " b1[2] " << b1[2] << std::endl);
  IMP_LOG(VERBOSE, "** AFTER e1[0] " << e1[0] << " e1[1] "<<
                    e1[1] << " e1[2] " << e1[2] << std::endl);
**/

  // get shortest segment
  segment=algebra::get_shortest_segment(algebra::Segment3D(b0,e0),
                                        algebra::Segment3D(b1,e1));
  t0=segment.get_point(0);
  t1=segment.get_point(1);

/**
  IMP_LOG(VERBOSE, "** t0[0] " << t0[0] << " t0[1] "<<
                    t0[1] << " t0[2] " << t0[2] << std::endl);
  IMP_LOG(VERBOSE, "** t1[0] " << t1[0] << " t1[1] "<<
                    t1[1] << " t1[2] " << t1[2] << std::endl);
**/

  omega=core::internal::dihedral(e0,t0,t1,e1,NULL,NULL,NULL,NULL);
  dist =segment.get_length();

  // log something
  IMP_LOG(VERBOSE, "** The crossing angle is " << omega <<
                   " and the distance is " << dist << std::endl);

  // find min and max distance
  min_dist=1.e+30;
  max_dist=-1.e+30;
  for(unsigned int i=0;i<omb_.size();i++){
   if(ddb_[i] < min_dist) min_dist=ddb_[i];
   if(dde_[i] > max_dist) max_dist=dde_[i];
  }
  // if TM are not interacting
  if(dist <= min_dist || dist >=max_dist) return 0.;

  // otherwise, calculate the score
  double score=1.;
  for(unsigned int i=0;i<omb_.size();i++)
   if(omega > omb_[i] && omega < ome_[i] &&
      dist > ddb_[i] && dist < dde_[i]) score=0.;

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
  for(unsigned int i=0;i<omb_.size();i++)
   out << "i" << i << "omb_=" << omb_[i] << "ome_=" << ome_[i] << std::endl;
}

IMPMEMBRANE_END_NAMESPACE
