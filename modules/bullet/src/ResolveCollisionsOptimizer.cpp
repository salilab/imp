/**
 *  \file ResolveCollisionsOptimizer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/bullet/ResolveCollisionsOptimizer.h"

#include <IMP/core/rigid_bodies.h>
#include <IMP/core/XYZR.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/domino2/utility.h>
#include <IMP/scoped.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/internal/map.h>
#include <IMP/atom/internal/SpecialCaseRestraints.h>

#include <btBulletDynamicsCommon.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/bind.hpp>


IMPBULLET_BEGIN_NAMESPACE

typedef IMP::internal::Map<double, btCollisionShape*> Spheres;

#define IMP_BNEW(Name, name, args) std::auto_ptr<Name> name(new Name args);
namespace {
  btVector3 tr(const algebra::VectorD<3> &v) {
    return btVector3(v[0], v[1], v[2]);
  }
  const algebra::VectorD<3> tr(const btVector3 &v) {
    return algebra::VectorD<3>(v[0], v[1], v[2]);
  }
  const algebra::Rotation3D tr(const btQuaternion q) {
    return algebra::Rotation3D(q[3], q[0], q[1], q[2]);
  }
  const algebra::Transformation3D tr(const btTransform &v) {
    algebra::Transformation3D ret(tr(v.getRotation()), tr(v.getOrigin()));
    /*std::cout << ret.get_transformed(algebra::Vector3D(1,0,0))
              << " vs " << (v*btVector3(1,0,0))[0]
              << " " <<  (v*btVector3(1,0,0))[1]
              << " " <<  (v*btVector3(1,0,0))[2] << std::endl;
    std::cout << ret.get_transformed(algebra::Vector3D(0,1,0))
              << " vs " << (v*btVector3(0,1,0))[0]
              << " " <<  (v*btVector3(0,1,0))[1]
              << " " <<  (v*btVector3(0,1,0))[2] << std::endl;
    std::cout << ret.get_transformed(algebra::Vector3D(0,1,0))
              << " vs " << (v*btVector3(0,1,0))[0]
              << " " <<  (v*btVector3(0,1,0))[1]
              << " " <<  (v*btVector3(0,1,0))[2] << std::endl;*/
    return ret;
  }
  ParticlesTemp get_particles(const ParticlesTemp &in) {
    ParticlesTemp ret;
    for (unsigned int i=0; i< in.size(); ++i) {
      if (core::XYZR::particle_is_instance(in[i])) {
        ret.push_back(in[i]);
      }
    }
    IMP_LOG(TERSE, "Found " << ret.size() << " particles." << std::endl);
    return ret;
  }

  btRigidBody *add_endpoint(btRigidBody *rb,
                            const algebra::Vector3D &center,
                            btDiscreteDynamicsWorld* world,
                            boost::ptr_vector<btMotionState >& motion_states,
                            boost::ptr_vector<btRigidBody > &rigid_bodies,
                            boost::ptr_vector< btTypedConstraint> &springs) {
    static std::auto_ptr<btCollisionShape> es(new btEmptyShape());
    btScalar mass =0;
    if (rb) {
      mass=1.0/rb->getInvMass()*.001;
    }
    // compute coordinates
    // create rigid body
    btDefaultMotionState* fallMotionState
      = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
                                             tr(center)));
    motion_states.push_back(fallMotionState);
    //btVector3 fallInertia(1,0,0);
    //shape->calculateLocalInertia(mass,fallInertia);
    btRigidBody::btRigidBodyConstructionInfo
      fallRigidBodyCI(mass,fallMotionState,es.get()/*,fallInertia*/);
    btRigidBody* fallRigidBody= new btRigidBody(fallRigidBodyCI);
    fallRigidBody->setDamping(.5, .5);
    world->addRigidBody(fallRigidBody);
    rigid_bodies.push_back(fallRigidBody);
    // add pivot
    if (rb) {
      btTransform trans= rb->getCenterOfMassTransform().inverse();
      btVector3 lc= trans*tr(center);
      btPoint2PointConstraint *p2p
        = new btPoint2PointConstraint(*rb, *fallRigidBody,
                                      lc, btVector3(0,0,0));
      springs.push_back(p2p);
      world->addConstraint(p2p);
    }
    return fallRigidBody;
  }

  void add_spring(btRigidBody *rb0, btRigidBody *rb1,
                  const algebra::Vector3D &center0,
                  const algebra::Vector3D &center1,
                  double x0, double k,
                  btDiscreteDynamicsWorld* world,
                  boost::ptr_vector<btMotionState >& motion_states,
                  boost::ptr_vector<btRigidBody > &rigid_bodies,
                  boost::ptr_vector< btTypedConstraint> &springs) {
    btRigidBody *anchor0= add_endpoint(rb0, center0, world,
                                       motion_states, rigid_bodies, springs);
    btRigidBody *anchor1= add_endpoint(rb1, center1, world,
                                       motion_states, rigid_bodies, springs);
    btTransform it; it.setIdentity();
    btTransform it1; it1.setIdentity();
    it1.setOrigin(btVector3(x0, 0,0));
    // check if it is a rigid body
    // if so, look up offsets and use that
    // we need to check that there aren't alighment difficulties
    btGeneric6DofSpringConstraint *spring
      =new btGeneric6DofSpringConstraint(*anchor0, *anchor1,
                                         it, it1, true);
    springs.push_back(spring);
    spring->enableSpring(0, true);
    spring->setStiffness(0, k);
    spring->setDamping(0,.5);
    world->addConstraint(spring);
  }


  void handle_xyzr(Particle *p,
                   double local,
                   boost::ptr_vector<btCollisionShape > &shapes,
                   Spheres &spheres,
                   boost::ptr_vector<btMotionState >& motion_states,
                   IMP::internal::Map<Particle*, btRigidBody *>& map,
                   btDiscreteDynamicsWorld* world,
                   boost::ptr_vector<btRigidBody > &rigid_bodies,
                   boost::ptr_vector< btTypedConstraint> &springs) {
    core::XYZR d(p);
    btScalar mass;
    if (d.get_coordinates_are_optimized()) {
      if (atom::Mass::particle_is_instance(p)) {
        mass= atom::Mass(p).get_mass();
      } else {
        mass=1;
      }
    } else {
      mass=0;
    }
    Spheres::const_iterator it= spheres.find(d.get_radius());
    btCollisionShape* shape;
    if (it != spheres.end()) {
      shape= it->second;
    } else {
      shape= new btSphereShape(d.get_radius());
      spheres[d.get_radius()]= shape;
      shapes.push_back(shape);
    }
    btDefaultMotionState* fallMotionState
      = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
                                             tr(d.get_coordinates())));
    motion_states.push_back(fallMotionState);
    btVector3 fallInertia(0,0,0);
    shape->calculateLocalInertia(mass,fallInertia);
    btRigidBody::btRigidBodyConstructionInfo
      fallRigidBodyCI(mass,fallMotionState,shape,fallInertia);
    btRigidBody* fallRigidBody= new btRigidBody(fallRigidBodyCI);
    fallRigidBody->setDamping(.5, .5);
    map[p]= fallRigidBody;
    world->addRigidBody(fallRigidBody);
    rigid_bodies.push_back(fallRigidBody);
    if (local>0) {
      add_spring(fallRigidBody, NULL,
                 d.get_coordinates(), d.get_coordinates(), 0, local,
                 world, motion_states, rigid_bodies, springs);
    }
  }




  void handle_rigidbody(Particle *p,
                        const ParticlesTemp &rp,
                        boost::ptr_vector<btCollisionShape > &shapes,
                        Spheres &spheres,
                        boost::ptr_vector<btMotionState >& motion_states,
                        IMP::internal::Map<Particle*, btRigidBody *>& map,
                        btDiscreteDynamicsWorld* dynamicsWorld,
                        boost::ptr_vector<btRigidBody > &rigid_bodies) {
    core::RigidBody d(p);
    btScalar mass;
    if (d.get_coordinates_are_optimized()) {
      if (atom::Mass::particle_is_instance(p)) {
        mass= atom::Mass(p).get_mass();
      } else {
        mass=0;
        for (unsigned int i=0; i< rp.size(); ++i) {
          if (atom::Mass::particle_is_instance(rp[i])) {
            mass += atom::Mass(rp[i]).get_mass();
          } else {
            mass += 1;
          }
        }
      }
    } else {
      mass=0;
    }
    std::cout << "Mass of " << p->get_name() << " is " << mass << std::endl;
    std::vector<btVector3> centers;
    std::vector<btScalar> radii;
    for (unsigned int i=0; i< rp.size(); ++i) {
      core::XYZR dc(rp[i]);
      centers.push_back(tr(dc.get_coordinates()));
      radii.push_back(dc.get_radius());
    }

    btCollisionShape* shape= new btMultiSphereShape(&centers[0], &radii[0],
                                                    centers.size());
    btDefaultMotionState* fallMotionState
      = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
                                             tr(d.get_coordinates())));
    motion_states.push_back(fallMotionState);
    btVector3 fallInertia(0,0,0);
    shape->calculateLocalInertia(mass,fallInertia);
    btRigidBody::btRigidBodyConstructionInfo
      fallRigidBodyCI(mass,fallMotionState,shape,fallInertia);
    btRigidBody* fallRigidBody= new btRigidBody(fallRigidBodyCI);
    fallRigidBody->setDamping(.5, .5);
    map[p]= fallRigidBody;
    dynamicsWorld->addRigidBody(fallRigidBody);
    rigid_bodies.push_back(fallRigidBody);
  }



  void handle_obstacle(std::vector<btScalar> &pts,
                       Ints &faces,
                       boost::ptr_vector<btCollisionShape> &obstacles,
                       boost::ptr_vector<btTriangleIndexVertexArray> &meshes,
                       boost::ptr_vector<btMotionState >& motion_states,
                       btDiscreteDynamicsWorld* dynamicsWorld,
                       boost::ptr_vector<btRigidBody > &rigid_bodies) {
    meshes.push_back(new btTriangleIndexVertexArray(faces.size()/3,
                                                    &faces[0],3*sizeof(int),
                                                    pts.size()/3, &pts[0],
                                                    3*sizeof(btScalar)));
    obstacles.push_back(new btBvhTriangleMeshShape(&meshes.back(), true));
    motion_states.push_back(
      new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
                                           btVector3(0,-1,0))));
    btRigidBody::btRigidBodyConstructionInfo
      groundRigidBodyCI(0,&motion_states.back(),&obstacles.back(),
                        btVector3(0,0,0));
    rigid_bodies.push_back(new btRigidBody(groundRigidBodyCI));
    dynamicsWorld->addRigidBody(&rigid_bodies.back());
  }

}

/**
   Target implementation:
   decompose restraints
   find all restraints with a harmonic distance pair score
   and implement those
 */

ResolveCollisionsOptimizer
::ResolveCollisionsOptimizer(const RestraintSetsTemp &rs):
  Optimizer(rs[0]->get_model(), "ResolveCollisionsOptimizer %1%"),
  rs_(rs), local_(0){
}

ResolveCollisionsOptimizer::ResolveCollisionsOptimizer(Model *m):
  Optimizer(m, "ResolveCollisionsOptimizer %1%"),
  rs_(1, m->get_root_restraint_set()), local_(0){
}


void ResolveCollisionsOptimizer::set_xyzrs_internal(const core::XYZRsTemp &ps) {
  ps_=core::XYZRs(ps, core::XYZR::get_default_radius_key());
}

/*
void ResolveCollisionsOptimizer::add_obstacle
(const algebra::Vector3Ds &vertices,
const std::vector<boost::tuple<int,int,int> > &tris) {
    obstacles_.push_back(std::make_pair(tris, vertices));
    }*/
/**
   bonds use btGeneric6DofSpringConstraint set the transform of one
   to the center of the other and constrain x,y,z to be 0
   or just use a point-to-point constraint on the center of one
 */

void ResolveCollisionsOptimizer
::add_obstacle(const algebra::Vector3Ds &vertices,
               const std::vector<Ints > &tris) {
  obstacles_.push_back(std::make_pair(std::vector<btScalar>(vertices.size()*3),
                                      Ints(tris.size()*3)));
  for (unsigned int i=0; i< vertices.size(); ++i) {
    obstacles_.back().first[3*i]=vertices[i][0];
    obstacles_.back().first[3*i+1]=vertices[i][1];
    obstacles_.back().first[3*i+2]=vertices[i][2];
  }
  for (unsigned int i=0; i< tris.size(); ++i) {
    IMP_USAGE_CHECK(tris[i].size()==3, "Not a triangle: "
                    << tris[i].size() << " at " << i);
    obstacles_.back().second[3*i]=tris[i][0];
    obstacles_.back().second[3*i+1]=tris[i][1];
    obstacles_.back().second[3*i+2]=tris[i][2];
  }
}



// not anon due to bind issues
bool handle_harmonic(btDiscreteDynamicsWorld *world,
                     const IMP::internal::Map<Particle*, btRigidBody *> &map,
                     boost::ptr_vector<btMotionState > *motion_states,
                     boost::ptr_vector<btRigidBody > *rigid_bodies,
                     boost::ptr_vector< btTypedConstraint> *springs,
                     const ParticlePair &pp,
                     double x0, double k) {
  Particle *rp0, *rp1;
  if (core::RigidMember::particle_is_instance(pp[0])) {
    rp0= core::RigidMember(pp[0]).get_rigid_body();
  } else {
    rp0= pp[0];
  }
  if (core::RigidMember::particle_is_instance(pp[1])) {
    rp1= core::RigidMember(pp[1]).get_rigid_body();
  } else {
    rp1=pp[1];
  }
  btRigidBody *r0= map.find(rp0)->second;
  btRigidBody *r1= map.find(rp1)->second;

  add_spring(r0, r1, core::XYZ(pp[0]).get_coordinates(),
             core::XYZ(pp[1]).get_coordinates(),
             x0, k, world,
             *motion_states, *rigid_bodies, *springs);
  return true;
}


bool handle_ev() {
  return true;
}



/**
 dynamicsWorld->setGravity(btVector3(0,-10,0));

 btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
  shapes.push_back(sp(groundShape));
  btCollisionShape* fallShape = new btSphereShape(1);
  shapes.push_back(sp(fallShape));
  IMP_BNEW(btDefaultMotionState, groundMotionState,
           (btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0))));
  btRigidBody::btRigidBodyConstructionInfo
    groundRigidBodyCI(0,groundMotionState.get(),groundShape,btVector3(0,0,0));
  IMP_BNEW(btRigidBody, groundRigidBody, (groundRigidBodyCI));
  dynamicsWorld->addRigidBody(groundRigidBody.get());

  IMP_BNEW(btDefaultMotionState, fallMotionState,
           (btTransform(btQuaternion(0,0,0,1),btVector3(0,50,0))));
  btScalar mass = 1;
  btVector3 fallInertia(0,0,0);
  fallShape->calculateLocalInertia(mass,fallInertia);
  btRigidBody::btRigidBodyConstructionInfo
    fallRigidBodyCI(mass,fallMotionState.get(),fallShape,fallInertia);
  IMP_BNEW(btRigidBody, fallRigidBody, (fallRigidBodyCI));
  dynamicsWorld->addRigidBody(fallRigidBody.get());
 */

double ResolveCollisionsOptimizer::optimize(unsigned int iter) {
  {
  IMP_OBJECT_LOG;
  // http://bulletphysics.org/mediawiki-1.5.8/index.php/Hello_World
  IMP_BNEW(btDbvtBroadphase, broadphase, ());
  IMP_BNEW(btDefaultCollisionConfiguration, collisionConfiguration, ());
  IMP_BNEW(btCollisionDispatcher, dispatcher,
           (collisionConfiguration.get()));
  IMP_BNEW(btSequentialImpulseConstraintSolver, solver, ());
  IMP_BNEW(btDiscreteDynamicsWorld,
           dynamicsWorld, (dispatcher.get(),broadphase.get(),
                           solver.get(),collisionConfiguration.get()));
  dynamicsWorld->setGravity(btVector3(0,0,0));
  ParticlesTemp ps;
  if (ps_.empty()) {
    ps= get_particles(ParticlesTemp(get_model()->particles_begin(),
                                    get_model()->particles_end()));
  } else {
    ps= ps_;
  }

  boost::ptr_vector<btCollisionShape > shapes;
  boost::ptr_vector<btMotionState > motion_states;
  boost::ptr_vector<btRigidBody > rigid_bodies;
  IMP::internal::Map<Particle*, btRigidBody *> map;
  boost::ptr_vector<btTypedConstraint > springs;
  boost::ptr_vector< ScopedRemoveRestraint> restraints;
  boost::ptr_vector<btCollisionShape> obstacles;
  boost::ptr_vector<btTriangleIndexVertexArray> meshes;
  typedef IMP::internal::Map<double, btCollisionShape*> Spheres;
  Spheres spheres;
  IMP::internal::Map<Particle*, ParticlesTemp> handled_bodies;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (core::RigidMember::particle_is_instance(ps[i])) {
      core::RigidBody d= core::RigidMember(ps[i]).get_rigid_body();
      handled_bodies[d].push_back(ps[i]);
    } else if (core::RigidBody::particle_is_instance(ps[i])) {
      IMP_THROW("Please pass rigid members not rigid body",
                ValueException);
      //http://www.bulletphysics.com/Bullet/BulletFull/classbtMultiSphereShape.html
    } else if (core::XYZR::particle_is_instance(ps[i])){
      handle_xyzr(ps[i], local_, shapes, spheres, motion_states, map,
                  dynamicsWorld.get(), rigid_bodies, springs);
    }
  }
  for (IMP::internal::Map<Particle*, ParticlesTemp>::const_iterator it=
         handled_bodies.begin(); it != handled_bodies.end(); ++it) {
    handle_rigidbody(it->first, it->second, shapes, spheres, motion_states, map,
                     dynamicsWorld.get(), rigid_bodies);
  }
  get_model()->update();
  IMP::atom::internal::SpecialCaseRestraints scr(get_model(), ps);
  for (unsigned int i=0; i< rs_.size(); ++i) {
    scr.add_restraint_set(rs_[i],
                          boost::bind(handle_harmonic, dynamicsWorld.get(),
                                      map, &motion_states, &rigid_bodies,
                                      &springs, _1, _2, _3),
                          handle_ev);
  }
  for (unsigned int i=0; i< obstacles_.size(); ++i) {
    handle_obstacle(obstacles_[i].first, obstacles_[i].second,
                    obstacles, meshes, motion_states, dynamicsWorld.get(),
                    rigid_bodies);
  }
  IMP_IF_LOG(TERSE) {
    ScoreStatesTemp sst
      = get_required_score_states(RestraintsTemp(rs_.begin(), rs_.end()));
    {
      IMP_LOG(TERSE, "Score states are ");
      for (unsigned int i=0; i< sst.size(); ++i) {
        IMP_LOG(TERSE, sst[i]->get_name() << " ");
      }
      IMP_LOG(TERSE, std::endl);
    }
    IMP_LOG(TERSE, "Special cased " << restraints.size()
            << " restraint." << std::endl);
    unsigned int rrs=0;
    for (unsigned int i=0; i< rs_.size(); ++i) {
      rrs+=get_restraints(rs_[i]).size();
    }
    IMP_LOG(TERSE, "Remaining " << rrs << " restraints: ");
    {
      for (unsigned int i=0; i< rs_.size(); ++i) {
        Restraints crs= get_restraints(rs_[i]);
        for (unsigned int j=0; j< crs.size(); ++j) {
          IMP_LOG(TERSE, crs[j]->get_name() <<" ");
        }
      }
      IMP_LOG(TERSE, std::endl);
    }
  }
  RestraintsTemp utrestraints;
  std::vector<double> weights;
  boost::tie(utrestraints, weights)
    = get_restraints_and_weights(rs_.begin(), rs_.end());
  for (unsigned int i=0; i< iter; ++i) {
    if (utrestraints.size() >0) {
      get_model()->evaluate(utrestraints, weights, true);
      for (IMP::internal::Map<Particle*, btRigidBody *>::const_iterator
             it = map.begin(); it != map.end(); ++it) {
        // need to handle rigid bodies
        if (core::RigidBody::particle_is_instance(it->first)) {
          core::RigidBody d(it->first);
          if (d.get_coordinates_are_optimized()) {
            it->second->applyCentralForce(tr(-d.get_derivatives()));
          }
        } else {
          core::XYZ d(it->first);
          if (d.get_coordinates_are_optimized()) {
            it->second->applyCentralForce(tr(-d.get_derivatives()));
          }
        }
      }
    }
    dynamicsWorld->stepSimulation(1/60.f,10);
    if (iter== i+1 || utrestraints.size() >0) {
      for (IMP::internal::Map<Particle*, btRigidBody *>::const_iterator
             it = map.begin(); it != map.end(); ++it) {
        btTransform trans;
        it->second->getMotionState()->getWorldTransform(trans);
        if (core::RigidBody::particle_is_instance(it->first)) {
          core::RigidBody(it->first).set_transformation(tr(trans));
        } else {
          core::XYZ(it->first).set_coordinates(tr(trans.getOrigin()));
        }
      }
    }
    update_states();
  }
  }
  return get_model()->evaluate(false);
}


void ResolveCollisionsOptimizer::do_show(std::ostream &out) const {
}


IMPBULLET_END_NAMESPACE
