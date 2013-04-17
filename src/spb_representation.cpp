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
#include <IMP/rmf.h>
#include <string>
#include <list>
#include <map>
#include <math.h>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

atom::Hierarchies create_representation
(Model *m, SPBParameters mydata,
 container::ListSingletonContainer *CP_ps,
 container::ListSingletonContainer *IL2_ps,
 core::Movers& mvs,
 Particle *SideXY, Particle *SideZ)
{

atom::Hierarchies hs=atom::Hierarchies();

for(int i=0;i<mydata.num_cells;++i){

// create hierarchy for i-th cell
 IMP_NEW(Particle,p,(m));
 atom::Hierarchy all_mol=atom::Hierarchy::setup_particle(p);
 std::stringstream out;
 out << i;
 all_mol->set_name("Cell " + out.str() + " Hierarchy");

// useful vectors
 algebra::Vector3D CP_x0  = mydata.CP_centers[i];
 algebra::Vector3D IL2_x0 = mydata.IL2_centers[i];
 algebra::Vector3D CC_x0  =
  algebra::Vector3D(CP_x0[0], CP_x0[1], CP_x0[2] + mydata.CP_IL2_gap/2.0);
 algebra::Vector3D CC2_x0 =
  algebra::Vector3D(CP_x0[0], CP_x0[1], CP_x0[2] - mydata.CP_thicknessMax);

 for(int j=0;j<mydata.num_copies;++j){
//
// SPC42P
//
  if(mydata.protein_list["Spc42p"]){
  //Spc42p_n, 2 copies, 1 bead
   atom::Molecules Spc42p_n, Spc42p_c;
   for(unsigned int k=0;k<2;++k){
    Spc42p_n.push_back(create_protein(m,"Spc42p_n",7,1,
                       display::Color(175./255.,238./255.,238./255.),
                       i,mydata.kappa,CP_x0,
                       mydata.use_connectivity,1,59));
    if(i==0){
     Particles ps=atom::get_leaves(Spc42p_n[k]);
     CP_ps->add_particles(ps);
     add_BallMover(ps,mydata.MC.dx,mvs);
    }
  //Spc42p_c, 2 copies, 2 beads
    Spc42p_c.push_back(create_protein(m,"Spc42p_c",13,2,
                       display::Color(175./255.,218./255.,238./255.),
                       i,mydata.kappa,IL2_x0,mydata.use_connectivity,138));
    if(i==0){
     Particles ps=atom::get_leaves(Spc42p_c[k]);
     IL2_ps->add_particles(ps);
     add_BallMover(ps,mydata.MC.dx,mvs);
    }
   }
   // Coiled-Coil
   atom::Molecules Spc42p_CC=
    create_coiled_coil(m,"Spc42_CC","CC_78_A.pdb", "CC_78_B.pdb",
                       mydata.resolution,
                       display::Color(175./255.,208./255.,238./255.),
                       i, CC_x0, 59);
   if(i==0){
    Particles ps;
    for(unsigned int k=0;k<2;++k){
     Particles ps_n=atom::get_leaves(Spc42p_n[k]);
     Particles ps_c=atom::get_leaves(Spc42p_c[k]);
     ps.insert(ps.end(),ps_n.begin(),ps_n.end());
     ps.insert(ps.end(),ps_c.begin(),ps_c.end());
    }
    core::RigidBody rb=
     core::RigidMember(atom::get_leaves(Spc42p_CC[0])[0]).get_rigid_body();
    IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
     (rb,ps,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs,
      SideXY,SideXY,SideZ));
    mvs.push_back(rbmv);
   }
// now create the merge
   for(unsigned int k=0;k<2;++k){
    atom::Molecules Spc42p_all;
    Spc42p_all.push_back(Spc42p_n[k]);
    Spc42p_all.push_back(Spc42p_CC[k]);
    Spc42p_all.push_back(Spc42p_c[k]);
    atom::Molecule Spc42p=
     create_merged_protein(m,"Spc42p",Spc42p_all,i,mydata.kappa,0.0);
    all_mol.add_child(Spc42p);
   }
   if(mydata.add_GFP){
    for(unsigned k=0;k<2;++k){
     atom::Molecule gfp_n=
      create_GFP(m,"Spc42p-N-GFP",i,CP_ps,CP_x0,mvs,mydata,SideXY,SideZ);
     all_mol.add_child(gfp_n);
     atom::Molecule gfp_c=
      create_GFP(m,"Spc42p-C-GFP",i,IL2_ps,IL2_x0,mvs,mydata,SideXY,SideZ);
     all_mol.add_child(gfp_c);
    }
   }
  }
//
// SPC29P
//
  if(mydata.protein_list["Spc29p"]){
  //Spc29p, 2 beads for N, 2 beads for C
   atom::Molecules Spc29p_all;
   Spc29p_all.push_back(create_protein(m,"Spc29p_n",14.5,2,
                        display::Color(255./255.,215./255.,0.),
                        i,mydata.kappa,CP_x0,mydata.use_connectivity));
   Spc29p_all.push_back(create_protein(m,"Spc29p_c",14.5,2,
                        display::Color(255./255.,140./255.,0.),
                        i,mydata.kappa,CP_x0,mydata.use_connectivity,132));
   atom::Molecule Spc29p=
    create_merged_protein(m,"Spc29p",Spc29p_all,i,mydata.kappa,0.0);
   all_mol.add_child(Spc29p);
   if(i==0){
    Particles ps=atom::get_leaves(Spc29p);
    CP_ps->add_particles(ps);
    add_PbcBoxedMover(ps,mydata.MC.dx,mydata.CP_centers,mydata.trs,mvs,
                      SideXY,SideZ);
   }
   if(mydata.add_GFP){
    atom::Molecule gfp_n=
     create_GFP(m,"Spc29p-N-GFP",i,CP_ps,CP_x0,mvs,mydata,SideXY,SideZ);
    all_mol.add_child(gfp_n);
    atom::Molecule gfp_c=
     create_GFP(m,"Spc29p-C-GFP",i,CP_ps,CP_x0,mvs,mydata,SideXY,SideZ);
    all_mol.add_child(gfp_c);
   }
  }
 } // cycle on copies
//
// CNM67p-C
//
 if(mydata.protein_list["Cnm67p"]){
  for(int j=0;j<mydata.num_copies/2;++j){
   atom::Molecules Cnm67p;
   Cnm67p.push_back(create_protein(m,"Cnm67p","3OA7_A.pdb",
                     mydata.resolution,
                     display::Color(50./255.,205./255.,50./255.),
                     i,IL2_x0,0,false));
   Cnm67p.push_back(create_protein(m,"Cnm67p","3OA7_B.pdb",
                     mydata.resolution,
                     display::Color(50./255.,205./255.,50./255.),
                     i,IL2_x0,0,false));

   for(unsigned kk=0;kk<2;++kk){
    all_mol.add_child(Cnm67p[kk]);
    if(i==0){
     atom::Selection sel=atom::Selection(Cnm67p[kk]);
     Ints indexes;
     for(int i=429;i<=573;++i){indexes.push_back(i);}
     sel.set_residue_indexes(indexes);
     Particles ps=sel.get_selected_particles();
     IL2_ps->add_particles(ps);
    }
   }
 //GFPs?
   if(mydata.add_GFP){
    for(unsigned kk=0;kk<2;++kk){
     atom::Molecule gfp_c=
      create_GFP(m,"Cnm67p-C-GFP",i,IL2_ps,IL2_x0,mvs,mydata,SideXY,SideZ);
     all_mol.add_child(gfp_c);
    }
   }
// Create the rigid body
   core::XYZRs rbps;
   for(unsigned kk=0;kk<2;++kk){
    Particles ps=atom::get_leaves(Cnm67p[kk]);
    for(unsigned jj=0;jj<ps.size();++jj){rbps.push_back(core::XYZR(ps[jj]));}
   }
   IMP_NEW(Particle,prb,(m));
   core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
   recenter_rb(rb,rbps,IL2_x0,-1.0);
// Mover
   if(i==0){
    Particles ps=atom::get_leaves(Cnm67p[0]);
    add_PbcBoxedRigidBodyMover(ps,mydata.MC.dx,
     mydata.MC.dang,mydata.IL2_centers,mydata.trs,mvs,SideXY,SideZ);
   }
  }
 }
//
// SPC110p && CMD1p
//
 if(mydata.protein_list["Spc110p"] && mydata.protein_list["Cmd1p"]){
  for(int j=0;j<mydata.num_copies/2;++j){
   algebra::Vector3D tmp_x0=
    algebra::Vector3D(CP_x0[0],CP_x0[1],CP_x0[2]-mydata.CP_thicknessMin/2.0);

   // B) from residue 799 to 895 no structure
   atom::Molecules Spc110p_799_895;
   for(unsigned kk=0;kk<2;++kk){
    Spc110p_799_895.push_back(create_protein(m,"Spc110p_799_895",11.0,2,
                        display::Color(255./255.,0.,0.),
                        i,mydata.kappa,tmp_x0,mydata.use_connectivity,799,97));
    if(i==0){
     Particles ps=atom::get_leaves(Spc110p_799_895[kk]);
     CP_ps->add_particles(ps);
     add_BallMover(ps,mydata.MC.dx,mvs);
    }
   }
   // C) C-terminal part (with structure)
   atom::Molecules Spc110p_896_944;
   for(unsigned kk=0;kk<2;++kk){
    Spc110p_896_944.push_back(create_protein(m,"Spc110p_896_944",
                       "4DS7_Spc110_swapped.pdb", mydata.resolution,
                       display::Color(255./255.,0.,0.),
                       i,tmp_x0,0,false));
    if(i==0){
     Particles ps=atom::get_leaves(Spc110p_896_944[kk]);
     CP_ps->add_particles(ps);
    }
// GFPs?
    if(mydata.add_GFP){
     atom::Molecule gfp_c=
      create_GFP(m,"Spc110p-C-GFP",i,CP_ps,tmp_x0,mvs,mydata,SideXY,SideZ);
     all_mol.add_child(gfp_c);
    }
   }
// Calmodulin
   atom::Molecules Cmd1p;
   for(unsigned kk=0;kk<2;++kk){
    Cmd1p.push_back(create_protein(m,"Cmd1p","4DS7_Cmd1_swapped.pdb",
                     mydata.resolution,
                     display::Color(255./255.,255./255.,0.),
                     i,tmp_x0,0,false));

    all_mol.add_child(Cmd1p[kk]);
    if(i==0){
     Particles ps=atom::get_leaves(Cmd1p[kk]);
     CP_ps->add_particles(ps);
    }
 //GFPs?
    if(mydata.add_GFP){
      atom::Molecule gfp_n=
       create_GFP(m,"Cmd1p-N-GFP",i,CP_ps,tmp_x0,mvs,mydata,SideXY,SideZ);
      all_mol.add_child(gfp_n);
      atom::Molecule gfp_c=
       create_GFP(m,"Cmd1p-C-GFP",i,CP_ps,tmp_x0,mvs,mydata,SideXY,SideZ);
      all_mol.add_child(gfp_c);
    }
   }
// Create the two rigid bodies Cmd1-Spc110
   for(unsigned kk=0;kk<2;++kk){
    core::XYZRs rbps;
    Particles ps0=atom::get_leaves(Spc110p_896_944[kk]);
    for(unsigned jj=0;jj<ps0.size();++jj){rbps.push_back(core::XYZR(ps0[jj]));}
    Particles ps1=atom::get_leaves(Cmd1p[kk]);
    for(unsigned jj=0;jj<ps1.size();++jj){rbps.push_back(core::XYZR(ps1[jj]));}
    IMP_NEW(Particle,prb,(m));
    core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
    recenter_rb(rb,rbps,tmp_x0);
//  add mover for the rigid body
    if(i==0){
      IMP_NEW(core::RigidBodyMover,rbmv,(rb,mydata.MC.dx,mydata.MC.dang));
      mvs.push_back(rbmv);
    }
   }
// Finally create the Coiled-Coil up to 798
   atom::Molecules Spc110p_CC=
    create_coiled_coil(m,"Spc110_CC","CC_120_A.pdb","CC_120_B.pdb",
                       mydata.resolution,
                       display::Color(255./255.,0.,0.),
                       i, CC2_x0, 679);
// add mover for a master rigid body with slave particles
   if(i==0){
    Particles ps;
    for(unsigned int kk=0;kk<2;++kk){
     Particles ps0=atom::get_leaves(Spc110p_799_895[kk]);
     Particles ps1=atom::get_leaves(Spc110p_896_944[kk]);
     Particles ps2=atom::get_leaves(Cmd1p[kk]);
     ps.insert(ps.end(),ps0.begin(),ps0.end());
     ps.insert(ps.end(),ps1.begin(),ps1.end());
     ps.insert(ps.end(),ps2.begin(),ps2.end());
    }
    core::RigidBody rb=
      core::RigidMember(atom::get_leaves(Spc110p_CC[0])[0]).get_rigid_body();
    IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
     (rb,ps,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs,
      SideXY,SideXY,SideZ));
    mvs.push_back(rbmv);
   }
// only at this point I can create the merged Spc110
   for(unsigned int kk=0;kk<2;++kk){
    atom::Molecules Spc110p_c_all;
    Spc110p_c_all.push_back(Spc110p_CC[kk]);
    Spc110p_c_all.push_back(Spc110p_799_895[kk]);
    Spc110p_c_all.push_back(Spc110p_896_944[kk]);
    atom::Molecule Spc110p=
     create_merged_protein(m,"Spc110p",Spc110p_c_all,i,mydata.kappa,0.0);
    all_mol.add_child(Spc110p);
   }
  }
 }

// set all particles as not optimized
 Particles ps=atom::get_leaves(all_mol);
 for(unsigned i=0;i<ps.size();++i){
  core::XYZR(ps[i]).set_coordinates_are_optimized(true);
 }
// add the hierarchy of the current unit cell to hs
 hs.push_back(all_mol);
} // cycle on cells

return hs;
}

atom::Molecule create_protein(Model *m,std::string name,double mass,
int nbeads, display::Color colore,int copy,double kappa,
 algebra::Vector3D x0, bool use_connectivity, int start_residue, int nres)
{
 if(nres==-1) {nres=(int) (mass*1000.0/110.0);}
 IMP_NEW(Particle,p,(m));
 atom::Molecule protein=atom::Molecule::setup_particle(p);
 protein->set_name(name);
 Ints nres_bead;
 for(int i=0;i<nbeads;++i){
  nres_bead.push_back(nres/nbeads);
  if(i<nres%nbeads){++nres_bead[i];}
 }
 int ires=0;
 for(int i=0;i<nbeads;++i){
  IMP_NEW(Particle,pp,(m));
  int first=start_residue+ires;
  int last=first+nres_bead[i];
  ires+=nres_bead[i];
  std::stringstream out1,out2;
  out1 << i;
  out2 << copy;
  atom::Domain dom=atom::Domain::setup_particle(pp, IntRange(first, last));
  dom->set_name(name+out1.str()+"-"+out2.str());
  core::XYZR  d=core::XYZR::setup_particle(pp);
  double ms=110.0*(double) nres_bead[i];
  double vol=atom::get_volume_from_mass(ms);
  double rg=algebra::get_ball_radius_from_volume_3d(vol);
  d.set_radius(rg);
  d.set_coordinates(x0);
  d.set_coordinates_are_optimized(true);
  atom::Mass mm=atom::Mass::setup_particle(pp,ms);
  display::Colored cc=display::Colored::setup_particle(pp,colore);
  protein.add_child(dom);
 }
 if(nbeads>1 && copy==0){
// ORGINAL IMP CONNECTIVITY
  if(use_connectivity){
   atom::Selections ss=atom::Selections();
   atom::Hierarchies hs=protein.get_children();
   for(unsigned int i=0;i<hs.size();++i){
    ss.push_back(atom::Selection(hs[i]));
   }
   Restraint *con=atom::create_connectivity_restraint(ss,kappa);
   con->set_name("Connectivity Restraint for "+name);
   m->add_restraint(con);
  }else{
// SIMPLER CONNECTIVITY
   add_my_connectivity(m,name,protein,kappa);
  }
 }
 return protein;
}

atom::Molecule create_protein(Model *m,std::string name,
 std::string filename,int nres_per_bead,display::Color colore,
 int copy, algebra::Vector3D x0, int offset, bool makerigid)
{
 IMP_NEW(Particle,p,(m));
 atom::Molecule protein=atom::Molecule::setup_particle(p);
 protein->set_name(name);
 IMP_NEW(atom::CAlphaPDBSelector,sel,());
 atom::Hierarchy hpdb=atom::read_pdb(filename,m,sel);
 Particles ps=atom::get_leaves(hpdb);
 int nres=ps.size();
 int nbeads=(int) round((double)nres/(double)nres_per_bead);
 Ints nres_bead;
 for(int i=0;i<nbeads;++i){
  nres_bead.push_back(nres/nbeads);
  if(i<nres%nbeads){++nres_bead[i];}
 }
 int ires=0;
 core::XYZRs rbps;
 for(int i=0;i<nbeads;++i){
  double ms=0.0;
  core::XYZRs xyz;
  int first,last;
  for(int j=ires;j<ires+nres_bead[i];++j){
   atom::Residue res=atom::Residue(atom::Atom(ps[j]).get_parent());
   atom::ResidueType restype=res.get_residue_type();
   if(j==ires) {first=res.get_index()+offset;}
   if(j==ires+nres_bead[i]-1) {last=res.get_index()+1+offset;}
   double vol=atom::get_volume_from_residue_type(restype);
   double rg=algebra::get_ball_radius_from_volume_3d(vol);
   core::XYZR(ps[j]).set_radius(rg);
   xyz.push_back(core::XYZR(ps[j]));
   ms+=atom::Mass(ps[j]).get_mass();
  }
  ires+=nres_bead[i];
  IMP_NEW(Particle,pp,(m));
  std::stringstream out1,out2;
  out1 << i;
  out2 << copy;
  atom::Domain dom=atom::Domain::setup_particle(pp, IntRange(first, last));
  dom->set_name(name+out1.str()+"-"+out2.str());
  core::XYZR  d=core::XYZR::setup_particle(pp);
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
 if(makerigid){
  IMP_NEW(Particle,prb,(m));
  core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
  rb->set_name(name);
  recenter_rb(rb,rbps,x0);
 }
 return protein;
}

void recenter_rb(core::RigidBody& rb, core::XYZRs& rbps,
 algebra::Vector3D x0, double flip)
{
  int size=rbps.size();
  double bb = (core::RigidMember(rbps[0]).get_internal_coordinates())[0];
  double ee = (core::RigidMember(rbps[size-1]).get_internal_coordinates())[0];
  if (flip*(ee-bb)<0.0){
   for(int k=0;k<size;++k){
    algebra::Vector3D coord=
    core::RigidMember(rbps[k]).get_internal_coordinates();
    algebra::Rotation3D rot=
     algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),
IMP::algebra::PI);
    algebra::Transformation3D tr=
     algebra::Transformation3D(rot,algebra::Vector3D(0,0,0));
    core::RigidMember(rbps[k]).set_internal_coordinates
     (tr.get_transformed(coord));
   }
  }
  rb.set_reference_frame(algebra::ReferenceFrame3D(algebra::Transformation3D
      (algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),
-IMP::algebra::PI/2.0),
       x0)));
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

atom::Molecules create_coiled_coil (Model *m,
 std::string name,std::string filename_A,std::string filename_B,
 int nbeads,display::Color colore,int copy,algebra::Vector3D x0,int offset)
{
 atom::Molecules coils;

 coils.push_back(create_protein(m,name,filename_A,
  nbeads,colore,copy,x0,offset,false));

 coils.push_back(create_protein(m,name,filename_B,
  nbeads,colore,copy,x0,offset,false));

 core::XYZRs rbps;
 for(unsigned int i=0;i<2;++i){
  Particles ps=atom::get_leaves(coils[i]);
  for(unsigned int j=0;j<ps.size();++j){rbps.push_back(core::XYZR(ps[j]));}
 }
 IMP_NEW(Particle,prb,(m));
 core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
 rb->set_name(name);
 // Check orientation of x-axis and topology
 recenter_rb(rb,rbps,x0);
 return coils;
}

atom::Molecule create_GFP(Model *m, std::string name, int copy,
 container::ListSingletonContainer *lsc, algebra::Vector3D x0,
 core::Movers& mvs, SPBParameters mydata,
 Particle *SideXY, Particle *SideZ)
{
 if(!mydata.use_GFP_structure){
  IMP_NEW(Particle,p,(m));
  atom::Molecule gfp=atom::Molecule::setup_particle(p);
  gfp->set_name(name);
  const int nbeads=3;
  const int nres_bead=50;
  const double ms=27.0*1000.0/(double) nbeads;
  const double rg=15.0;
  const display::Color colore=display::Color(124./255.,252./255.,0./255.);
  core::XYZRs rbps;
  for(int i=0;i<nbeads;++i){
   IMP_NEW(Particle,pp,(m));
   int first=i*nres_bead;
   int last=(i+1)*nres_bead;
   std::stringstream out1,out2;
   out1 << i;
   out2 << copy;
   atom::Domain dom=atom::Domain::setup_particle(pp, IntRange(first, last));
   dom->set_name(name+out1.str()+"-"+out2.str());
   core::XYZR d=core::XYZR::setup_particle(pp);
   d.set_radius(rg);
   algebra::Vector3D x0i=
    algebra::Vector3D(x0[0],x0[1],x0[2]+5.0*(double) (i-1));
   d.set_coordinates(x0i);
   d.set_coordinates_are_optimized(true);
   atom::Mass mm=atom::Mass::setup_particle(pp,ms);
   display::Colored cc=display::Colored::setup_particle(pp,colore);
   gfp.add_child(dom);
   rbps.push_back(d);
  }
  IMP_NEW(Particle,prb,(m));
  core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
  rb->set_name(name);
  if(copy==0){
   Particles ps_gfp=atom::get_leaves(gfp);
   if(mydata.keep_GFP_layer) {lsc->add_particles(ps_gfp);}
   if(!mydata.fix_GFP){add_PbcBoxedRigidBodyMover(ps_gfp,mydata.MC.dx,
      mydata.MC.dang,mydata.IL2_centers,mydata.trs,mvs,SideXY,SideZ);}
  }
  return gfp;
 } else {
  atom::Molecule gfp=
   create_protein(m,name,"1EMA.pdb",mydata.resolution,
                    display::Color(124./255.,252./255.,0./255.),
                    copy,x0);
  if(copy==0){
   Particles ps_gfp=atom::get_leaves(gfp);
   if(mydata.keep_GFP_layer) {lsc->add_particles(ps_gfp);}
   if(!mydata.fix_GFP){add_PbcBoxedRigidBodyMover(ps_gfp,mydata.MC.dx,
      mydata.MC.dang,mydata.IL2_centers,mydata.trs,mvs,SideXY,SideZ);}
  }
  return gfp;
 }
}

void load_restart(atom::Hierarchies& all_mol,SPBParameters mydata)
{
 std::map<std::string,std::string>::iterator it;
 std::list<std::string>::iterator iit;
 std::list<std::string> file_list;

 for (it = mydata.file_list.begin(); it != mydata.file_list.end(); it++){
  file_list.push_back((*it).second);
 }
// eliminate duplicates from file_list
 file_list.sort();
 file_list.unique();
// now cycle on file list
 for (iit = file_list.begin(); iit != file_list.end(); iit++){
  RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(*iit);
  atom::Hierarchies hs;
  for(unsigned int i=0;i<all_mol.size();++i){
   atom::Hierarchies hhs=all_mol[i].get_children();
   for(unsigned int j=0;j<hhs.size();++j){
    if(mydata.file_list[hhs[j]->get_name()]==*iit){
     hs.push_back(hhs[j]);
    }
   }
  }
  rmf::link_hierarchies(rh, hs);
// reload last frame
  unsigned int iframe=rh.get_number_of_frames();
  rmf::load_frame(rh,iframe-1);
 }
}

IMPMEMBRANE_END_NAMESPACE
