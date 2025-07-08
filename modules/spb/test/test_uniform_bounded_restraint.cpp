/**
 * \brief test attributeDistancePairScore
 *
 */
#include <string>
#include <list>
#include <map>
#include <cmath>
#include <time.h>
#include <IMP/check_macros.h>
#include <IMP/flags.h>
#include <IMP/Model.h>
#include <IMP/RestraintSet.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/isd/Scale.h>
#include <IMP/spb/UniformBoundedRestraint.h>
#include <IMP/test/test_macros.h>

using namespace IMP;
using namespace IMP::spb;

int main(int argc, char* argv[])
{
 IMP::setup_from_argv(argc,argv,"Test uniformBoundedRestraint");

 // create a new model
 IMP_NEW(Model,m,());
 IMP_NEW(RestraintSet, allrs, (m, "All restraints"));

 IMP_NEW(Particle,p1,(m,"p1"));
 IMP::core::XYZ::setup_particle(m,p1->get_index(),IMP::algebra::Vector3D
   (1.0,4.0,9.0));

 IMP_NEW(Particle,p2,(m,"p2"));
 IMP::core::XYZ::setup_particle(m,p2->get_index(),IMP::algebra::Vector3D
   (1.0,4.0,9.0));

// isd particles representing boundaries
 IMP_NEW(Particle, pa, (m));
 isd::Scale a = isd::Scale::setup_particle(pa, 0.0);
 a.set_lower(0.0);
 a.set_upper(0.0);

 IMP_NEW(Particle, pb, (m));
 isd::Scale b = isd::Scale::setup_particle(pb, 100.0);
 b.set_lower(90.0);
 b.set_upper(110.0);

IMP_NEW(spb::UniformBoundedRestraint, ubr,
            (p2, FloatKey("z"),pa,pb));
allrs->add_restraint(ubr);

 return EXIT_SUCCESS;

}
