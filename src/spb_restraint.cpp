/**
 *  \file spb_restraint.cpp
 *  \brief SPB Restraints
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/membrane.h>
#include <string>
using namespace IMP;

IMPMEMBRANE_BEGIN_NAMESPACE

core::DistancePairScore* get_pair_score(FloatRange dist)
{
 IMP_NEW(core::HarmonicWell,hw,(dist,1.0));
 IMP_NEW(core::DistancePairScore,ps,(hw));
 return ps.release();
}

core::SphereDistancePairScore* get_sphere_pair_score(FloatRange dist)
{
 IMP_NEW(core::HarmonicWell,hw,(dist,1.0));
 IMP_NEW(core::SphereDistancePairScore,ps,(hw));
 return ps.release();
}

void add_internal_restraint(Model *m,std::string name,
atom::Molecule protein_a,atom::Molecule protein_b,double dist)
{
 core::DistancePairScore* ps=get_pair_score(FloatRange(-500.0, dist));
 atom::Selection sa=atom::Selection(protein_a);
 atom::Selection sb=atom::Selection(protein_b);
 sa.set_terminus(atom::Selection::C);
 sb.set_terminus(atom::Selection::N);
 Particle*  pa=sa.get_selected_particles()[0];
 Particle*  pb=sb.get_selected_particles()[0];
 IMP_NEW(core::PairRestraint,r,(ps,ParticlePair(pa, pb)));
 r->set_name("IR " + name);
 m->add_restraint(r);
}

FloatRange get_range_from_fret_class(std::string r_class)
{
 FloatRange range;
 if (r_class=="High")   {range=FloatRange(-500.0,  41.0);}
 if (r_class=="Mod")    {range=FloatRange(41.0, 55.5);}
 if (r_class=="Low")    {range=FloatRange(55.5, 66.0);}
 if (r_class=="Lowest") {range=FloatRange(66.0, 70.0);}
 if (r_class=="None")   {range=FloatRange(70.0, 100000.0);}
 return range;
}

FloatRange get_range_from_fret_value(double r_value)
{
 std::string r_class;
 if (r_value >= 2.0)                   {r_class="High";}
 if (r_value >= 1.5 && r_value < 2.0)  {r_class="Mod";}
 if (r_value >= 1.2 && r_value < 1.5)  {r_class="Low";}
 if (r_value >= 1.05 && r_value < 1.2) {r_class="Lowest";}
 if (r_value <  1.05)                  {r_class="None";}
 return get_range_from_fret_class(r_class);
}

void do_bipartite_mindist(Model *m,Particles p1,Particles p2,
 core::DistancePairScore* dps,bool filter)
{
 IMP_NEW(container::ListSingletonContainer,lp1,(p1));
 IMP_NEW(container::ListSingletonContainer,lp2,(p2));
 IMP_NEW(container::AllBipartitePairContainer,pc,(lp1,lp2));
 if(filter){
  //f=IMP.atom.SameResiduePairFilter()
  //pc.add_pair_filter(f)
 }
 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,pc,1));
 m->add_restraint(mpr);
}

void do_allpairs_mindist(Model *m,Particles ps,
 core::DistancePairScore* dps,bool filter)
{
 IMP_NEW(container::ListSingletonContainer,lp,(ps));
 IMP_NEW(container::AllPairContainer,pc,(lp));
 if(filter){
  //f=IMP.atom.SameResiduePairFilter()
  //pc.add_pair_filter(f)
 }
 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,pc,1));
 m->add_restraint(mpr);
}

void do_bipartite_mindist(Model *m,Particles p1,Particles p2,
 core::SphereDistancePairScore* dps,bool filter)
{
 IMP_NEW(container::ListSingletonContainer,lp1,(p1));
 IMP_NEW(container::ListSingletonContainer,lp2,(p2));
 IMP_NEW(container::AllBipartitePairContainer,pc,(lp1,lp2));
 if(filter){
  //f=IMP.atom.SameResiduePairFilter()
  //pc.add_pair_filter(f)
 }
 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,pc,1));
 m->add_restraint(mpr);
}

void do_allpairs_mindist(Model *m,Particles ps,
 core::SphereDistancePairScore* dps,bool filter)
{
 IMP_NEW(container::ListSingletonContainer,lp,(ps));
 IMP_NEW(container::AllPairContainer,pc,(lp));
 if(filter){
  //f=IMP.atom.SameResiduePairFilter()
  //pc.add_pair_filter(f)
 }
 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,pc,1));
 m->add_restraint(mpr);
}

void add_fret_restraint
(Model *m,atom::Hierarchies ha,std::string protein_a,std::string residues_a,
 atom::Hierarchies hb, std::string protein_b, std::string residues_b,
 double r_value)
{
 atom::Selection sa=atom::Selection(ha);
 sa.set_molecule(protein_a);
 if(residues_a=="C") {sa.set_terminus(atom::Selection::C);}
 if(residues_a=="N") {sa.set_terminus(atom::Selection::N);}
 atom::Selection sb=atom::Selection(hb);
 sb.set_molecule(protein_b);
 if(residues_b=="C") {sb.set_terminus(atom::Selection::C);}
 if(residues_b=="N") {sb.set_terminus(atom::Selection::N);}
 Particles p1=sa.get_selected_particles();
 Particles p2=sb.get_selected_particles();
 FloatRange range=get_range_from_fret_value(r_value);
 core::DistancePairScore* ps=get_pair_score(range);
 if(protein_a != protein_b){
  do_bipartite_mindist(m,p1,p2,ps);
 }else if(protein_a==protein_b && residues_a==residues_b){
  do_allpairs_mindist(m,p1,ps);
 }else if(protein_a==protein_b && residues_a!=residues_b){
  do_bipartite_mindist(m,p1,p2,ps);
 }
}

void add_y2h_restraint
(Model *m,atom::Hierarchies ha,std::string protein_a,IntRange residues_a,
 atom::Hierarchies hb,std::string protein_b,IntRange residues_b)
{
 atom::Selection sa=atom::Selection(ha);
 sa.set_molecule(protein_a);
 Ints r_a;
 for(int i=residues_a.first;i<=residues_a.second;++i) r_a.push_back(i);
 sa.set_residue_indexes(r_a);
 atom::Selection sb=atom::Selection(hb);
 sb.set_molecule(protein_b);
 Ints r_b;
 for(int i=residues_b.first;i<=residues_b.second;++i) r_b.push_back(i);
 sb.set_residue_indexes(r_b);
 Particles p1=sa.get_selected_particles();
 Particles p2=sb.get_selected_particles();
 FloatRange range=FloatRange(-500,0.0);
 core::SphereDistancePairScore* ps=get_sphere_pair_score(range);
 if(protein_a != protein_b){
  do_bipartite_mindist(m,p1,p2,ps);
 }else if(protein_a==protein_b && residues_a==residues_b){
  do_allpairs_mindist(m,p1,ps);
 }else if(protein_a==protein_b && residues_a!=residues_b){
  do_bipartite_mindist(m,p1,p2,ps);
 }
}

void add_y2h_restraint
(Model *m,atom::Hierarchies ha,std::string protein_a,std::string residues_a,
 atom::Hierarchies hb, std::string protein_b, std::string residues_b)
{
 atom::Selection sa=atom::Selection(ha);
 sa.set_molecule(protein_a);
 if(residues_a=="C") {sa.set_terminus(atom::Selection::C);}
 if(residues_a=="N") {sa.set_terminus(atom::Selection::N);}
 atom::Selection sb=atom::Selection(hb);
 sb.set_molecule(protein_b);
 if(residues_b=="C") {sb.set_terminus(atom::Selection::C);}
 if(residues_b=="N") {sb.set_terminus(atom::Selection::N);}
 Particles p1=sa.get_selected_particles();
 Particles p2=sb.get_selected_particles();
 FloatRange range=FloatRange(-500,0.0);
 core::SphereDistancePairScore* ps=get_sphere_pair_score(range);
 if(protein_a != protein_b){
  do_bipartite_mindist(m,p1,p2,ps);
 }else if(protein_a==protein_b && residues_a==residues_b){
  do_allpairs_mindist(m,p1,ps);
 }else if(protein_a==protein_b && residues_a!=residues_b){
  do_bipartite_mindist(m,p1,p2,ps);
 }
}

void add_y2h_restraint
(Model *m,atom::Hierarchies ha,std::string protein_a,IntRange residues_a,
 atom::Hierarchies hb,std::string protein_b,std::string residues_b)
{
 atom::Selection sa=atom::Selection(ha);
 sa.set_molecule(protein_a);
 Ints r_a;
 for(int i=residues_a.first;i<=residues_a.second;++i) r_a.push_back(i);
 sa.set_residue_indexes(r_a);
 atom::Selection sb=atom::Selection(hb);
 sb.set_molecule(protein_b);
 if(residues_b=="C") {sb.set_terminus(atom::Selection::C);}
 if(residues_b=="N") {sb.set_terminus(atom::Selection::N);}
 Particles p1=sa.get_selected_particles();
 Particles p2=sb.get_selected_particles();
 FloatRange range=FloatRange(-500,0.0);
 core::SphereDistancePairScore* ps=get_sphere_pair_score(range);
 do_bipartite_mindist(m,p1,p2,ps);
}

void add_y2h_restraint
(Model *m,atom::Hierarchies ha,std::string protein_a,std::string residues_a,
 atom::Hierarchies hb,std::string protein_b,IntRange residues_b)
{
 add_y2h_restraint(m,hb,protein_b,residues_b,ha,protein_a,residues_a);
}

void add_symmetry_restraint
 (Model *m,atom::Hierarchies hs,SPBParameters myparam)
{
 algebra::Vector3Ds translations=algebra::Vector3Ds();
 algebra::Transformation3Ds transformations=
 algebra::Transformation3Ds();
 int num_rotations;
 double angle;
// save parameters in local variables
 double side=myparam.side;
 std::string cell_type=myparam.cell_type;
 int num_cells=myparam.num_cells;

 if(cell_type!="square"){
  translations.push_back(algebra::Vector3D(0.0,0.0,0.0));
  translations.push_back(algebra::Vector3D(0.0,side*sqrt(3.0),0.0));
  translations.push_back(algebra::Vector3D(1.5*side,side*sqrt(3.0)/2.0,0.0));
  translations.push_back(algebra::Vector3D(1.5*side,-side*sqrt(3.0)/2.0,0.0));
  translations.push_back(algebra::Vector3D(0.0,-side*sqrt(3.0),0.0));
  translations.push_back(algebra::Vector3D(-1.5*side,-side*sqrt(3.0)/2.0,0.0));
  translations.push_back(algebra::Vector3D(-1.5*side,side*sqrt(3.0)/2.0,0.0));
  num_rotations=num_cells/7;
  angle=2.0*IMP::PI/(double)num_rotations;
 }else{
  translations.push_back(algebra::Vector3D(0.0,0.0,0.0));
  translations.push_back(algebra::Vector3D(0.0,side,0.0));
  translations.push_back(algebra::Vector3D(0.0,-side,0.0));
  translations.push_back(algebra::Vector3D(side,0.0,0.0));
  translations.push_back(algebra::Vector3D(side,side,0.0));
  translations.push_back(algebra::Vector3D(side,-side,0.0));
  translations.push_back(algebra::Vector3D(-side,0.0,0.0));
  translations.push_back(algebra::Vector3D(-side,side,0.0));
  translations.push_back(algebra::Vector3D(-side,-side,0.0));
  num_rotations=1;
  angle=0.0;
 }
 for(int i=0;i<translations.size();++i){
  for(int j=0;j<num_rotations;++j){
   algebra::Rotation3D rot=algebra::get_rotation_about_axis
   (algebra::Vector3D(0.0,0.0,1.0), (double) j * angle);
   transformations.push_back(algebra::Transformation3D(rot,translations[i]));
  }
 }
 Particles ps0=hs[0].get_leaves();
 for(int i=1;i<num_cells;++i){
  IMP_NEW(core::TransformationSymmetry,sm,(transformations[i]));
  Particles ps1=hs[i].get_leaves();
  for(int j=0;j<ps1.size();++j){
   core::Reference::setup_particle(ps1[j],ps0[j]);
  }
  IMP_NEW(container::ListSingletonContainer,lc,(ps1));
  IMP_NEW(container::SingletonsConstraint,c,(sm,NULL,lc));
  m->add_score_state(c);
 }
}

IMPMEMBRANE_END_NAMESPACE
