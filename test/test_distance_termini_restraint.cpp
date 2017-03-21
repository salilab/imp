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
 IMP::setup_from_argv(argc,argv,"Test ISD DistanceTerminiRestraint");

 // create a new model
 IMP_NEW(Model,m,());
 IMP_NEW(RestraintSet, allrs, (m, "All restraints"));

 IMP_NEW(Particle,p1,(m,"p1"));
 IMP::core::XYZ::setup_particle(m,p1->get_index(),IMP::algebra::Vector3D
   (1.0,4.0,9.0));

 IMP_NEW(Particle,p2,(m,"p2"));
 IMP::core::XYZ::setup_particle(m,p2->get_index(),IMP::algebra::Vector3D
   (1.0,4.0,7.0));

 // isd particle representing distance
 IMP_NEW(Particle, pdist, (m));
 isd::Scale dist = isd::Scale::setup_particle(pdist, 0.0);
 dist.set_lower(0.0);
 dist.set_upper(10.0);

 IMP_NEW(membrane::DistanceTerminiRestraint, dtr,
 (p1,p2,dist,0.001));
  dtr->set_name("Distance terminus restraint");

  allrs->add_restraint(dtr);

std::cout << allrs->evaluate(false) << std::endl;
//IMP_TEST_EQUAL(std::abs(allrs->evaluate(false)),242.0)

 return EXIT_SUCCESS;

}
