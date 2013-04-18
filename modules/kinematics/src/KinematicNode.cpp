/**
 *  \file KinematicNode. cpp
 *  \brief functionality for defining nodes on a kinematic chain
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/kinematics/KinematicForest.h>
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/kinematics/Joint.h>
#include <IMP/exception.h>

IMPKINEMATICS_BEGIN_NAMESPACE



// sets up a node in a kinematic tree
KinematicNode
KinematicNode::setup_particle
( Particle*p, KinematicForest* owner,
  Joint* in_joint,  Joints out_joints)
{
  // Checks:
  if ( owner == nullptr ) {
    IMP_THROW( "Kinematic node must have a valid owner kinematic forest",
               IMP::ValueException );
  }
  if ( IMP::core::RigidMember::particle_is_instance(p) ) {
    // see also RigidBody::add_member
    IMP_THROW("RigidMemer cannot be set as KinematicNode at this point,"
              << " in order to guarantee coherent coordinates update",
              IMP::ValueException);
  }

  if ( ! IMP::core::RigidBody::particle_is_instance(p) ) {
    IMP::core::RigidBody::setup_particle(p, ParticlesTemp() );
  }
  p->get_model()->add_attribute(get_owner_key(),
                                p->get_index(),
                                owner);
  if( in_joint ){
    p->get_model()->add_attribute(get_in_joint_key(),
                                  p->get_index(),
                                  in_joint);
  }
  if( !out_joints.empty() ){
    p->get_model()->add_attribute(get_out_joints_key(),
                                  p->get_index(),
                                  out_joints);
  }
  return KinematicNode(p);
}


//! returns the kinematic forest associated with this node
KinematicForest*
KinematicNode::get_owner()
{
  Object* owner_as_obj =
    get_model()->get_attribute
    ( get_owner_key(), get_particle_index() );
  // safe downcast cause we upcasted it to Object* to begin with
  KinematicForest* owner =
    dynamic_cast<KinematicForest*> (owner_as_obj);
  return owner;
}

void
KinematicNode::set_out_joints(Joints in) {
  if( in.empty() ) {
    IMP_THROW( "cannot set an empty list of out_joints",
               IMP::ValueException );
  }
  if( get_model()->get_has_attribute( get_out_joints_key(),
                                      get_particle_index() ) )
    {
      get_model()->set_attribute( get_out_joints_key(),
                                  get_particle_index(), in );
    }
  else
    {
      get_model()->add_attribute( get_out_joints_key(),
                                  get_particle_index(), in );
    }
}

void
KinematicNode::add_out_joint(Joint* j)
{
  if( j == nullptr ) {
    IMP_THROW( "cannot add a NULL out_joint to KinematicNode",
               IMP::ValueException );
  }
  Joints joints;
  if( get_model()->get_has_attribute( get_out_joints_key(),
                                      get_particle_index() ) )
    {
      joints = get_out_joints();
    }
  joints.push_back(j);
  set_out_joints(joints);
}

void
KinematicNode::set_in_joint(Joint* j)
{
  if( j == nullptr ) {
    IMP_THROW( "cannot set a NULL in_joint to KinematicNode",
               IMP::ValueException );
  }
  if( get_model()->get_has_attribute( get_in_joint_key(),
                                      get_particle_index() ) )
    {
      get_model()->set_attribute( get_in_joint_key(),
                                  get_particle_index(), j );
    }
  else
    {
      get_model()->add_attribute(get_in_joint_key(),
                                 get_particle_index(),
                                 j );
    }
}

void KinematicNode::show(std::ostream &out) const {
  RigidBody::show(out);
}

IMPKINEMATICS_END_NAMESPACE
