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
#include <IMP/test_macros.h>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{
 IMP::setup_from_argv(argc,argv,"Test DistanceRMSD");
 // create a new model
 IMP_NEW(Model,m,());
 IMP_NEW(Particle,p1,(m,"p1"));
 IMP_NEW(Particle,p2,(m,"p2"));
 IMP::core::XYZ p1coord=IMP::core::XYZ::setup_particle(m,p1->get_index());
 IMP::core::XYZ p2coord=IMP::core::XYZ::setup_particle(m,p2->get_index());

 // Define mandatory nuisance arguments to pass to constructor
 IMP_NEW(Particle,px,(m,"px"));
 IMP_NEW(Particle,py,(m,"py"));
 IMP_NEW(Particle,pz,(m,"pz"));

 IMP::isd::Scale pxscale=IMP::isd::Scale::setup_particle(m,px->get_index(),1.0);
 IMP::isd::Scale pyscale=IMP::isd::Scale::setup_particle(m,py->get_index(),1.0);
 IMP::isd::Scale pzscale=IMP::isd::Scale::setup_particle(m,pz->get_index(),1.0);

 // Particles to cluster
 Particles cluster_ps;
 cluster_ps.push_back(p1);
 cluster_ps.push_back(p2);
 // Assignments for symmetry
 Ints assign;
 assign.push_back(0);
 assign.push_back(1);

 // transformations for symmetry
 algebra::Transformation3Ds trs;
 trs.push_back(algebra::get_identity_transformation_3d());

 // Metric DRMS
 IMP_NEW(membrane::DistanceRMSDMetric,drmsd,(cluster_ps,assign,trs,
 pxscale,pyscale,pzscale));

 // first model's coordinates
 p1coord.set_coordinates(IMP::algebra::Vector3D(1.0,2.0,8.0));
 p2coord.set_coordinates(IMP::algebra::Vector3D(1.0,4.0,6.0));
 drmsd->add_configuration(1.0);

 // second model's coordinates
 p1coord.set_coordinates(IMP::algebra::Vector3D(1.0,2.0,9.0));
 p2coord.set_coordinates(IMP::algebra::Vector3D(-1.0,3.0,-4.0));
 drmsd->add_configuration(2.0);

 IMP_TEST_EQUAL(drmsd->get_weight(0),1.0);
 IMP_TEST_EQUAL(drmsd->get_weight(1),2.0);

 std::cout <<  drmsd->get_distance(0,1) <<std::endl;
 // if this is easy, add clustering part too
 return EXIT_SUCCESS;
}
