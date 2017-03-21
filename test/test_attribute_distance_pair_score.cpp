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

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{
 IMP::setup_from_argv(argc,argv,"Test AttributeDistancePairScore");

 // create a new model
 IMP_NEW(Model,m,());
 IMP_NEW(Particle,p1,(m,"p1"));
 IMP::core::XYZ::setup_particle(m,p1->get_index(),IMP::algebra::Vector3D
   (1.0,4.0,89.0));

 IMP_NEW(Particle,p2,(m,"p2"));
 IMP::core::XYZ::setup_particle(m,p2->get_index(),IMP::algebra::Vector3D
   (1.0,4.0,67.0));

 IMP_NEW(core::Harmonic, har, (0.0, 1.0));
 IMP_NEW(membrane::AttributeDistancePairScore, adps,
          (har, core::XYZ::get_coordinate_key(2)));
IMP_NEW(IMP::core::PairRestraint, pr,(m,adps,IMP::internal::get_index
(ParticlePair(p1, p2))));
//IMP::Pointer<IMP::core:PairRestraint> rst=dynamic_cast
// <IMP::core::PairRestraint*>(pr);

std::cout << pr.get_score() << std::endl;
/*IMP_USAGE_CHECK(std::abs(pr->get_score()-144.0),
 "Values for AttributeDistancePairScorenot matching");
*/
 return EXIT_SUCCESS;

}
