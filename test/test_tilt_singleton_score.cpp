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
 IMP::setup_from_argv(argc,argv,"Test tiltSingletonRestraint");

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
 rbps.push_back(d3);

 /*
 IMP::atom::Hierarchy h=IMP::atom::Hierarchy();
 h.add_child(IMP::atom::Hierarchy(p1));
 h.add_child(IMP::atom::Hierarchy(p2));
 h.add_child(IMP::atom::Hierarchy(p3));
 IMP::core::RigidBody rb=IMP::atom::create_rigid_body(h);
 */
 // create rigid body particle
 IMP_NEW(Particle, prb, (m));
  core::RigidBody rb = core::RigidBody::setup_particle(prb, rbps);

  algebra::Vector3D laxis = algebra::Vector3D(0.0, 0.0, 1.0);
  algebra::Vector3D zaxis = algebra::Vector3D(0.0, 0.0, 1.0);
  FloatRange tilt_range=FloatRange(0.0,45.0);
  IMP_NEW(core::HarmonicWell, well, (tilt_range, 1.0));
  IMP_NEW(TiltSingletonScore, tss, (well, laxis, zaxis));
  IMP_NEW(core::SingletonRestraint, sr1, (m,tss,
 rb.get_particle_index()));
  allrs->add_restraint(sr1);
//std::cout << allrs->evaluate(false) << std::endl;
IMP_TEST_EQUAL(std::abs(allrs->evaluate(false)),242.0)
 return EXIT_SUCCESS;

}
