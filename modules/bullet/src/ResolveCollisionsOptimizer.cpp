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
#include <IMP/bullet/internal/utility.h>
#include <IMP/scoped.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/internal/map.h>
#include <IMP/atom/internal/SpecialCaseRestraints.h>
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#ifdef IMP_USE_CGAL
#include <IMP/cgal/internal/polyhedrons.h>
#endif

#include <btBulletDynamicsCommon.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/bind.hpp>


IMPBULLET_BEGIN_NAMESPACE


#define IMP_BNEW(Name, name, args) boost::scoped_ptr<Name> name(new Name args);
namespace {
  ObjectKey surface_key("bullet rigid body surface");
  class IMPBULLETEXPORT SurfaceMeshObject: public Object {
    //const algebra::Vector3Ds vertices_;
    //const std::vector<Ints> faces_;
    const std::pair<std::vector<btScalar>, Ints> btg_;
  public:
    SurfaceMeshObject(const algebra::Vector3Ds &vertices,
                      const std::vector<Ints> &faces):
      Object("Surface mesh for bullet"),
      btg_(internal::get_as_bt(vertices, faces)){}
    /*const algebra::Vector3Ds& get_vertices() const {
      return vertices_;
    }
    const std::vector<Ints>& get_faces() const {
      return faces_;
      }*/
    const std::vector<btScalar> &get_coordinates() const {
      return btg_.first;
    }
    const Ints& get_faces() const {
      return btg_.second;
    }
    IMP_OBJECT(SurfaceMeshObject);
  };

  void SurfaceMeshObject::do_show(std::ostream &out) const {
  }

  const double damping=1;
  btRigidBody *add_endpoint(btRigidBody *rb,
                            const algebra::Vector3D &center,
                            btDiscreteDynamicsWorld* world,
                            internal::Memory &memory) {
    if (!memory.empty_shape.get()) {
      memory.empty_shape.reset(new btEmptyShape());
    }
    btRigidBody *fallRigidBody
      = internal::create_rigid_body(memory.empty_shape.get(),
                                    algebra::Transformation3D(center),
                                    rb? -1: 0,
                                    world,memory);
    if (rb) {
      btTransform trans= rb->getCenterOfMassTransform().inverse();
      btVector3 lc= trans*internal::tr(center);
      btPoint2PointConstraint *p2p
        = new btPoint2PointConstraint(*rb, *fallRigidBody,
                                      lc, btVector3(0,0,0));
      memory.springs.push_back(p2p);
      world->addConstraint(p2p, true);
    }
    return fallRigidBody;
  }

  void add_spring(btRigidBody *rb0, btRigidBody *rb1,
                  const algebra::Vector3D &center0,
                  const algebra::Vector3D &center1,
                  double x0, double k,
                  btDiscreteDynamicsWorld* world,
                  internal::Memory &memory) {
    btRigidBody *anchor0= add_endpoint(rb0, center0, world,
                                       memory);
    btRigidBody *anchor1= add_endpoint(rb1, center1, world,
                                       memory);
    btTransform it; it.setIdentity();
    btTransform it1; it1.setIdentity();
    it1.setOrigin(btVector3(0, x0,0));
    // check if it is a rigid body
    // if so, look up offsets and use that
    // we need to check that there aren't alighment difficulties
    btGeneric6DofSpringConstraint *spring
      =new btGeneric6DofSpringConstraint(*anchor0, *anchor1,
                                         it, it1, true);
    memory.springs.push_back(spring);
    for (unsigned int i=1; i< 2; ++i) {
      spring->enableSpring(i, true);
      spring->setStiffness(i, 10*k);
      spring->setDamping(i,damping);
    }
    world->addConstraint(spring);
  }


  void add_rb_anchor(btRigidBody *rb0, double k,
                  btDiscreteDynamicsWorld* world,
                     internal::Memory &memory) {
    if (!memory.empty_shape.get()) {
      memory.empty_shape.reset(new btEmptyShape());
    }
    btScalar mass =0;
    btRigidBody *anchor
      = internal::create_rigid_body(memory.empty_shape.get(),
                     internal::tr(rb0->getCenterOfMassTransform()),
                                    mass,
                                    world,memory);
    // add pivot
    btTransform it; it.setIdentity();
    // check if it is a rigid body
    // if so, look up offsets and use that
    // we need to check that there aren't alighment difficulties
    btGeneric6DofSpringConstraint *spring
      =new btGeneric6DofSpringConstraint(*rb0, *anchor,
                                         it, it, true);
    memory.springs.push_back(spring);
    for (unsigned int i=0; i< 6; ++i) {
      spring->enableSpring(i, true);
      if (i < 3) {
        spring->setStiffness(i, k);
      } else {
        spring->setStiffness(i, .1*k);
      }
      spring->setDamping(i,damping);
    }
    spring->setEquilibriumPoint();
    world->addConstraint(spring, true);
  }


  void handle_xyzr(Particle *p,
                   double local,
                   internal::RigidBodyMap& map,
                   btDiscreteDynamicsWorld* world,
                   internal::Memory &memory) {
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
    internal::SpheresMap::const_iterator it
      = memory.spheres.find(d.get_radius());
    btCollisionShape* shape;
    if (it != memory.spheres.end()) {
      shape= it->second;
    } else {
      shape= new btSphereShape(d.get_radius());
      memory.spheres[d.get_radius()]= shape;
      memory.shapes.push_back(shape);
    }
    btRigidBody *rb
      = internal::create_rigid_body(shape,
              algebra::Transformation3D(d.get_coordinates()),
                                    mass,
                                    world,
                                    memory);
    rb->setCenterOfMassTransform(btTransform(btQuaternion(btVector3(1,0,0), 2),
                                  rb->getCenterOfMassTransform().getOrigin()));
    map[p]= rb;
    if (local>0) {
      add_spring(rb, NULL,
                 d.get_coordinates(), d.get_coordinates(), 0, local,
                 world, memory);
    }
  }




  void handle_rigidbody(Particle *p,
                        const ParticlesTemp &rp,
                        double local,
                        internal::RigidBodyMap& map,
                        btDiscreteDynamicsWorld* dynamicsWorld,
                        internal::Memory &memory) {
    core::RigidBody d(p);
    btScalar mass;
    if (d.get_coordinates_are_optimized()) {
      mass=.01;
      /*if (atom::Mass::particle_is_instance(p)) {
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
        }*/
    } else {
      mass=0;
    }
    if (!p->has_attribute(surface_key)) {
      //std::cout << "Mass of " << p->get_name() << " is " << mass << std::endl;
      std::vector<algebra::Sphere3D> spheres;
      for (unsigned int i=0; i< rp.size(); ++i) {
        core::XYZR dc(rp[i]);
        core::RigidMember dm(rp[i]);
        spheres.push_back(algebra::Sphere3D(dm.get_internal_coordinates(),
                                            dc.get_radius()));
        //std::cout << spheres.back() << std::endl;
      }
      if (spheres.empty()) {
        std::cout << "Empty: "<< p->get_name() << std::endl;
      }
      std::pair<algebra::Vector3Ds, std::vector<Ints> > impfaces
        = IMP::cgal::internal::get_skin_surface(spheres);
      Pointer<SurfaceMeshObject> smo= new SurfaceMeshObject(impfaces.first,
                                                            impfaces.second);
      p->add_attribute(surface_key, smo);
      IMP::core::add_rigid_body_cache_key(surface_key);
    }
    SurfaceMeshObject *smo
      = dynamic_cast<SurfaceMeshObject*>(p->get_value(surface_key));
    memory.geometry.push_back( smo );
    btTriangleIndexVertexArray *arr
      =new btTriangleIndexVertexArray(smo->get_faces().size()/3,
                                      const_cast<int*>(&smo->get_faces()[0]),
                                      3*sizeof(int),
                                      smo->get_coordinates().size()/3,
                            const_cast<btScalar*>(&smo->get_coordinates()[0]),
                                      3*sizeof(btScalar));
    memory.meshes.push_back(arr);
    btGImpactMeshShape *mesh= new btGImpactMeshShape(&memory.meshes.back());
    memory.shapes.push_back(mesh);
    mesh->updateBound();
    btRigidBody *fallRigidBody
      = internal::create_rigid_body(mesh,
                               d.get_reference_frame().get_transformation_to(),
                                    mass, dynamicsWorld, memory);

      /*btCollisionShape* shape= new btMultiSphereShape(&centers[0], &radii[0],
                                                    centers.size());
    btRigidBody *fallRigidBody
      = internal::create_rigid_body(shape,
                        algebra::Transformation3D(d.get_coordinates()),
                                    mass,
                                    dynamicsWorld,
                                    memory);*/
    map[p]=fallRigidBody;
    if (local > 0) {
      add_rb_anchor(fallRigidBody, local,
                    dynamicsWorld, memory);
    }
  }



  void handle_obstacle(std::vector<btScalar> &pts,
                       Ints &faces,
                       btDiscreteDynamicsWorld* dynamicsWorld,
                       internal::Memory &memory) {
    btTriangleIndexVertexArray *arr
      =new btTriangleIndexVertexArray(faces.size()/3,
                                      &faces[0],3*sizeof(int),
                                      pts.size()/3, &pts[0],
                                      3*sizeof(btScalar));
    memory.meshes.push_back(arr);
    btBvhTriangleMeshShape*shape
      =new btBvhTriangleMeshShape(&memory.meshes.back(),
                                  true);
    internal::create_rigid_body(shape,
                   algebra::Transformation3D(algebra::Vector3D(0,0,0)),
                                0,
                                dynamicsWorld, memory);
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
  ps_=core::XYZRs(ps);
}

void ResolveCollisionsOptimizer
::add_obstacle(const algebra::Vector3Ds &vertices,
               const std::vector<Ints > &tris) {
  obstacles_.push_back(internal::get_as_bt(vertices, tris));
}



// not anon due to bind issues
bool handle_harmonic(btDiscreteDynamicsWorld *world,
                     const internal::RigidBodyMap &map,
                     internal::Memory* memory,
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
             *memory);
  return true;
}


bool handle_ev() {
  return true;
}

/*
  Must pass rbs + normal particles to special case
 */


double ResolveCollisionsOptimizer::do_optimize(unsigned int iter) {
  {
  IMP_OBJECT_LOG;
  internal::Memory memory;
  // http://bulletphysics.org/mediawiki-1.5.8/index.php/Hello_World
  // change from btDbvtBroadphase
  IMP_BNEW(btSimpleBroadphase, broadphase, ());
  IMP_BNEW(btDefaultCollisionConfiguration, collisionConfiguration, ());
  IMP_BNEW(btCollisionDispatcher, dispatcher,
           (collisionConfiguration.get()));
  IMP_BNEW(btSequentialImpulseConstraintSolver, solver, ());
  IMP_BNEW(btDiscreteDynamicsWorld,
           dynamicsWorld, (dispatcher.get(),broadphase.get(),
                           solver.get(),collisionConfiguration.get()));

  dynamicsWorld->setGravity(btVector3(0,0,0));

  // for concave
  btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher.get());

  ParticlesTemp ps;
  if (ps_.empty()) {
    for (Model::ParticleIterator pit= get_model()->particles_begin();
         pit != get_model()->particles_end(); ++pit) {
      if (core::XYZR::particle_is_instance(*pit)
          && !core::RigidBody::particle_is_instance(*pit)) {
        ps.push_back(*pit);
      }
    }
  } else {
    ps= ps_;
  }


  internal::RigidBodyMap map;
  IMP::internal::Map<Particle*, ParticlesTemp> handled_bodies;
  ParticlesTemp xyzr_particles;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (core::RigidMember::particle_is_instance(ps[i])) {
      core::RigidBody d= core::RigidMember(ps[i]).get_rigid_body();
      handled_bodies[d].push_back(ps[i]);
    } else if (core::RigidBody::particle_is_instance(ps[i])) {
      IMP_THROW("Please pass rigid members not rigid body",
                ValueException);
      //http://www.bulletphysics.com/Bullet/BulletFull/classbtMultiSphereShape.html
    } else if (core::XYZR::particle_is_instance(ps[i])){
      handle_xyzr(ps[i], local_, map,
                  dynamicsWorld.get(), memory);
      xyzr_particles.push_back(ps[i]);
    }
  }
  for (IMP::internal::Map<Particle*, ParticlesTemp>::const_iterator it=
         handled_bodies.begin(); it != handled_bodies.end(); ++it) {
    handle_rigidbody(it->first, it->second, local_, map,
                     dynamicsWorld.get(), memory);
  }
  get_model()->update();
  ParticlesTemp root_particles= xyzr_particles;
  for (IMP::internal::Map<Particle*, ParticlesTemp>::const_iterator it=
         handled_bodies.begin(); it != handled_bodies.end(); ++it) {
      root_particles.push_back(it->first);
  }
  IMP::atom::internal::SpecialCaseRestraints scr(get_model(), root_particles);
  for (unsigned int i=0; i< rs_.size(); ++i) {
    scr.add_restraint_set(rs_[i],
                          boost::bind(handle_harmonic, dynamicsWorld.get(),
                                      map, &memory, _1, _2, _3),
                          handle_ev);
  }
  for (unsigned int i=0; i< obstacles_.size(); ++i) {
    handle_obstacle(obstacles_[i].first, obstacles_[i].second,
                    dynamicsWorld.get(),
                    memory);
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
    if (get_model()->get_number_of_restraints() > 0
        || get_number_of_optimizer_states() > 0) {
      internal::copy_back_coordinates(map);
      get_model()->evaluate(get_model()->get_number_of_restraints() >0);
      update_states();
      for (internal::RigidBodyMap::const_iterator
             it = map.begin(); it != map.end(); ++it) {
        // need to handle rigid bodies
        btTransform full_xform;
        it->second->getMotionState()->getWorldTransform(full_xform);
        btTransform xform(full_xform.getRotation());
        //std::cout << it->first->get_name() << std::endl;
        if (core::RigidBody::particle_is_instance(it->first)) {
          core::RigidBody d(it->first);
          algebra::Rotation3D out
            = d.get_reference_frame().get_transformation_from().get_rotation();
          if (d.get_coordinates_are_optimized()
              && d.get_torque().get_squared_magnitude() >0) {
            algebra::Vector3D torque= d.get_torque();
            //std::cout << "torque " << torque << std::endl;
            it->second->applyTorqueImpulse(internal::tr(torque));
          }
        }
        core::XYZ d(it->first);
        if (d.get_coordinates_are_optimized()
            && d.get_derivatives().get_squared_magnitude() >0) {
          algebra::Vector3D force=-d.get_derivatives();
          //std::cout << "force " << force << std::endl;
          if (force.get_squared_magnitude() > 5) {
            force= force.get_unit_vector()*5;
          }
          it->second->applyCentralForce(/*xform
                                        **/internal::tr(force));
        }
      }
    }
    dynamicsWorld->stepSimulation(5/60.f,10);
  }
  internal::copy_back_coordinates(map);
  } // clean up restraints
  double ret= get_model()->evaluate(false);
  update_states();
  return ret;
}


void ResolveCollisionsOptimizer::do_show(std::ostream &out) const {
}


IMPBULLET_END_NAMESPACE
