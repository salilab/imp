/**
 *  \file RigidBodyPackingScore.cpp
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/membrane/RigidBodyPackingScore.h>
#include <IMP/membrane/HelixDecorator.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/TableRefiner.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/shortest_segment.h>

IMPMEMBRANE_BEGIN_NAMESPACE

RigidBodyPackingScore::RigidBodyPackingScore(core::TableRefiner *tbr,
                                             Floats omb, Floats ome,
                                             Floats ddb, Floats dde,
                                             double kappa):
                                             tbr_(tbr), omb_(omb), ome_(ome),
                                             ddb_(ddb), dde_(dde),
                                             kappa_(kappa) {}

Float RigidBodyPackingScore::evaluate_index(kernel::Model *m,
    const ParticleIndexPair &pip,DerivativeAccumulator *da) const
{
  // turn on logging for this method
  // IMP_OBJECT_LOG;

  // check if derivatives are requested
  IMP_USAGE_CHECK(!da, "Derivatives not available");

  // check if rigid body
  IMP_USAGE_CHECK(core::RigidBody::particle_is_instance(
     m->get_particle(pip[0])), "Particle is not a rigid body");
  IMP_USAGE_CHECK(core::RigidBody::particle_is_instance(
     m->get_particle(pip[1])), "Particle is not a rigid body");

  // check if rigid bodies are close enough
  Particles ps0=tbr_->get_refined(m->get_particle(pip[0]));
  Particles ps1=tbr_->get_refined(m->get_particle(pip[1]));

  unsigned int close = 0;
  for(unsigned int i=0;i<ps0.size();++i){
     for(unsigned int j=0;j<ps1.size();++j){
        if(core::get_distance(core::XYZR(ps0[i]),core::XYZR(ps1[j]))<0.6){
           ++close;
        }
        if(close >= 3) {break;}
     }
     if(close >= 3) {break;}
  }

  if (close < 3) {return 0.;}

  // assume they have an helix decorator
  membrane::HelixDecorator d0(m->get_particle(pip[0]));
  membrane::HelixDecorator d1(m->get_particle(pip[1]));
  // begin and end point
  algebra::Vector3D b0=algebra::Vector3D(d0.get_begin(),0.0,0.0);
  algebra::Vector3D e0=algebra::Vector3D(d0.get_end(),0.0,0.0);
  algebra::Vector3D b1=algebra::Vector3D(d1.get_begin(),0.0,0.0);
  algebra::Vector3D e1=algebra::Vector3D(d1.get_end(),0.0,0.0);

  // get rigid body transformation
  algebra::Transformation3D tr0=
   core::RigidBody(m->get_particle(pip[0])).get_reference_frame().
       get_transformation_to();
  algebra::Transformation3D tr1=
   core::RigidBody(m->get_particle(pip[1])).get_reference_frame().
       get_transformation_to();

  // and apply it to vectors
  b0=tr0.get_transformed(b0);
  e0=tr0.get_transformed(e0);
  b1=tr1.get_transformed(b1);
  e1=tr1.get_transformed(e1);

  //std::cout << b0 << " " << e0 << " " << b1 << " " << e1 <<std::endl;

  // get shortest segment
  algebra::Segment3D segment=
   algebra::get_shortest_segment(algebra::Segment3D(b0,e0),
                                 algebra::Segment3D(b1,e1));
  algebra::Vector3D t0=segment.get_point(0);
  algebra::Vector3D t1=segment.get_point(1);

  double sign = 1.0;
  algebra::Vector3D tmp0=e0;
  algebra::Vector3D tmp1=e1;

  // choose more convenient points for dihedral calculation
  if ((e0-t0).get_magnitude() < (b0-t0).get_magnitude()){
   tmp0=b0;
   sign *= -1.0;
  }
  if ((e1-t1).get_magnitude() < (b1-t1).get_magnitude()){
   tmp1=b1;
   sign *= -1.0;
  }

  //std::cout << tmp0 << " " << t0 << " " << t1 << " " << tmp1 <<std::endl;

  double omega=core::internal::dihedral(tmp0,t0,t1,tmp1,NULL,NULL,NULL,NULL);

  //std::cout << omega << std::endl;

  // correction for having changed dihedral points
  omega += (sign-1.0)*IMP::algebra::PI/2.0;
  if ( omega < -IMP::algebra::PI ) {omega += 2.0*IMP::algebra::PI;}

  double dist =segment.get_length();

  //std::cout << " OMEGA " << omega << " LENGTH " << dist << std::endl;

  // log something
  IMP_LOG(VERBOSE, "** The crossing angle is " << omega <<
                   " and the distance is " << dist << std::endl);

  // calculate the score
  double score=kappa_;
  for(unsigned int i=0;i<omb_.size();++i){
   if(omega >= omb_[i] && omega <= ome_[i] &&
      dist >= ddb_[i] && dist <= dde_[i]) {score=0.;}
  }

  return score;
}


ParticlesTemp RigidBodyPackingScore::do_get_inputs(
 kernel::Model *m, const kernel::ParticleIndexes &pis) const  {
  return IMP::kernel::get_particles(m, pis);
}


void RigidBodyPackingScore::show(std::ostream &out) const {
  for(unsigned int i=0;i<omb_.size();++i)
   out << "i" << i << "omb_=" << omb_[i] << "ome_=" << ome_[i] << std::endl;
}

IMPMEMBRANE_END_NAMESPACE
