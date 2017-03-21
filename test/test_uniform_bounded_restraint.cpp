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

IMP_NEW(membrane::UniformBoundedRestraint, ubr,
            (p2, FloatKey("z"),pa,pb));
allrs->add_restraint(ubr);

 return EXIT_SUCCESS;

}
