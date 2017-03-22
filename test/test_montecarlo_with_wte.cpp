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
 IMP::setup_from_argv(argc,argv,"Test MC with WTE");

 // create a new model
 IMP_NEW(Model,m,());
IMP_NEW(RestraintSet, allrs, (m, "All restraints"));

 IMP_NEW(Particle,p1,(m,"p1"));
 IMP::core::XYZR d1=IMP::core::XYZR::setup_particle(
 m,p1->get_index(),IMP::algebra::Sphere3D(
IMP::algebra::Vector3D(1.0,4.0,8.0),1.0));
 atom::Mass mm1 = atom::Mass::setup_particle(p1, 30.0);

 // set the scoring function
 IMP_NEW(core::Harmonic, har, (0.0, 1.0));
 IMP_NEW(core::AttributeSingletonScore, ass,
          (har, core::XYZ::get_coordinate_key(2)));
IMP_NEW(core::SingletonRestraint, sr,(m,ass,p1->get_index()));
allrs->add_restraint(sr);


 //set the movers
 core::MonteCarloMovers mvs;
 Particles pps;
 pps.push_back(p1);
 IMP_NEW(core::BallMover, bmv, (pps, 1.0));
 mvs.push_back(bmv);

 // set WTE
 IMP::Pointer<core::MonteCarlo> mc;
 mc = new membrane::MonteCarloWithWte(
        m, -100.0, 5000.0, 0.001,
        9.0, 1.0);
  mc->set_return_best(false);
  mc->set_kt(1.0);
  mc->add_mover(new core::SerialMover(get_as<core::MonteCarloMoversTemp>(mvs)));
  mc->set_scoring_function(allrs);

  // run some sampling
  mc->optimize(10);
  return EXIT_SUCCESS;

}
