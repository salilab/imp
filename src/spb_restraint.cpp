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
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <list>

using namespace IMP;

IMPMEMBRANE_BEGIN_NAMESPACE

Pointer<core::DistancePairScore>
 get_pair_score(FloatRange dist, double kappa)
{
 IMP_NEW(core::HarmonicWell,hw,(dist,kappa));
 IMP_NEW(core::DistancePairScore,ps,(hw));
 return ps.release();
}

Pointer<core::SphereDistancePairScore>
 get_sphere_pair_score(FloatRange dist, double kappa)
{
 IMP_NEW(core::HarmonicWell,hw,(dist,kappa));
 IMP_NEW(core::SphereDistancePairScore,ps,(hw));
 return ps.release();
}

Pointer<core::SphereDistancePairScore>
 get_sphere_pair_score(double dist, double kappa)
{
 IMP_NEW(core::Harmonic,hu,(dist,kappa));
 IMP_NEW(core::SphereDistancePairScore,ps,(hu));
 return ps.release();
}

void add_SPBexcluded_volume
 (Model *m,atom::Hierarchies& hhs,bool GFP_exc_volume,double kappa)
{
 std::list<std::string> names;
 IMP_NEW(container::ListSingletonContainer,lsc0,(m));
 IMP_NEW(container::ListSingletonContainer,lsc1,(m));
 for(unsigned int i=0;i<hhs.size();++i){
  atom::Hierarchies hs=hhs[i].get_children();
  for(unsigned int j=0;j<hs.size();++j) {
   std::vector<std::string> strs;
   boost::split(strs,hs[j]->get_name(),boost::is_any_of("-"));
   if(strs[strs.size()-1]!="GFP"){
    if(i==0){lsc0->add_particles(atom::get_leaves(hs[j]));}
    else{    lsc1->add_particles(atom::get_leaves(hs[j]));}
   }else{
    names.push_back(hs[j]->get_name());
   }
  }
 }
 double slack=5.0;
// score
 IMP_NEW(core::SoftSpherePairScore,ssps,(kappa));
// within the primitive cell
 IMP_NEW(container::ClosePairContainer,cpc,(lsc0,slack));
 IMP_NEW(membrane::SameRigidBodyPairFilter,rbpf,());
 cpc->add_pair_filter(rbpf);
 IMP_NEW(container::PairsRestraint,evr0,(ssps,cpc));
 evr0->set_name("Excluded Volume primitive cell");
 m->add_restraint(evr0);
// across cells
 IMP_NEW(container::CloseBipartitePairContainer,cbpc,(lsc0,lsc1,slack));
 IMP_NEW(container::PairsRestraint,evr1,(ssps,cbpc));
 evr1->set_name("Excluded Volume across cells");
 m->add_restraint(evr1);
// GFPS
 if(GFP_exc_volume){
  names.sort();
  names.unique();
  std::list<std::string>::iterator iit;
  for (iit = names.begin(); iit != names.end(); iit++){
   std::string GFP_name=*iit;
// get the name of the protein attached to the GFP
   std::vector<std::string> strs;
   boost::split(strs,GFP_name,boost::is_any_of("-"));
// GFPs in the primitive cell
   IMP_NEW(container::ListSingletonContainer,lsc0,(m));
   atom::Selection s0=atom::Selection(hhs[0]);
   s0.set_molecule(GFP_name);
   lsc0->add_particles(s0.get_selected_particles());
   IMP_NEW(container::ClosePairContainer,cpc,(lsc0,slack));
   IMP_NEW(membrane::SameRigidBodyPairFilter,rbpf,());
   cpc->add_pair_filter(rbpf);
   IMP_NEW(container::PairsRestraint,evr2,(ssps,cpc));
   evr2->set_name("Excluded Volume GFPs primitive cell");
   m->add_restraint(evr2);
// GFPs across cells
   IMP_NEW(container::ListSingletonContainer,lsc1,(m));
   for(unsigned j=1;j<hhs.size();++j){
    atom::Selection s1=atom::Selection(hhs[j]);
    s1.set_molecule(GFP_name);
    lsc1->add_particles(s1.get_selected_particles());
   }
   IMP_NEW(container::CloseBipartitePairContainer,cbpc,(lsc0,lsc1,slack));
   IMP_NEW(container::PairsRestraint,evr3,(ssps,cbpc));
   evr3->set_name("Excluded Volume GFPs across cells");
   m->add_restraint(evr3);
// GFP primitive cell vs protein all cells
   IMP_NEW(container::ListSingletonContainer,lsc2,(m));
   atom::Selection s2=atom::Selection(hhs);
   s2.set_molecule(strs[0]);
   lsc2->add_particles(s2.get_selected_particles());
   IMP_NEW(container::CloseBipartitePairContainer,cbpc2,(lsc0,lsc2,slack));
   IMP_NEW(container::PairsRestraint,evr4,(ssps,cbpc2));
   evr4->set_name("Excluded Volume GFP-protein");
   m->add_restraint(evr4);
  }
 }
}

void add_internal_restraint(Model *m,std::string name,
atom::Molecule protein_a,atom::Molecule protein_b,double kappa,double dist)
{
 Pointer<core::SphereDistancePairScore> ps=get_sphere_pair_score(dist,kappa);
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

void add_my_connectivity(Model *m,std::string name,
atom::Molecule protein, double kappa)
{
 Pointer<core::SphereDistancePairScore> sdps=get_sphere_pair_score(0.0,kappa);
 Particles ps=atom::get_leaves(protein);
 for(unsigned int i=0;i<ps.size()-1;++i){
  IMP_NEW(core::PairRestraint,r,(sdps,ParticlePair(ps[i],ps[i+1])));
  r->set_name("My connectivity " + name);
  m->add_restraint(r);
 }
}

FloatRange get_range_from_fret_class(std::string r_class)
{
 FloatRange range;
 if (r_class=="High")   {range=FloatRange(0.0,  41.0);}
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

Pointer<container::MinimumPairRestraint> do_bipartite_mindist
 (Model *m,Particles p1,Particles p2,
 Pointer<core::SphereDistancePairScore> dps,bool filter)
{
 IMP_NEW(container::ListPairContainer,lpc,(m));
 for(unsigned int i=0;i<p1.size();++i){
  for(unsigned int j=0;j<p2.size();++j){
   bool samep=(atom::Hierarchy(p1[i]).get_parent() ==
               atom::Hierarchy(p2[j]).get_parent());
   if(filter && samep){continue;}
   else{lpc->add_particle_pair(ParticlePair(p1[i],p2[j]));}
  }
 }
 if(lpc->get_number_of_particle_pairs()==0) {return NULL;}
 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,lpc,1));
 return mpr.release();
}

Pointer<container::MinimumPairRestraint> do_bipartite_mindist
 (Model *m,Particles p1,Particles p2,
 Pointer<core::DistancePairScore> dps,bool filter)
{
 IMP_NEW(container::ListPairContainer,lpc,(m));
 for(unsigned int i=0;i<p1.size();++i){
  for(unsigned int j=0;j<p2.size();++j){
   bool samep=(atom::Hierarchy(p1[i]).get_parent() ==
               atom::Hierarchy(p2[j]).get_parent());
   if(filter && samep){continue;}
   else{lpc->add_particle_pair(ParticlePair(p1[i],p2[j]));}
  }
 }
 if(lpc->get_number_of_particle_pairs()==0) {return NULL;}
 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,lpc,1));
 return mpr.release();
}

Pointer<container::MinimumPairRestraint> fret_restraint
 (Model *m,
 const atom::Hierarchy&   ha, std::string protein_a, std::string residues_a,
       atom::Hierarchies& hb, std::string protein_b, std::string residues_b,
 double r_value, double kappa, bool use_GFP)
{
 atom::Selection sa=atom::Selection(ha);
 atom::Selection sb=atom::Selection(hb);
 FloatRange range=get_range_from_fret_value(r_value);
 if(use_GFP){
  protein_a=protein_a+"-"+residues_a+"-GFP";
  sa.set_molecule(protein_a);
  sa.set_residue_index(65);
  protein_b=protein_b+"-"+residues_b+"-GFP";
  sb.set_molecule(protein_b);
  sb.set_residue_index(65);
  Particles p1=sa.get_selected_particles();
  Particles p2=sb.get_selected_particles();
  if(p1.size()==0 || p2.size()==0) {return NULL;}
  Pointer<core::DistancePairScore> sps=get_pair_score(range,kappa);
  return do_bipartite_mindist(m,p1,p2,sps,false);
 } else {
  sa.set_molecule(protein_a);
  sb.set_molecule(protein_b);
  if(residues_a=="C") {sa.set_terminus(atom::Selection::C);}
  if(residues_a=="N") {sa.set_terminus(atom::Selection::N);}
  if(residues_b=="C") {sb.set_terminus(atom::Selection::C);}
  if(residues_b=="N") {sb.set_terminus(atom::Selection::N);}
  Particles p1=sa.get_selected_particles();
  Particles p2=sb.get_selected_particles();
  if(p1.size()==0 || p2.size()==0) {return NULL;}
  Pointer<core::SphereDistancePairScore> sps=get_sphere_pair_score(range,kappa);
  return do_bipartite_mindist(m,p1,p2,sps,false);
 }
}

Pointer<membrane::FretrRestraint> NEW_fret_restraint
(Model *m, atom::Hierarchies& hs,
 std::string protein_a, std::string residues_a,
 std::string protein_b, std::string residues_b, double r_value,
 FretParameters Fret, double kappa, bool use_GFP)
{
 std::string name=protein_a+"-"+residues_a+" "+protein_b+"-"+residues_b;
 atom::Selection sa=atom::Selection(hs);
 atom::Selection sb=atom::Selection(hs);
 if(use_GFP){
  protein_a=protein_a+"-"+residues_a+"-GFP";
  sa.set_molecule(protein_a);
  sa.set_residue_index(65);
  protein_b=protein_b+"-"+residues_b+"-GFP";
  sb.set_molecule(protein_b);
  sb.set_residue_index(65);
  Particles p1=sa.get_selected_particles();
  Particles p2=sb.get_selected_particles();
  if(p1.size()==0 || p2.size()==0) {return NULL;}
  IMP_NEW(membrane::FretrRestraint,fr,
   (p1,p2,Fret.R0,Fret.Sd,Fret.Sa,Fret.Gamma,Fret.Ida,r_value,kappa,name));
  return fr.release();
 } else {
  sa.set_molecule(protein_a);
  sb.set_molecule(protein_b);
  if(residues_a=="C") {sa.set_terminus(atom::Selection::C);}
  if(residues_a=="N") {sa.set_terminus(atom::Selection::N);}
  if(residues_b=="C") {sb.set_terminus(atom::Selection::C);}
  if(residues_b=="N") {sb.set_terminus(atom::Selection::N);}
  Particles p1=sa.get_selected_particles();
  Particles p2=sb.get_selected_particles();
  if(p1.size()==0 || p2.size()==0) {return NULL;}
  IMP_NEW(membrane::FretrRestraint,fr,
   (p1,p2,Fret.R0,Fret.Sd,Fret.Sa,Fret.Gamma,Fret.Ida,r_value,kappa,name));
  return fr.release();
 }
}

Pointer<container::MinimumPairRestraint> y2h_restraint
 (Model *m,
 const atom::Hierarchy&   ha, std::string protein_a, IntRange residues_a,
       atom::Hierarchies& hb, std::string protein_b, IntRange residues_b,
 double kappa)
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
 if(p1.size()==0 || p2.size()==0) {return NULL;}
 Pointer<core::SphereDistancePairScore> sps=get_sphere_pair_score(0.0,kappa);
 return do_bipartite_mindist(m,p1,p2,sps);
}

Pointer<container::MinimumPairRestraint> y2h_restraint
 (Model *m,
 const atom::Hierarchy&   ha, std::string protein_a, std::string residues_a,
       atom::Hierarchies& hb, std::string protein_b, std::string residues_b,
 double kappa)
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
 if(p1.size()==0 || p2.size()==0) {return NULL;}
 Pointer<core::SphereDistancePairScore> sps=get_sphere_pair_score(0.0,kappa);
 return do_bipartite_mindist(m,p1,p2,sps);
}

Pointer<container::MinimumPairRestraint> y2h_restraint
 (Model *m,
 const atom::Hierarchy&   ha, std::string protein_a,    IntRange residues_a,
       atom::Hierarchies& hb, std::string protein_b, std::string residues_b,
 double kappa)
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
 if(p1.size()==0 || p2.size()==0) {return NULL;}
 Pointer<core::SphereDistancePairScore> sps=get_sphere_pair_score(0.0,kappa);
 return do_bipartite_mindist(m,p1,p2,sps);
}

Pointer<container::MinimumPairRestraint> y2h_restraint
 (Model *m,
 const atom::Hierarchy&   ha, std::string protein_a, std::string residues_a,
       atom::Hierarchies& hb, std::string protein_b,    IntRange residues_b,
 double kappa)
{
 atom::Selection sa=atom::Selection(ha);
 sa.set_molecule(protein_a);
 if(residues_a=="C") {sa.set_terminus(atom::Selection::C);}
 if(residues_a=="N") {sa.set_terminus(atom::Selection::N);}
 atom::Selection sb=atom::Selection(hb);
 sb.set_molecule(protein_b);
 Ints r_b;
 for(int i=residues_b.first;i<=residues_b.second;++i) r_b.push_back(i);
 sb.set_residue_indexes(r_b);
 Particles p1=sa.get_selected_particles();
 Particles p2=sb.get_selected_particles();
 if(p1.size()==0 || p2.size()==0) {return NULL;}
 Pointer<core::SphereDistancePairScore> sps=get_sphere_pair_score(0.0,kappa);
 return do_bipartite_mindist(m,p1,p2,sps);
}

void add_link (Model *m,
 const atom::Hierarchy&   ha, std::string protein_a, std::string residues_a,
       atom::Hierarchies& hb, std::string protein_b,    IntRange residues_b,
 double kappa)
{
 atom::Hierarchies hs=ha.get_children();
 std::vector<unsigned int> index_a,index_b;
 for(unsigned int i=0;i<hs.size();++i){
  if(hs[i]->get_name()==protein_a) {index_a.push_back(i);}
  if(hs[i]->get_name()==protein_b) {index_b.push_back(i);}
 }
 if(index_a.size()!=index_b.size() || index_a.size()==0){return;}
 for(unsigned int i=0;i<index_a.size();++i){
  atom::Hierarchies hhb;
  for(unsigned int j=0;j<hb.size();++j){
   hhb.push_back(hb[j].get_children()[index_b[i]]);
  }
  Pointer<container::MinimumPairRestraint> y2h=
   y2h_restraint(m,hs[index_a[i]],protein_a,residues_a,
                   hhb,protein_b,residues_b,kappa);
  if(y2h!=NULL){m->add_restraint(y2h);}
 }
}

void add_link (Model *m,
 const atom::Hierarchy&   ha, std::string protein_a, std::string residues_a,
       atom::Hierarchies& hb, std::string protein_b, std::string residues_b,
 double kappa)
{
 atom::Hierarchies hs=ha.get_children();
 std::vector<unsigned int> index_a,index_b;
 for(unsigned int i=0;i<hs.size();++i){
  if(hs[i]->get_name()==protein_a) {index_a.push_back(i);}
  if(hs[i]->get_name()==protein_b) {index_b.push_back(i);}
 }
 if(index_a.size()!=index_b.size() || index_a.size()==0){return;}
 for(unsigned int i=0;i<index_a.size();++i){
  atom::Hierarchies hhb;
  for(unsigned int j=0;j<hb.size();++j){
   hhb.push_back(hb[j].get_children()[index_b[i]]);
  }
  Pointer<container::MinimumPairRestraint> y2h=
   y2h_restraint(m,hs[index_a[i]],protein_a,residues_a,
                   hhb,protein_b,residues_b,kappa);
  if(y2h!=NULL){m->add_restraint(y2h);}
 }
}

void add_symmetry_restraint
(Model *m,atom::Hierarchies& hs,algebra::Transformation3Ds transformations)
{
 Particles ps0=atom::get_leaves(hs[0]);
 core::RigidBodies rbs0=get_rigid_bodies(ps0);
 for(unsigned int i=1;i<transformations.size();++i){
  IMP_NEW(core::TransformationSymmetry,sm,(transformations[i]));
  Particles ps1=atom::get_leaves(hs[i]);
  IMP_NEW(container::ListSingletonContainer,lc,(m));
  for(unsigned int j=0;j<ps1.size();++j){
   if(!core::RigidMember::particle_is_instance(ps1[j])){
    core::Reference::setup_particle(ps1[j],ps0[j]);
    lc->add_particle(ps1[j]);
   }
  }
  IMP_NEW(container::SingletonsConstraint,c,(sm,NULL,lc));
  m->add_score_state(c);
  // rigid bodies
  core::RigidBodies rbs1=get_rigid_bodies(ps1);
  IMP_NEW(container::ListSingletonContainer,rblc,(m));
  for(unsigned int j=0;j<rbs1.size();++j){
   core::Reference::setup_particle(rbs1[j].get_particle(),
                                   rbs0[j].get_particle());
   rblc->add_particle(rbs1[j].get_particle());
  }
  IMP_NEW(container::SingletonsConstraint,c1,(sm,NULL,rblc));
  m->add_score_state(c1);
 }
}

void add_layer_restraint(Model *m, container::ListSingletonContainer *lsc,
 FloatRange range, double kappa)
{
 IMP_NEW(core::HarmonicWell,hw,(range,kappa));
 IMP_NEW(core::AttributeSingletonScore,asc,(hw,FloatKey("z")));
 IMP_NEW(container::SingletonsRestraint,sr,(asc, lsc));
 m->add_restraint(sr);
}

core::RigidBodies get_rigid_bodies(Particles ps)
{
 std::list<core::RigidBody> rbs_list;
 std::list<core::RigidBody>::iterator iit;
 core::RigidBodies rbs;
 for(unsigned int i=0;i<ps.size();++i){
  if(core::RigidMember::particle_is_instance(ps[i])){
   rbs_list.push_back(core::RigidMember(ps[i]).get_rigid_body());
  }
 }
 rbs_list.unique();
 for (iit = rbs_list.begin(); iit != rbs_list.end(); iit++){
  rbs.push_back(*iit);
 }
 return rbs;
}

void add_tilt (Model *m, const atom::Hierarchy& h,
 std::string name, double tilt, double kappa)
{
 atom::Selection s=atom::Selection(h);
 s.set_molecule(name);
 Particles ps=s.get_selected_particles();
 core::RigidBodies rbs=get_rigid_bodies(ps);
 for(unsigned int i=0;i<rbs.size();++i){
  add_tilt_restraint(m,rbs[i],FloatRange(0.0,tilt),kappa);
 }
}

IMPMEMBRANE_END_NAMESPACE
