/**
 * \brief test attributeDistancePairScore
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/spb.h>
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
using namespace IMP::spb;

int main(int argc, char* argv[])
{
 IMP::setup_from_argv(argc,argv,"Test PBCBoxedMover and PBCRigidBodyMover");

 // create a new model
 IMP_NEW(Model,m,());
 Particles ps;
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
ps.push_back(p3);

 IMP_NEW(Particle,p4,(m,"p4"));
 IMP::core::XYZR d4=IMP::core::XYZR::setup_particle(
 m,p4->get_index(),IMP::algebra::Sphere3D(
IMP::algebra::Vector3D(1.0,4.0,12.0),1.0));
atom::Mass mm4 = atom::Mass::setup_particle(p4, 30.0);



//define a rigid body for particles 1 and 2
 IMP_NEW(Particle, prb, (m));
  core::RigidBody rb = core::RigidBody::setup_particle(prb, rbps);
rb.set_coordinates_are_optimized(true);

  // define a transformation i..e translation + rotation
algebra::Vector3D trans1 = algebra::Vector3D(0.0, 0.0, 0.0);
algebra::Rotation3D rot1 = algebra::get_rotation_about_axis(
algebra::Vector3D(0.0, 0.0, 1.0), 0.00);
algebra::Transformation3D tr1 =
algebra::Transformation3D(rot1, trans1);

algebra::Vector3D trans2 = algebra::Vector3D(0.0, 0.0, 0.0);
algebra::Rotation3D rot2 = algebra::get_rotation_about_axis(
algebra::Vector3D(0.0, 0.0, 2.0), 0.00);
algebra::Transformation3D tr2 =
algebra::Transformation3D(rot2, trans2);

// vector of transformations
algebra::Transformation3Ds trs;
trs.push_back(tr1);
trs.push_back(tr2);

// define 3 scales
 IMP_NEW(Particle, px, (m));
 isd::Scale dx = isd::Scale::setup_particle(px, 1.0);
 IMP_NEW(Particle, py, (m));
 isd::Scale dy = isd::Scale::setup_particle(py, 1.0);
 IMP_NEW(Particle, pz, (m));
 isd::Scale dz = isd::Scale::setup_particle(pz, 1.0);

// cell centers
 algebra::Vector3Ds centers;
centers.push_back(algebra::Vector3D(1.0,4.0,10.0));
centers.push_back(algebra::Vector3D(1.0,4.0,0.0));

// define the mover for non-rigid particles
IMP_NEW(spb::PbcBoxedMover,pbm,(p4,ps,1.0,centers,trs,
 dx,dy,dz));

// define the mover for rigid particles
Particles fake;
IMP_NEW(spb::PbcBoxedRigidBodyMover,pbrm,
 (rb, fake, 1.0, 0.1, centers, trs, dx,dy,dz));

 return EXIT_SUCCESS;

}
