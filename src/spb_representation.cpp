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
container::ListSingletonContainer *bCP_ps,
container::ListSingletonContainer *CP_ps,
container::ListSingletonContainer *IL2_ps,
core::Movers& mvs)
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
 algebra::Vector3D CP_x0=mydata.CP_centers[i];
 algebra::Vector3D IL2_x0=mydata.IL2_centers[i];
 double ddz=mydata.CP_thickness/2.0+mydata.CP_IL2_gap/2.0;
 algebra::Vector3D CC_x0=algebra::Vector3D(CP_x0[0],CP_x0[1],ddz);
 algebra::Vector3D CC2_x0=
  algebra::Vector3D(CP_x0[0],CP_x0[1],-mydata.CP_thickness);

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
                       i,mydata.kappa,CP_x0));
    if(i==0){
     Particles ps_Spc42p_n=atom::get_leaves(Spc42p_n[k]);
     CP_ps->add_particles(ps_Spc42p_n);
     add_BallMover(ps_Spc42p_n,mydata.MC.dx,mvs);
    }
  //Spc42p_c, 2 copies, 2 beads
    Spc42p_c.push_back(create_protein(m,"Spc42p_c",13,2,
                       display::Color(175./255.,218./255.,238./255.),
                       i,mydata.kappa,IL2_x0,139));
    if(i==0){
     Particles ps_Spc42p_c=atom::get_leaves(Spc42p_c[k]);
     IL2_ps->add_particles(ps_Spc42p_c);
     add_BallMover(ps_Spc42p_c,mydata.MC.dx,mvs);
    }
   }
   // Coiled-Coil
   atom::Molecules Spc42p_CC=
    create_coiled_coil(m,"Spc42_CC","2Q6Q_A.pdb", "2Q6Q_B.pdb",
                       mydata.resolution,
                       display::Color(175./255.,208./255.,238./255.),
                       i, CC_x0);
   if(i==0){
    Particles ps_Spc42p;
    for(unsigned int k=0;k<2;++k){
     Particles ps_Spc42p_n=atom::get_leaves(Spc42p_n[k]);
     Particles ps_Spc42p_c=atom::get_leaves(Spc42p_c[k]);
     ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_n.begin(),ps_Spc42p_n.end());
     ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_c.begin(),ps_Spc42p_c.end());
    }
    core::RigidBody prb=
     core::RigidMember(atom::get_leaves(Spc42p_CC[0])[0]).get_rigid_body();
    IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
     (prb,ps_Spc42p,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs));
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
      create_GFP(m,"Spc42p-N-GFP",i,CP_ps,CP_x0,mvs,mydata);
     all_mol.add_child(gfp_n);
     atom::Molecule gfp_c=
      create_GFP(m,"Spc42p-C-GFP",i,IL2_ps,IL2_x0,mvs,mydata);
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
                        i,mydata.kappa,CP_x0));
   Spc29p_all.push_back(create_protein(m,"Spc29p_c",14.5,2,
                        display::Color(255./255.,140./255.,0.),
                        i,mydata.kappa,CP_x0,132));
   atom::Molecule Spc29p=
    create_merged_protein(m,"Spc29p",Spc29p_all,i,mydata.kappa,0.0);
   all_mol.add_child(Spc29p);
   if(i==0){
    Particles ps_Spc29p=atom::get_leaves(Spc29p);
    CP_ps->add_particles(ps_Spc29p);
    add_PbcBoxedMover(ps_Spc29p,mydata.MC.dx,mydata.CP_centers,mydata.trs,mvs);
   }
   if(mydata.add_GFP){
    atom::Molecule gfp_n=
     create_GFP(m,"Spc29p-N-GFP",i,CP_ps,CP_x0,mvs,mydata);
    all_mol.add_child(gfp_n);
    atom::Molecule gfp_c=
     create_GFP(m,"Spc29p-C-GFP",i,CP_ps,CP_x0,mvs,mydata);
    all_mol.add_child(gfp_c);
   }
  }
//
// CMD1P
//
  if(mydata.protein_list["Cmd1p"]){
 //Cmd1p, 1 bead for N, 1 bead for C
   if(!mydata.use_structure){
    atom::Molecules Cmd1p_all;
    Cmd1p_all.push_back(create_protein(m,"Cmd1p_n",8,1,
                        display::Color(255./255.,255./255.,0.),
                        i,mydata.kappa,CP_x0));
    Cmd1p_all.push_back(create_protein(m,"Cmd1p_c",8,1,
                        display::Color(255./255.,215./255.,0.),
                        i,mydata.kappa,CP_x0,80));
    atom::Molecule Cmd1p=
     create_merged_protein(m,"Cmd1p",Cmd1p_all,i,mydata.kappa,0.0);
    all_mol.add_child(Cmd1p);
    if(i==0){
     Particles ps_Cmd1p=atom::get_leaves(Cmd1p);
     CP_ps->add_particles(ps_Cmd1p);
     add_PbcBoxedMover(ps_Cmd1p,mydata.MC.dx,mydata.CP_centers,mydata.trs,mvs);
    }
   }else{
    std::string filename;
    if(mydata.use_compact_Cmd1p){
     filename="1PRW.pdb";
    } else {
     filename="3CLN.pdb";
    }
    atom::Molecule Cmd1p=
     create_protein(m,"Cmd1p",filename,mydata.resolution,
                     display::Color(255./255.,255./255.,0.),
                     i,CP_x0);
    all_mol.add_child(Cmd1p);
    if(i==0){
     Particles ps_Cmd1p=atom::get_leaves(Cmd1p);
     CP_ps->add_particles(ps_Cmd1p);
     add_PbcBoxedRigidBodyMover(ps_Cmd1p,mydata.MC.dx,
      mydata.MC.dang,mydata.CP_centers,mydata.trs,mvs);
    }
   }
   if(mydata.add_GFP){
    atom::Molecule gfp_n=
     create_GFP(m,"Cmd1p-N-GFP",i,CP_ps,CP_x0,mvs,mydata);
    all_mol.add_child(gfp_n);
    atom::Molecule gfp_c=
     create_GFP(m,"Cmd1p-C-GFP",i,CP_ps,CP_x0,mvs,mydata);
    all_mol.add_child(gfp_c);
   }
  }
//
// CNM67p-C
//
  if(mydata.protein_list["Cnm67p_c"]){
 //Cnm67p_c, 2 beads
   if(!mydata.use_structure){
    atom::Molecule Cnm67p_c=
     create_protein(m,"Cnm67p_c",15,2,
                      display::Color(50./255.,205./255.,50./255.),
                      i,mydata.kappa,IL2_x0,429);
    all_mol.add_child(Cnm67p_c);
    if(i==0){
     Particles ps_Cnm67p_c=atom::get_leaves(Cnm67p_c);
     IL2_ps->add_particles(ps_Cnm67p_c);
     add_PbcBoxedMover(ps_Cnm67p_c,mydata.MC.dx,
      mydata.IL2_centers,mydata.trs,mvs);
    }
   } else {
    atom::Molecule Cnm67p_c=
     create_protein(m,"Cnm67p_c","3OA7.pdb",mydata.resolution,
                      display::Color(50./255.,205./255.,50./255.),
                      i,IL2_x0);
    all_mol.add_child(Cnm67p_c);
    if(i==0){
     Particles ps_Cnm67p_c=atom::get_leaves(Cnm67p_c);
     IL2_ps->add_particles(ps_Cnm67p_c);
     add_PbcBoxedRigidBodyMover(ps_Cnm67p_c,mydata.MC.dx,
      mydata.MC.dang,mydata.IL2_centers,mydata.trs,mvs);
    }
   }
   if(mydata.add_GFP){
    atom::Molecule gfp_c=
     create_GFP(m,"Cnm67p_c-C-GFP",i,IL2_ps,IL2_x0,mvs,mydata);
    all_mol.add_child(gfp_c);
   }
  }
 } // cycle on copies
//
// SPC110p
//
 if(mydata.protein_list["Spc110p"]){
  for(int j=0;j<mydata.num_copies/2;++j){
 //Spc110p_c, 4 beads for C terminus
   atom::Molecules Spc110p_c;
   for(unsigned int kk=0;kk<2;++kk){
    Spc110p_c.push_back(create_protein(m,"Spc110p_c",26,4,
                        display::Color(255./255.,0.,0.),
                        i,mydata.kappa,CP_x0,799));
    if(i==0){
     Particles ps_Spc110p_c=atom::get_leaves(Spc110p_c[kk]);
     CP_ps->add_particles(ps_Spc110p_c);
     add_BallMover(ps_Spc110p_c,mydata.MC.dx,mvs);
    }
   }
   // Coiled-Coil
   atom::Molecules Spc110p_CC=
    create_coiled_coil(m,"Spc110_CC","CC_120_A.pdb","CC_120_B.pdb",
                       mydata.resolution,
                       display::Color(255./255.,0.,0.),
                       i, CC2_x0,678);
   if(i==0){
    Particles ps_Spc110p;
    for(unsigned int k=0;k<2;++k){
     bCP_ps->add_particles(atom::get_leaves(Spc110p_CC[k]));
     Particles ps_Spc110p_c=atom::get_leaves(Spc110p_c[k]);
     ps_Spc110p.insert(ps_Spc110p.end(),
     ps_Spc110p_c.begin(),ps_Spc110p_c.end());
    }
    core::RigidBody prb=
     core::RigidMember(atom::get_leaves(Spc110p_CC[0])[0]).get_rigid_body();
    IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
     (prb,ps_Spc110p,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs));
    mvs.push_back(rbmv);
   }
// now create the merge
   for(unsigned int k=0;k<2;++k){
    atom::Molecules Spc110p_all;
    Spc110p_all.push_back(Spc110p_CC[k]);
    Spc110p_all.push_back(Spc110p_c[k]);
    atom::Molecule Spc110p=
      create_merged_protein(m,"Spc110p",Spc110p_all,i,mydata.kappa,0.0);
    all_mol.add_child(Spc110p);
   }
   if(mydata.add_GFP){
    for(unsigned k=0;k<2;++k){
     atom::Molecule gfp_c=
      create_GFP(m,"Spc110p-C-GFP",i,CP_ps,CP_x0,mvs,mydata);
     all_mol.add_child(gfp_c);
    }
   }
  }
 }

// add the hierarchy of the current unit cell to hs
 hs.push_back(all_mol);
} // cycle on cells

return hs;
}

atom::Molecule create_protein(Model *m,std::string name,double mass,
int nbeads, display::Color colore,int copy,double kappa,
 algebra::Vector3D x0, int start_residue, int nres)
{
 if(nres==-1) {nres=(int) (mass*1000.0/110.0);}
 IMP_NEW(Particle,p,(m));
 atom::Molecule protein=atom::Molecule::setup_particle(p);
 protein->set_name(name);
 Ints nres_bead;
 for(unsigned int i=0;i<nbeads;++i){
  nres_bead.push_back(nres/nbeads);
  if(i<nres%nbeads){++nres_bead[i];}
 }
 int ires=0;
 for(unsigned int i=0;i<nbeads;++i){
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
 std::string filename,int nres_per_bead,display::Color colore,
 int copy,algebra::Vector3D x0,int offset,bool recenter)
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
 for(unsigned int i=0;i<nbeads;++i){
  nres_bead.push_back(nres/nbeads);
  if(i<nres%nbeads){++nres_bead[i];}
 }
 int ires=0;
 core::XYZRs rbps;
 for(unsigned int i=0;i<nbeads;++i){
  double ms=0.0;
  core::XYZRs xyz;
  int first,last;
  for(unsigned int j=ires;j<ires+nres_bead[i];++j){
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
 IMP_NEW(Particle,prb,(m));
 core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
 rb->set_name(name);
 if(recenter){recenter_rb(rb,rbps,x0);}
 return protein;
}

void recenter_rb(core::RigidBody& rb,core::XYZRs& rbps,algebra::Vector3D x0)
{
  int size=rbps.size();
  double bb = (core::RigidMember(rbps[0]).get_internal_coordinates())[0];
  double ee = (core::RigidMember(rbps[size-1]).get_internal_coordinates())[0];
  if (ee-bb<0.0){
   for(unsigned int k=0;k<size;++k){
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

// now I need to destroy the two rigid bodies
 core::XYZRs rbps;
 for(unsigned int i=0;i<2;++i){
  Particles ps=atom::get_leaves(coils[i]);
  core::RigidBody::teardown_particle(core::RigidMember(ps[0]).get_rigid_body());
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
 core::Movers& mvs, SPBParameters mydata)
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
   add_PbcBoxedRigidBodyMover(ps_gfp,mydata.MC.dx,
      mydata.MC.dang,mydata.IL2_centers,mydata.trs,mvs);
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
   add_PbcBoxedRigidBodyMover(ps_gfp,mydata.MC.dx,
      mydata.MC.dang,mydata.IL2_centers,mydata.trs,mvs);
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
 file_list.unique();
// now cycle on file list
 for (iit = file_list.begin(); iit != file_list.end(); iit++){
  RMF::RootHandle rh = RMF::open_rmf_file(*iit);
  atom::Hierarchies hs;
  for(unsigned int i=0;i<all_mol.size();++i){
   atom::Hierarchies hhs=all_mol[i].get_children();
   for(unsigned int j=0;j<hhs.size();++j){
    if(mydata.file_list[hhs[j]->get_name()]==*iit){
     hs.push_back(hhs[j]);
    }
   }
  }
  rmf::set_hierarchies(rh, hs);
// reload last frame
  for(unsigned int i=0;i<hs.size();++i){
   unsigned int iframe=rmf::get_number_of_frames(rh,hs[i]);
   rmf::load_frame(rh,iframe-1,hs[i]);
  }
 }

}

IMPMEMBRANE_END_NAMESPACE
