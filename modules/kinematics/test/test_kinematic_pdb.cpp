/**
   This is the program for creating a simple kinematic tree

*/
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/kinematics/KinematicForest.h>
#include <IMP/kinematics/PrismaticJoint.h>
#include <IMP/kinematics/TransformationJoint.h>
#include <IMP/kinematics/revolute_joints.h>
#include <string>

using namespace IMP::kinematics;

IMP::core::RigidBody create_rigid_particle
(IMP::Model* m, double x, double y, double z)
{
  using namespace IMP::algebra;
  IMP_NEW( IMP::Particle, p,  (m) );
  Vector3D v(x,y,z);
  Transformation3D T(v);
  ReferenceFrame3D RF(T);
  return IMP::core::RigidBody::setup_particle(p, RF);
}

IMP::Pointer<IMP::Model> build_model_no_pdb
(IMP::core::RigidBodies& rbs)
{
  IMP_NEW( IMP::Model, m, () );
  IMP_NEW( IMP::Particle, p0, (m) );
  IMP_NEW( IMP::Particle, p1, (m) );
  IMP_NEW( IMP::Particle, p2, (m) );

  rbs.push_back( create_rigid_particle ( m, 0, 0, 0 ) );
  rbs.push_back( create_rigid_particle ( m, 1, 0, 0 ) );
  rbs.push_back( create_rigid_particle ( m, 1, 1, 0 ) );
  rbs.push_back( create_rigid_particle ( m, 2, 1, 0 ) );
  rbs.push_back( create_rigid_particle ( m, 2, 2, 0 ) );
  return m;
 }

IMP::Pointer<IMP::Model> build_model_pdb
(std::string pdb_fname,
 IMP::core::RigidBodies& rbs,
 IMP::atom::Hierarchy& mhd)
{
  // read pdb
  //  IMP::Model* model = new IMP::Model();
  IMP_NEW( IMP::Model, m, () );
  mhd = IMP::atom::read_pdb(pdb_fname,
                            m,
                            new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                            // don't add radii
                            true, true);
   IMP::FloatKey radius_key= IMP::FloatKey("radius");

   // create rigid bodies
   IMP::ParticlesTemp hr =
     IMP::atom::get_by_type(mhd, IMP::atom::RESIDUE_TYPE);

   IMP::ParticlesTemp ps = get_by_type(mhd, IMP::atom::ATOM_TYPE);
   for(unsigned int i = 0; i < ps.size(); i++) {
     ps[i]->add_attribute(radius_key, 1.5);
     std::cout << IMP::core::XYZ(ps[i]).get_coordinates() << std::endl;
   }

   std::cerr << hr.size() << " residues were read from pdb file with "
             << ps.size() << " atoms"
             << std::endl;

   for(unsigned int i=0; i<hr.size(); i++) {
     rbs.push_back(IMP::atom::create_rigid_body(IMP::atom::Hierarchy(hr[i])));
     std::cout << "Rigid body number " << i << " is " << rbs[i] << std::endl;
   }

   return m;
}

void test_pdb_model(IMP::Model* model,
                    IMP::core::RigidBodies& rbs,
                    bool print_hierarchy = false,
                    IMP::atom::Hierarchy mhd = IMP::atom::Hierarchy() )
{
  IMP_ALWAYS_CHECK(rbs.size() >=5,
                   "Must have at least 5 rigid bodies but only got "
                   << rbs.size(), ValueException);
  std::cout << "initial coords residues";
  for(unsigned int i = 0; i < rbs.size(); i++) {
    std::cout << rbs[i].get_coordinates() << ", ";
  }
  std::cout << std::endl;
  std::cout << "initial coords atoms";
  for(unsigned int i = 0; i < rbs.size(); i++) {
    std::cout << "rb " << i << ": ";
    for(unsigned int j = 0; j < rbs[i].get_number_of_members(); j++){
      std::cout << rbs[i].get_member(j) << " "
                << IMP::core::XYZ(rbs[i].get_member(j)).get_coordinates()
                << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  IMP_NEW(IMP::kinematics::DihedralAngleRevoluteJoint, phi1,
          (rbs[0], rbs[1],
           rbs[0].get_member(2) /*C*/, rbs[1].get_member(0) /*N*/,
           rbs[1].get_member(1) /*CA*/, rbs[1].get_member(2) /*C*/) );
  IMP_NEW(IMP::kinematics::DihedralAngleRevoluteJoint, phi2,
          (rbs[1], rbs[2],
           rbs[1].get_member(2) /*C*/, rbs[2].get_member(0) /*N*/,
           rbs[2].get_member(1) /*CA*/, rbs[2].get_member(2) /*C*/) );
  IMP_NEW(IMP::kinematics::DihedralAngleRevoluteJoint, phi3,
          (rbs[2], rbs[3],
           rbs[2].get_member(2) /*C*/, rbs[3].get_member(0) /*N*/,
           rbs[3].get_member(1) /*CA*/, rbs[3].get_member(2) /*C*/) );
  IMP_NEW(IMP::kinematics::DihedralAngleRevoluteJoint, phi4,
          (rbs[3], rbs[4],
           rbs[3].get_member(2) /*C*/, rbs[4].get_member(0) /*N*/,
           rbs[4].get_member(1) /*CA*/, rbs[4].get_member(2) /*C*/) );

  IMP_NEW(IMP::kinematics::KinematicForest, kf, (model) );
  kf->add_edge(phi1);
  kf->add_edge(phi2);
  kf->add_edge(phi3);
  kf->add_edge(phi4);
  double phi1_init =  phi1->get_angle();
  double phi3_init =  phi3->get_angle();
  std::cout << "Phi1 " << IMP_RAD_2_DEG(phi1->get_angle()) << std::endl;
  std::cout << "Phi2 " << IMP_RAD_2_DEG(phi2->get_angle()) << " transformation "
            << phi2-> get_transformation_child_to_parent() <<std::endl;
  std::cout << "Phi3 " << IMP_RAD_2_DEG(phi3->get_angle()) << std::endl;
  phi1->set_angle(IMP_DEG_2_RAD(150.0));
  phi3->set_angle(IMP_DEG_2_RAD(240.0));

  kf->update_all_external_coordinates();
  std::cout << "Phi1 " << IMP_RAD_2_DEG(phi1->get_angle()) << std::endl;
  std::cout << "Phi2 " << IMP_RAD_2_DEG(phi2->get_angle()) << " transformation "
            << phi2-> get_transformation_child_to_parent() << std::endl;
  std::cout << "Phi3 " << IMP_RAD_2_DEG(phi3->get_angle()) << std::endl;
  IMP::atom::write_pdb
    (mhd, "./after_set_phi1_to_120deg.pdb");
  phi1->set_angle(phi1_init);
  phi3->set_angle(phi3_init);
  kf->update_all_external_coordinates();
  std::cout << "Phi1 " << IMP_RAD_2_DEG(phi1->get_angle()) << std::endl;
  std::cout << "Phi2 " << IMP_RAD_2_DEG(phi2->get_angle()) << " transformation "
            << phi2-> get_transformation_child_to_parent() << std::endl;
  std::cout << "Phi3 " << IMP_RAD_2_DEG(phi3->get_angle()) << std::endl;

 IMP::atom::write_pdb
    (mhd, "./after_set_phi1_back.pdb");

}

void test_model_with_rbs(IMP::Model* model,
                         IMP::core::RigidBodies& rbs,
                         bool print_hierarchy = false,
                         IMP::atom::Hierarchy mhd = IMP::atom::Hierarchy() )
{
  IMP_ALWAYS_CHECK(rbs.size() >=4,
                   "Must have at least 4 rigid bodies but only got "
                   << rbs.size(), ValueException)

  std::cout << "initial coords "
            << rbs[0].get_coordinates()
            << ", "
            << rbs[1].get_coordinates()
            << std::endl;

  IMP_NEW(IMP::kinematics::PrismaticJoint, pj, (rbs[0], rbs[1]));
  IMP_NEW(IMP::kinematics::TransformationJoint, tj, (rbs[1], rbs[2]));

  IMP_NEW(IMP::kinematics::KinematicForest, kf, (model) );
  kf->add_edge(pj);
  kf->add_edge(tj);

  std::cout << "coords after KinematicForest ctr "
            << kf->get_coordinates_safe( rbs[0] )
            << ", "
            << kf->get_coordinates_safe( rbs[1] )
            << ", "
            << kf->get_coordinates_safe( rbs[2] )
            << std::endl;
  std::cerr << "Distance 0-1: "
            << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[0] ),
                                           kf->get_coordinates_safe( rbs[1] ) )
            << std::endl;
  std::cerr << "Distance 1-2: "
            << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[1] ),
                                           kf->get_coordinates_safe( rbs[2] ) )
            << std::endl;
  std::cout << "length 0-1 = "
            << pj->get_length()
            << std::endl;
  std::cout << "trans 0-1 = "
            << pj->get_transformation_child_to_parent()
            << std::endl;
  std::cout << "trans 1-2 = "
            << tj->get_transformation_child_to_parent()
            << std::endl;

  std::cout << "Setting length to 10.0: " << std::endl;

  pj->set_length(10.0);
   std::cerr << "coords after set_length(10.0) " << std::endl;
   std::cerr << kf->get_coordinates_safe( rbs[0] )
             << ", "
             << kf->get_coordinates_safe( rbs[1] )
             << ", "
             << kf->get_coordinates_safe( rbs[2] )
             << std::endl;
   std::cerr << "Distance 0-1: "
             << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[0] ),
                                            kf->get_coordinates_safe( rbs[1] ) )
             << std::endl;
   std::cerr << "Distance 1-2: "
             << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[1] ),
                                            kf->get_coordinates_safe( rbs[2] ) )
             << std::endl;

   std::cout << "length 0-1 = "
             << pj->get_length()
             << std::endl;
   std::cout << "trans 0-1 = "
             << pj->get_transformation_child_to_parent()
             << std::endl;
   std::cout << "trans 1-2 = "
             << tj->get_transformation_child_to_parent()
             << std::endl;

   //IMP::atom::write_pdb
   //  (mhd, "./after_set_length_10_and_get_coords_safe.pdb");

  //
   std::cout << "Setting length to 20.0: " << std::endl;
   pj->set_length(20.0);
   kf->update_all_external_coordinates();
   std::cerr << "Distance 0-1: "
             << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[0] ),
                                            kf->get_coordinates_safe( rbs[1] ) )
             << std::endl;
   std::cerr << "Distance 1-2: "
             << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[1] ),
                                            kf->get_coordinates_safe( rbs[2] ) )
             << std::endl;

   if(print_hierarchy){
     IMP::atom::write_pdb
       (mhd, "./after_set_length_20_and_update_all_external.pdb");
   }

   std::cout << "length 0-1 = "
             << pj->get_length()
             << std::endl;
   std::cout << "trans 0-1 = "
             << pj->get_transformation_child_to_parent()
             << std::endl;
   std::cout << "trans 1-2 = "
             << tj->get_transformation_child_to_parent()
             << std::endl;

  // test that set_coords safe works
   std::cout << "setting rbs[0] coords to 0,0,3" << std::endl;
   kf->set_coordinates_safe
     ( rbs[0], IMP::algebra::Vector3D(0,0,3) );
   std::cout << "rb[0] coords after set_coords_safe "
             <<   kf->get_coordinates_safe( rbs[0] )           << std::endl;
   std::cout << "rb[1] coords "
             <<   kf->get_coordinates_safe( rbs[1] )           << std::endl;
   std::cout << "rb[2] coords "
             << kf->get_coordinates_safe( rbs[2] )
             << std::endl;
   std::cout << "hello" << std::endl;
   if(print_hierarchy){
     std::cout << "hi" << std::endl;
     IMP::atom::write_pdb
       (mhd, "./after_set_coords_safe_rb0__0_0_3.pdb");
   }
   std::cout << "ho" << std::endl;
   std::cout << "updated length 0-1 = " << std::endl;;
   std::cout << pj->get_length()
             << std::endl;
   std::cout << "updated trans 0-1 = "
             << pj->get_transformation_child_to_parent()
             << std::endl;
   std::cout << "updated trans 1-2 = "
             << tj->get_transformation_child_to_parent()
             << std::endl;
   std::cerr << "Distance 0-1: "
             << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[0] ),
                                            kf->get_coordinates_safe( rbs[1] ) )
             << std::endl;
   std::cerr << "Distance 1-2: "
             << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[1] ),
                                            kf->get_coordinates_safe( rbs[2] ) )
             << std::endl;

  // test that set_coords safe works
   std::cout << "setting rbs[1] coords to 0,0,0" << std::endl;
   kf->set_coordinates_safe
     ( rbs[1], IMP::algebra::Vector3D(0,0,0) );
   std::cout << "rb[0] coords after set_coords_safe "
             <<   kf->get_coordinates_safe( rbs[0] )           << std::endl;
   std::cout << "rb[1] coords "
             <<   kf->get_coordinates_safe( rbs[1] )           << std::endl;
   std::cout << "rb[2] coords "
             << kf->get_coordinates_safe( rbs[2] )
             << std::endl;
   if(print_hierarchy){
     IMP::atom::write_pdb
       (mhd, "./after_set_coords_safe_rb1__0_0_0.pdb");
   }
   std::cout << "updated length 0-1 = "
             << pj->get_length()
             << std::endl;
   std::cout << "updated trans 0-1 = "
             << pj->get_transformation_child_to_parent()
             << std::endl;
   std::cout << "updated trans 1-2 = "
             << tj->get_transformation_child_to_parent()
             << std::endl;
   std::cerr << "Distance 0-1: "
             << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[0] ),
                                            kf->get_coordinates_safe( rbs[1] ) )
             << std::endl;
   std::cerr << "Distance 1-2: "
             << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[1] ),
                                            kf->get_coordinates_safe( rbs[2] ) )
             << std::endl;

}

void print_transformation
(IMP::algebra::Transformation3D T, std::string description)
{
  std::pair< IMP::algebra::Vector3D, double > aa =
    IMP::algebra::get_axis_and_angle( T.get_rotation() );
  std::cout << "trans " << description << ": "
            << "axis = " << aa.first
            << "; angle = " << aa.second * 180.0 / 3.141256 << " deg"
            << "; translation = " << T.get_translation()
            << std::endl;
}

void print_info(KinematicForest* kf,
                IMP::core::RigidBodies& rbs,
                PrismaticJoint* pj0,
                DihedralAngleRevoluteJoint* dj1,
                DihedralAngleRevoluteJoint* dj2,
                PrismaticJoint* pj3,
                std::string action_desc)
{
  std::cout << std::endl
            << "******** After " << action_desc << " ********" << std::endl;

  std::cout << "Coords: "
            << kf->get_coordinates_safe( rbs[0] )
            << ", "
            << kf->get_coordinates_safe( rbs[1] )
            << ", "
            << kf->get_coordinates_safe( rbs[2] )
            << ", "
            << kf->get_coordinates_safe( rbs[3] )
            << ", "
            << kf->get_coordinates_safe( rbs[4] )
            << std::endl;
  std::cout << "Distance 0-1: "
            << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[0] ),
                                           kf->get_coordinates_safe( rbs[1] ) )
            << std::endl;
  std::cout << "Distance 1-2: "
            << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[1] ),
                                           kf->get_coordinates_safe( rbs[2] ) )
            << std::endl;
  std::cout << "Distance 2-3: "
            << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[2] ),
                                           kf->get_coordinates_safe( rbs[3] ) )
            << std::endl;
  std::cout << "Distance 3-4: "
            << IMP::algebra::get_distance( kf->get_coordinates_safe( rbs[3] ),
                                           kf->get_coordinates_safe( rbs[4] ) )
            << std::endl;
  std::cout << "length 0-1 = "
            << pj0->get_length()
            << std::endl;
  print_transformation( pj0->get_transformation_child_to_parent() , "0-1");
  std::cout << "angle 1-2 = "
            << dj1->get_angle() * 180 / 3.141256
            << std::endl;
  print_transformation( dj1->get_transformation_child_to_parent() , "1-2");
  std::cout << "angle 2-3 = "
            << dj2->get_angle() * 180 / 3.141256
            << std::endl;
  print_transformation( dj2->get_transformation_child_to_parent() , "2-3");
  std::cout << "length 3-4 = "
            << pj3->get_length()
            << std::endl;
  print_transformation( pj3->get_transformation_child_to_parent() , "3-4");
}


void test_dihedral
(IMP::Model* model,
 IMP::core::RigidBodies& rbs)
{
  IMP_ALWAYS_CHECK(rbs.size() >= 5,
                   "Must have at least 5 rigid bodies but only got "
                   << rbs.size(), ValueException)

  // joints
  IMP_NEW(IMP::kinematics::PrismaticJoint, pj0, (rbs[0], rbs[1]));
  IMP_NEW(IMP::kinematics::DihedralAngleRevoluteJoint, dj1,
          (rbs[1], rbs[2], rbs[0], rbs[1], rbs[2], rbs[3]));
  IMP_NEW(IMP::kinematics::DihedralAngleRevoluteJoint, dj2,
          (rbs[2], rbs[3], rbs[1], rbs[2], rbs[3], rbs[4]));
  IMP_NEW(IMP::kinematics::PrismaticJoint, pj3, (rbs[3], rbs[4]));
  // forest
  IMP_NEW(IMP::kinematics::KinematicForest, kf, (model) );
  kf->add_edge(pj0);
  kf->add_edge(dj1);
  kf->add_edge(dj2);
  kf->add_edge(pj3);

  print_info(kf, rbs, pj0, dj1, dj2, pj3, "KinematicForest ctr");
  dj1->set_angle(0.0 * 3.141259265358973/180);
  print_info(kf, rbs, pj0, dj1, dj2, pj3, "set_angle dj1 0 deg");
  //  dj1->set_angle(45.0 * 3.141259265358973/180);
  //print_info(kf, rbs, pj0, dj1, dj2, pj3, "set_angle 45 deg");
  dj1->set_angle(180.0 * 3.141259265358973/180);
  print_info(kf, rbs, pj0, dj1, dj2, pj3, "set_angle dj1 180 deg");
  dj2->set_angle(0.0 * 3.141259265358973/180);
  print_info(kf, rbs, pj0, dj1, dj2, pj3, "set_angle dj2 0 deg");
  dj1->set_angle(0.0 * 3.141259265358973/180);
  print_info(kf, rbs, pj0, dj1, dj2, pj3, "set_angle dj1 0 deg");
  kf->set_coordinates_safe( rbs[1], IMP::algebra::Vector3D( 0, 0, 1 ) );
  print_info(kf, rbs, pj0, dj1, dj2, pj3, "set_angle set_rbs[1] = 0,0,1");
  dj1->set_angle(0.0 * 3.141259265358973/180);
  print_info(kf, rbs, pj0, dj1, dj2, pj3, "set_angle 0 deg");
  dj1->set_angle(180.0 * 3.141259265358973/180);
  print_info(kf, rbs, pj0, dj1, dj2, pj3, "set_angle 180 deg");
}



int main(int argc, char **argv)
{
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  if(argc != 2) {
    std::cerr << "Usage: " << argv[0] << " pdb " << std::endl;
    exit(1);
  }
  std::string fname(argv[1]);
  std::cout << fname << std::endl;

  IMP::core::RigidBodies rbs1;
  IMP::Pointer<IMP::Model> m1 = build_model_no_pdb(rbs1);
  // test_model_with_rbs(m1, rbs1);
  test_dihedral(m1, rbs1);

  IMP::core::RigidBodies rbs2;
  IMP::atom::Hierarchy mhd2;
  IMP::Pointer<IMP::Model> m2 = build_model_pdb(fname, rbs2, mhd2);
  test_pdb_model(m2, rbs2, true, mhd2);
  //test_model_with_rbs(m2, rbs2, true, mhd);
  //test_dihedral(m2, rbs2);
}
