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
 IMP::setup_from_argv(argc,argv,"Test AttributeDistancePairScore");

 // create a new model
 IMP_NEW(Model,m,());
 IMP_NEW(RestraintSet, allrs, (m, "All restraints"));

 IMP_NEW(Particle,p1,(m,"p1"));
 IMP::core::XYZ::setup_particle(m,p1->get_index(),IMP::algebra::Vector3D
   (1.0,4.0,89.0));

 IMP_NEW(Particle,p2,(m,"p2"));
 IMP::core::XYZ::setup_particle(m,p2->get_index(),IMP::algebra::Vector3D
   (1.0,4.0,67.0));

 IMP_NEW(core::Harmonic, har, (0.0, 1.0));
 IMP_NEW(spb::AttributeDistancePairScore, adps,
          (har, core::XYZ::get_coordinate_key(2)));
IMP_NEW(core::PairRestraint, pr,(m,adps,IMP::internal::get_index
(ParticlePair(p1, p2))));
allrs->add_restraint(pr);

//std::cout << allrs->evaluate(false) << std::endl;
IMP_TEST_EQUAL(std::abs(allrs->evaluate(false)),242.0)

 return EXIT_SUCCESS;

}
