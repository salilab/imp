/**
 *  \file spb_representation.cpp
 *  \brief SPB Representation
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/membrane.h>
#include <IMP/display.h>
#include <string>
using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

atom::Hierarchies create_representation
(Model *m, SPBParameters mydata,
container::ListSingletonContainer *CP_ps,
container::ListSingletonContainer *IL2_ps, core::Movers& mvs)
{

atom::Hierarchies hs=atom::Hierarchies();

for(int i=0;i<mydata.num_cells;++i){

// create hierarchy for i-th cell
 IMP_NEW(Particle,p,(m));
 atom::Hierarchy all_mol=atom::Hierarchy::setup_particle(p);
 std::stringstream out;
 out << i;
 all_mol->set_name("Cell " + out.str() + " Hierarchy");
 hs.push_back(all_mol);

// useful vectors
 algebra::Vector3D CP_x0=mydata.CP_centers[i];
 algebra::Vector3D IL2_x0=mydata.IL2_centers[i];
 double ddz=mydata.CP_thickness/2.0+mydata.CP_IL2_gap/2.0;
 algebra::Vector3D CC_x0=algebra::Vector3D(CP_x0[0],CP_x0[1],ddz);

 for(int j=0;j<mydata.num_copies;++j){

  if(mydata.add_Spc42p){
  //Spc42p_n, 2 copies, 1 bead
   atom::Molecule Spc42p_n_0=
    create_protein(m,"Spc42p_n",7,1,
                     display::Color(175./255.,238./255.,238./255.),
                     i,mydata.kappa,CP_x0);
   atom::Molecule Spc42p_n_1=
    create_protein(m,"Spc42p_n",7,1,
                     display::Color(175./255.,238./255.,238./255.),
                     i,mydata.kappa,CP_x0);
   if(i==0){
    Particles ps_Spc42p_n_0=atom::get_leaves(Spc42p_n_0);
    Particles ps_Spc42p_n_1=atom::get_leaves(Spc42p_n_1);
    CP_ps->add_particles(ps_Spc42p_n_0);
    CP_ps->add_particles(ps_Spc42p_n_1);
    for(unsigned int k=0;k<ps_Spc42p_n_0.size();++k){
     Particles pps;
     pps.push_back(ps_Spc42p_n_0[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
    }
    for(unsigned int k=0;k<ps_Spc42p_n_1.size();++k){
     Particles pps;
     pps.push_back(ps_Spc42p_n_1[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
    }
   }
  //Spc42p_c, 2 copies, 2 beads
   atom::Molecule Spc42p_c_0=
    create_protein(m,"Spc42p_c",13,2,
                     display::Color(175./255.,218./255.,238./255.),
                     i,mydata.kappa,IL2_x0,139);
   atom::Molecule Spc42p_c_1=
    create_protein(m,"Spc42p_c",13,2,
                     display::Color(175./255.,218./255.,238./255.),
                     i,mydata.kappa,IL2_x0,139);
   if(i==0){
    Particles ps_Spc42p_c_0=atom::get_leaves(Spc42p_c_0);
    Particles ps_Spc42p_c_1=atom::get_leaves(Spc42p_c_1);
    IL2_ps->add_particles(ps_Spc42p_c_0);
    IL2_ps->add_particles(ps_Spc42p_c_1);
    for(unsigned int k=0;k<ps_Spc42p_c_0.size();++k){
     Particles pps;
     pps.push_back(ps_Spc42p_c_0[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
    }
    for(unsigned int k=0;k<ps_Spc42p_c_1.size();++k){
     Particles pps;
     pps.push_back(ps_Spc42p_c_1[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
    }
   }
   // Coiled-Coil
   atom::Molecules Spc42p_CC=
    create_coiled_coil(m,"Spc42_CC","2Q6Q_A.pdb", "2Q6Q_B.pdb",30,
                      display::Color(175./255.,208./255.,238./255.),
                      i, CC_x0,67);
   if(i==0){
    Particles ps_Spc42p_n_0=atom::get_leaves(Spc42p_n_0);
    Particles ps_Spc42p_c_0=atom::get_leaves(Spc42p_c_0);
    Particles ps_Spc42p_n_1=atom::get_leaves(Spc42p_n_1);
    Particles ps_Spc42p_c_1=atom::get_leaves(Spc42p_c_1);
    Particles ps_Spc42p;
    ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_n_0.begin(),ps_Spc42p_n_0.end());
    ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_c_0.begin(),ps_Spc42p_c_0.end());
    ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_n_1.begin(),ps_Spc42p_n_1.end());
    ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_c_1.begin(),ps_Spc42p_c_1.end());
    Particle *ps_tmp=atom::get_leaves(Spc42p_CC[0])[0];
    core::RigidBody prb=core::RigidMember(ps_tmp).get_rigid_body();
    IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
     (prb,ps_Spc42p,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs));
    mvs.push_back(rbmv);
   }
// now create the merge
   atom::Molecules Spc42p_0_all,Spc42p_1_all;
   Spc42p_0_all.push_back(Spc42p_n_0);
   Spc42p_0_all.push_back(Spc42p_CC[0]);
   Spc42p_0_all.push_back(Spc42p_c_0);
   Spc42p_1_all.push_back(Spc42p_n_1);
   Spc42p_1_all.push_back(Spc42p_CC[1]);
   Spc42p_1_all.push_back(Spc42p_c_1);

   atom::Molecule Spc42p_0=
     create_merged_protein(m,"Spc42p",Spc42p_0_all,i,mydata.kappa,0.0);
   atom::Molecule Spc42p_1=
     create_merged_protein(m,"Spc42p",Spc42p_1_all,i,mydata.kappa,0.0);
   all_mol.add_child(Spc42p_0);
   all_mol.add_child(Spc42p_1);
  }

  if(mydata.add_Spc29p){
  //Spc29p, 2 beads for N, 2 beads for C
   atom::Molecules Spc29p_all;
   atom::Molecule Spc29p_n=
    create_protein(m,"Spc29p_n",14.5,2,
                     display::Color(255./255.,165./255.,0.),
                     i,mydata.kappa,CP_x0);
   Spc29p_all.push_back(Spc29p_n);
   atom::Molecule Spc29p_c=
    create_protein(m,"Spc29p_c",14.5,2,
                     display::Color(255./255.,140./255.,0.),
                     i,mydata.kappa,CP_x0,132);
   Spc29p_all.push_back(Spc29p_c);
   atom::Molecule Spc29p=
    create_merged_protein(m,"Spc29p",Spc29p_all,i,mydata.kappa,0.0);
   all_mol.add_child(Spc29p);

   if(i==0){
    Particles ps_Spc29p=atom::get_leaves(Spc29p);
    CP_ps->add_particles(ps_Spc29p);
    IMP_NEW(membrane::PbcBoxedMover,mv,
           (ps_Spc29p[0],ps_Spc29p,mydata.MC.dx,mydata.CP_centers,mydata.trs));
    mvs.push_back(mv);
    for(unsigned int k=1;k<ps_Spc29p.size();++k){
     Particles pps;
     pps.push_back(ps_Spc29p[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
    }
   }
  }
  if(mydata.add_Spc110p){
  //Spc110p_c, 3 beads
   atom::Molecule Spc110p_c=
    create_protein(m,"Spc110p_c",26,3,
                     display::Color(255./255.,0.,0.),
                     i,mydata.kappa,CP_x0,627+164);
   all_mol.add_child(Spc110p_c);
   if(i==0){
    Particles ps_Spc110p_c=atom::get_leaves(Spc110p_c);
    CP_ps->add_particles(ps_Spc110p_c);
    IMP_NEW(membrane::PbcBoxedMover,mv,
           (ps_Spc110p_c[0],ps_Spc110p_c,mydata.MC.dx,
           mydata.CP_centers,mydata.trs));
    mvs.push_back(mv);
    for(unsigned int k=1;k<ps_Spc110p_c.size();++k){
     Particles pps;
     pps.push_back(ps_Spc110p_c[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
    }
   }
  }
  if(mydata.add_Cmd1p){
 //Cmd1p, 1 bead for N, 1 bead for C
   if(!mydata.use_structure){
    atom::Molecules Cmd1p_all;
    atom::Molecule Cmd1p_n=
     create_protein(m,"Cmd1p_n",8,1,
                     display::Color(255./255.,255./255.,0.),
                     i,mydata.kappa,CP_x0);
    Cmd1p_all.push_back(Cmd1p_n);
    atom::Molecule Cmd1p_c=
     create_protein(m,"Cmd1p_c",8,1,
                     display::Color(255./255.,215./255.,0.),
                     i,mydata.kappa,CP_x0,80);
    Cmd1p_all.push_back(Cmd1p_c);
    atom::Molecule Cmd1p=
     create_merged_protein(m,"Cmd1p",Cmd1p_all,i,mydata.kappa,0.0);
    all_mol.add_child(Cmd1p);
    if(i==0){
     Particles ps_Cmd1p=atom::get_leaves(Cmd1p);
     CP_ps->add_particles(ps_Cmd1p);
     IMP_NEW(membrane::PbcBoxedMover,mv,
           (ps_Cmd1p[0],ps_Cmd1p,mydata.MC.dx,mydata.CP_centers,mydata.trs));
     mvs.push_back(mv);
     for(unsigned int k=1;k<ps_Cmd1p.size();++k){
      Particles pps;
      pps.push_back(ps_Cmd1p[k]);
      IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
      mvs.push_back(bmv);
     }
    }
   }else{
    atom::Molecule Cmd1p=
     create_protein(m,"Cmd1p","3CLN.pdb",30,
                     display::Color(255./255.,255./255.,0.),
                     i,CP_x0,5);
    all_mol.add_child(Cmd1p);
    if(i==0){
     Particles ps_Cmd1p=atom::get_leaves(Cmd1p);
     Particles fake;
     CP_ps->add_particles(ps_Cmd1p);
     core::RigidBody prb=core::RigidMember(ps_Cmd1p[0]).get_rigid_body();
     IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
      (prb,fake,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs));
     mvs.push_back(rbmv);
    }
   }
  }
  if(mydata.add_Cnm67p_c){
 //Cnm67p_c, 2 beads
   if(!mydata.use_structure){
    atom::Molecule Cnm67p_c=
     create_protein(m,"Cnm67p_c",15,2,
                      display::Color(50./255.,205./255.,50./255.),
                      i,mydata.kappa,IL2_x0,272+179);
    all_mol.add_child(Cnm67p_c);
    if(i==0){
     Particles ps_Cnm67p_c=atom::get_leaves(Cnm67p_c);
     IL2_ps->add_particles(ps_Cnm67p_c);
     IMP_NEW(membrane::PbcBoxedMover,mv,
      (ps_Cnm67p_c[0],ps_Cnm67p_c,mydata.MC.dx,mydata.CP_centers,mydata.trs));
     mvs.push_back(mv);
     for(unsigned int k=1;k<ps_Cnm67p_c.size();++k){
      Particles pps;
      pps.push_back(ps_Cnm67p_c[k]);
      IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
      mvs.push_back(bmv);
     }
    }
   } else {
    atom::Molecule Cnm67p_c=
     create_protein(m,"Cnm67p_c","3OA7.pdb",30,
                      display::Color(50./255.,205./255.,50./255.),
                      i,IL2_x0,429);
    all_mol.add_child(Cnm67p_c);
    if(i==0){
     Particles ps_Cnm67p_c=atom::get_leaves(Cnm67p_c);
     Particles fake;
     IL2_ps->add_particles(ps_Cnm67p_c);
     core::RigidBody prb=core::RigidMember(ps_Cnm67p_c[0]).get_rigid_body();
     IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
       (prb,fake,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs));
     mvs.push_back(rbmv);
    }
   }
  }
 } // cycle on copies
} // cycle on cells
return hs;
}


atom::Molecule create_protein(Model *m,std::string name,double mass,int nbeads,
 display::Color colore,int copy,double kappa,
 algebra::Vector3D x0, int start_residue, int length)
{
 if(length==-1) {length=(int) (mass*1000.0/110.0);}
 IMP_NEW(Particle,p,(m));
 atom::Molecule protein=atom::Molecule::setup_particle(p);
 protein->set_name(name);
 double vol=atom::get_volume_from_mass(1000.0*mass)/(double)nbeads;
 double ms=1000.0*mass/(double)nbeads;
 double rg=algebra::get_ball_radius_from_volume_3d(vol);
 for(int i=0;i<nbeads;++i){
  IMP_NEW(Particle,pp,(m));
  int first=start_residue+i*(int)(length/nbeads);
  int last=start_residue+(i+1)*(int)(length/nbeads);
  std::stringstream out1,out2;
  out1 << i;
  out2 << copy;
  atom::Domain dom=atom::Domain::setup_particle(pp, IntRange(first, last));
  dom->set_name(name+out1.str()+"-"+out2.str());
  core::XYZR  d=core::XYZR::setup_particle(pp);
  d.set_radius(rg);
  d.set_coordinates(x0);
  d.set_coordinates_are_optimized(true);
  atom::Mass mm=atom::Mass::setup_particle(pp,ms);
  display::Colored cc=display::Colored::setup_particle(pp,colore);
  protein.add_child(dom);
 }
 if(nbeads>1 && copy==0){
  atom::Selections ss=atom::Selections();
  atom::Hierarchies hs=protein.get_children();
  for(unsigned int i=0;i<hs.size();++i){
   ss.push_back(atom::Selection(hs[i]));
  }
  Restraint *con=atom::create_connectivity_restraint(ss,kappa);
  con->set_name("Connectivity Restraint for "+name);
  m->add_restraint(con);
 }
 return protein;
}

atom::Molecule create_protein(Model *m,std::string name,
 std::string filename,int nbeads,display::Color colore,int copy,
 algebra::Vector3D x0,int start_residue, bool recenter)
{
 IMP_NEW(Particle,p,(m));
 atom::Molecule protein=atom::Molecule::setup_particle(p);
 protein->set_name(name);
 IMP_NEW(atom::CAlphaPDBSelector,sel,());
 atom::Hierarchy hpdb=atom::read_pdb(filename,m,sel);
 Particles ps=atom::get_leaves(hpdb);
 int nres=ps.size();
 core::XYZRs rbps;
 for(int i=0;i<nbeads;++i){
  IMP_NEW(Particle,pp,(m));
  int first=start_residue+i*(int)(nres/nbeads);
  int last=start_residue+(i+1)*(int)(nres/nbeads);
  std::stringstream out1,out2;
  out1 << i;
  out2 << copy;
  atom::Domain dom=atom::Domain::setup_particle(pp, IntRange(first, last));
  dom->set_name(name+out1.str()+"-"+out2.str());
  core::XYZR  d=core::XYZR::setup_particle(pp);
// calculate enclosing sphere and mass
  double ms=0.0;
  core::XYZRs xyz;
  for(int j=i*(int)(nres/nbeads);j<(i+1)*(int)(nres/nbeads);++j){
   atom::ResidueType restype=
    atom::Residue(atom::Atom(ps[j]).get_parent()).get_residue_type();
   double vol=atom::get_volume_from_residue_type(restype);
   double rg=algebra::get_ball_radius_from_volume_3d(vol);
   core::XYZR(ps[j]).set_radius(rg);
   xyz.push_back(core::XYZR(ps[j]));
   ms+=atom::Mass(ps[j]).get_mass();
  }
  algebra::Sphere3D sph=core::get_enclosing_sphere(xyz);
  d.set_radius(sph.get_radius());
  d.set_coordinates(sph.get_center());
  d.set_coordinates_are_optimized(true);
  atom::Mass mm=atom::Mass::setup_particle(pp,ms);
  display::Colored cc=display::Colored::setup_particle(pp,colore);
  protein.add_child(dom);
  rbps.push_back(d);
 }
 atom::destroy(hpdb);
 IMP_NEW(Particle,prb,(m));
 core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
 rb->set_name(name);
 if(recenter){
  // Check orientation of x-axis and topology
  double bb = (core::RigidMember(rbps[0]).get_internal_coordinates())[0];
  double ee = (core::RigidMember(rbps[nbeads-1]).get_internal_coordinates())[0];
  if (ee-bb<0.0){
   for(unsigned int k=0;k<rbps.size();++k){
    algebra::Vector3D coord=
    core::RigidMember(rbps[k]).get_internal_coordinates();
    algebra::Rotation3D rot=
     algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),IMP::PI);
    algebra::Transformation3D tr=
     algebra::Transformation3D(rot,algebra::Vector3D(0,0,0));
    core::RigidMember(rbps[k]).set_internal_coordinates
     (tr.get_transformed(coord));
   }
  }
  rb.set_reference_frame(algebra::ReferenceFrame3D(algebra::Transformation3D
      (algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),-IMP::PI/2.0),
       x0)));
 }
 return protein;
}


atom::Molecule create_merged_protein
(Model *m,std::string name,atom::Molecules proteins,
 int copy,double kappa,double dist)
{
 IMP_NEW(Particle,p,(m));
 atom::Molecule h=atom::Molecule::setup_particle(p);
 h->set_name(name);
 if (copy==0 && dist>=0.0){
  for(unsigned int j=0;j<proteins.size()-1;++j){
   add_internal_restraint(m,name,proteins[j],proteins[j+1],kappa,dist);
  }
 }
 for(unsigned int j=0;j<proteins.size();++j){
  Particles ps=atom::get_leaves(proteins[j]);
  for(unsigned int i=0;i<ps.size();++i){
   proteins[j].remove_child(atom::Domain(ps[i]));
   h.add_child(atom::Domain(ps[i]));
  }
  atom::destroy(proteins[j]);
 }
 return h;
}

atom::Molecules create_coiled_coil
(Model *m,std::string name,std::string filename_A, std::string filename_B,
int nbeads,display::Color colore,int copy,
algebra::Vector3D x0,int start_residue)
{

 atom::Molecule coil_A=create_protein(m,name,filename_A,
  nbeads,colore,copy,x0,start_residue,false);

 atom::Molecule coil_B=create_protein(m,name,filename_B,
  nbeads,colore,copy,x0,start_residue,false);

// now I need to destroy the two rigid bodies
 Particles psA=atom::get_leaves(coil_A);
 Particles psB=atom::get_leaves(coil_B);
 core::RigidBody::teardown_particle(core::RigidMember(psA[0]).get_rigid_body());
 core::RigidBody::teardown_particle(core::RigidMember(psB[0]).get_rigid_body());
// and make a new one
 core::XYZRs rbps;
 for(unsigned int i=0;i<psA.size();++i){rbps.push_back(core::XYZR(psA[i]));}
 for(unsigned int i=0;i<psB.size();++i){rbps.push_back(core::XYZR(psB[i]));}
 IMP_NEW(Particle,prb,(m));
 core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
 rb->set_name(name);

 // Check orientation of x-axis and topology
 double bb = (core::RigidMember(rbps[0]).get_internal_coordinates())[0];
 double ee = (core::RigidMember(rbps[psA.size()-1]).
  get_internal_coordinates())[0];
 if (ee-bb<0.0){
  for(unsigned int k=0;k<rbps.size();++k){
    algebra::Vector3D coord=
    core::RigidMember(rbps[k]).get_internal_coordinates();
    algebra::Rotation3D rot=
     algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),IMP::PI);
    algebra::Transformation3D tr=
     algebra::Transformation3D(rot,algebra::Vector3D(0,0,0));
    core::RigidMember(rbps[k]).set_internal_coordinates
     (tr.get_transformed(coord));
  }
 }

 rb.set_reference_frame(algebra::ReferenceFrame3D(algebra::Transformation3D
      (algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),-IMP::PI/2.0),
       x0)));

 atom::Molecules ret;
 ret.push_back(coil_A);
 ret.push_back(coil_B);
 return ret;
}

IMPMEMBRANE_END_NAMESPACE
