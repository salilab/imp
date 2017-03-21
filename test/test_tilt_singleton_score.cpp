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

 IMP_NEW(Particle,p1,(m,"p1"));
 d1=IMP::core::XYZR::setup_particle(m,p1->get_index(),IMP::algebra::Sphere3D(
IMP::algebra::Vector3D(1.0,4.0,8.0),1.0));

 IMP_NEW(Particle,p2,(m,"p2"));
 d2=IMP::core::XYZR::setup_particle(m,p2->get_index(),IMP::algebra::Sphere3D(
IMP::algebra::Vector3D(1.0,4.0,6.0),1.0));

 IMP_NEW(Particle,p3,(m,"p3"));
 d3=IMP::core::XYZR::setup_particle(m,p3->get_index(),IMP::algebra::Sphere3D(
IMP::algebra::Vector3D(1.0,4.0,10.0),1.0));

 IMP::atom::Hierarchy h=IMP::atom::Hierarchy();
 h.add_child(d1);
 h.add_child(d2);
 h.add_child(d3);

 IMP::core::RigidBody rb=IMP::atom::create_rigid_body(h);

  algebra::Vector3D laxis = algebra::Vector3D(0.0, 0.0, 1.0);
  algebra::Vector3D zaxis = algebra::Vector3D(0.0, 0.0, 1.0);
  FloatRange tilt_range=FloatRange(0.0,45.0);
  IMP_NEW(core::HarmonicWell, well, (tilt_range, 1.0));
  IMP_NEW(TiltSingletonScore, tss, (well, laxis, zaxis));
  IMP_NEW(core::SingletonRestraint, sr1, (m,tss,
  IMP::internal::get_index(rb)));
  allrs->add_restraint(sr1);
/*
std::cout << allrs->evaluate(false) << std::endl;
//IMP_TEST_EQUAL(std::abs(allrs->evaluate(false)),242.0)
*/
 return EXIT_SUCCESS;

}
