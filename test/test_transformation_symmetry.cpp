/**
 * \brief test attributeDistancePairScore
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/membrane.h>
#include <IMP/display.h>
#include <IMP/rmf.h>
#include <string>
#include <list>
#include <map>
#include <math.h>
#include <time.h>
#include <IMP/check_macros.h>
#include <IMP/flags.h>
#include <IMP/test/test_macros.h>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{
 IMP::setup_from_argv(argc,argv,"Test TransformationSymmetry");

 // create a new model
 IMP_NEW(Model,m,());
 IMP_NEW(RestraintSet, allrs, (m, "All restraints"));
 core::XYZRs rbps;

 IMP_NEW(Particle,p1,(m,"p1"));
 IMP::core::XYZR d1=IMP::core::XYZR::setup_particle(
 m,p1->get_index(),IMP::algebra::Sphere3D(
IMP::algebra::Vector3D(1.0,4.0,8.0),1.0));
 atom::Mass mm1 = atom::Mass::setup_particle(p1, 30.0);
rbps.push_back(d1);

 IMP_NEW(Particle,p2,(m,"p2"));
 IMP::core::XYZR d2=IMP::core::XYZR::setup_particle(
m,p2->get_index(),IMP::algebra::Sphere3D(
IMP::algebra::Vector3D(1.0,4.0,6.0),1.0));
 atom::Mass mm2 = atom::Mass::setup_particle(p2, 30.0);
rbps.push_back(d2);

 IMP_NEW(Particle,p3,(m,"p3"));
 IMP::core::XYZR d3=IMP::core::XYZR::setup_particle(
 m,p3->get_index(),IMP::algebra::Sphere3D(
IMP::algebra::Vector3D(1.0,4.0,10.0),1.0));
atom::Mass mm3 = atom::Mass::setup_particle(p3, 30.0);

 // create rigid body particle
 IMP_NEW(Particle, prb, (m));
  core::RigidBody rb = core::RigidBody::setup_particle(prb, rbps);

 // define a transformation i..e translation + rotation
  algebra::Vector3D trans = algebra::Vector3D(0.0, 80.0, 0.0);
algebra::Rotation3D rot = algebra::get_rotation_about_axis(
 algebra::Vector3D(0.0, 0.0, 1.0), 2.09);
algebra::Transformation3D tr =
 algebra::Transformation3D(rot, trans);

// define 3 scales
 IMP_NEW(Particle, px, (m));
 isd::Scale dx = isd::Scale::setup_particle(px, 1.0);
 IMP_NEW(Particle, py, (m));
 isd::Scale dy = isd::Scale::setup_particle(py, 1.0);
 IMP_NEW(Particle, pz, (m));
 isd::Scale dz = isd::Scale::setup_particle(pz, 1.0);

// define the transformation symmetry
IMP_NEW(membrane::TransformationSymmetry, sm,
            (tr,dx,dy,dz));

// constrain rigid bodies
core::Reference::setup_particle(prb, prb);
IMP_NEW(core::SingletonConstraint, crb, (sm, NULL,m,rb.get_particle_index()));
m->add_score_state(crb);

// constrain non rigid bodies
core::Reference::setup_particle(p3, p3);
IMP_NEW(core::SingletonConstraint, cflex, (sm, NULL,m,p3->get_index()));
m->add_score_state(cflex);

 return EXIT_SUCCESS;

}
