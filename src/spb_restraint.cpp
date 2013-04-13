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
#include <IMP/isd2.h>
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
 IMP_NEW(container::ListSingletonContainer,noGFP,(m));
 IMP_NEW(container::ListSingletonContainer,GFP0,(m));
 for(unsigned int i=0;i<hhs.size();++i){
  atom::Hierarchies hs=hhs[i].get_children();
  for(unsigned int j=0;j<hs.size();++j){
   std::vector<std::string> strs;
   boost::split(strs,hs[j]->get_name(),boost::is_any_of("-"));
   if(strs[strs.size()-1]!="GFP"){
    noGFP->add_particles(atom::get_leaves(hs[j]));
   }else{
    if(i==0){GFP0->add_particles(atom::get_leaves(hs[j]));}
    names.push_back(hs[j]->get_name());
   }
  }
 }
// non GFPs
 IMP_NEW(core::ExcludedVolumeRestraint,evr1,(noGFP,kappa));
 evr1->set_name("Excluded Volume for non-GFPs");
 m->add_restraint(evr1);
// In case you don't want transparent GFPs
 if(GFP_exc_volume){
  names.sort();
  names.unique();
// intra GFPs of a specific type
  std::list<std::string>::iterator iit;
  for (iit = names.begin(); iit != names.end(); iit++){
   std::string GFP_name=*iit;
   IMP_NEW(container::ListSingletonContainer,lsc0,(m));
   atom::Selection s0=atom::Selection(hhs);
   s0.set_molecule(GFP_name);
   lsc0->add_particles(s0.get_selected_particles());
   IMP_NEW(core::ExcludedVolumeRestraint,evr2,(lsc0,kappa));
   evr2->set_name("Excluded Volume for "+GFP_name);
   m->add_restraint(evr2);
  }
// all the GFPs in the primitive cell against all the other proteins
  double cut=1.0;
  double slack=10.0;
  IMP_NEW(container::CloseBipartitePairContainer,cbpc,(GFP0,noGFP,cut,slack));
  IMP_NEW(core::SoftSpherePairScore,ssps,(kappa));
  IMP_NEW(container::PairsRestraint,evr3,(ssps,cbpc));
  evr3->set_name("Excluded Volume GFPs vs. the rest of the world");
  m->add_restraint(evr3);
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

Pointer<isd2::FretRestraint> fret_restraint
(Model *m, atom::Hierarchies& hs,
 std::string protein_a, std::string residues_a,
 std::string protein_b, std::string residues_b, double fexp,
 FretParameters Fret, std::string cell_type, bool use_GFP,
 Particle *Kda, Particle *Ida, Particle *R0, Particle *Sigma0,
 Particle *pBl)
{
 std::string name=protein_a+"-"+residues_a+" "+protein_b+"-"+residues_b;
// donor and acceptor multiplicity
 double m_d=1.;
 double m_a=7.;
 atom::Hierarchies hhs;
 hhs.push_back(hs[0]);
 if(cell_type=="rhombus"){m_d=1./3.;}
// Selections
 atom::Selection sa=atom::Selection(hhs);
 atom::Selection sb=atom::Selection(hs);
// Particles
 Particles p1;
 Particles p2;
 if(use_GFP){
  protein_a=protein_a+"-"+residues_a+"-GFP";
  sa.set_molecule(protein_a);
  sa.set_residue_index(65);
  protein_b=protein_b+"-"+residues_b+"-GFP";
  sb.set_molecule(protein_b);
  sb.set_residue_index(65);
 } else {
  sa.set_molecule(protein_a);
  sb.set_molecule(protein_b);
  if(residues_a=="C") {sa.set_terminus(atom::Selection::C);}
  if(residues_a=="N") {sa.set_terminus(atom::Selection::N);}
  if(residues_b=="C") {sb.set_terminus(atom::Selection::C);}
  if(residues_b=="N") {sb.set_terminus(atom::Selection::N);}
 }
 p1=sa.get_selected_particles();
 p2=sb.get_selected_particles();
 if(p1.size()==0 || p2.size()==0) {return NULL;}
 IMP_NEW(isd2::FretRestraint,fr,(p1,p2,Kda,Ida,R0,Sigma0,pBl,fexp,m_d,m_a));
 fr->set_name(name);
 return fr.release();
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
(Model *m,atom::Hierarchies& hs,algebra::Transformation3Ds transformations,
 Particle *SideXY, Particle *SideZ)
{
 Particles ps0=atom::get_leaves(hs[0]);
 core::RigidBodies rbs0=get_rigid_bodies(ps0);
 for(unsigned int i=1;i<transformations.size();++i){
  IMP_NEW(isd2::TransformationSymmetry,sm,
          (transformations[i],SideXY,SideXY,SideZ));
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

void add_bayesian_layer_restraint
(Model *m, container::ListSingletonContainer *lsc, Particle *a, Particle *b)
{
 Particles ps=lsc->get_particles();
 for(unsigned i=0; i<ps.size(); ++i){
  IMP_NEW(isd2::UniformBoundedRestraint,ubr,(ps[i], FloatKey("z"), a, b));
  m->add_restraint(ubr);
 }
}

core::RigidBodies get_rigid_bodies(Particles ps)
{
 std::vector<core::RigidBody> rbs;
 for(unsigned i=0;i<ps.size();++i){
  if(core::RigidMember::particle_is_instance(ps[i])){
   core::RigidBody rb = core::RigidMember(ps[i]).get_rigid_body();
   std::vector<core::RigidBody>::iterator it = find(rbs.begin(), rbs.end(), rb);
   if(it==rbs.end()){rbs.push_back(rb);}
  }
 }
 return rbs;
}

void add_tilt_restraint
 (Model *m,Particle *p,FloatRange tilt_range,double kappa)
{
 algebra::Vector3D laxis=algebra::Vector3D(1.0,0.0,0.0);
 algebra::Vector3D zaxis=algebra::Vector3D(0.0,0.0,1.0);
 IMP_NEW(core::HarmonicWell,well,(tilt_range, kappa));
 IMP_NEW(TiltSingletonScore,tss,(well,laxis,zaxis));
 IMP_NEW(core::SingletonRestraint,sr,(tss,p));
 m->add_restraint(sr);
 sr->set_name("Tilt restraint");
}

void add_tilt (Model *m, const atom::Hierarchy& h,
 std::string name, IntRange range, double tilt, double kappa)
{
 atom::Selection s=atom::Selection(h);
 s.set_molecule(name);
 Ints indices;
 for(int i=range.first;i<=range.second;++i){indices.push_back(i);}
 s.set_residue_indexes(indices);
 Particles ps=s.get_selected_particles();
 if(ps.size()>0){
  core::RigidBodies rbs=get_rigid_bodies(ps);
  for(unsigned int i=0;i<rbs.size();++i){
   add_tilt_restraint(m,rbs[i],FloatRange(0.0,tilt),kappa);
  }
 }
}

void add_GFP_restraint(Model *m, const atom::Hierarchy& h, double kappa)
{
 IMP_NEW(core::Harmonic,hmonic,(0.0,kappa));
 atom::Hierarchies hs=h.get_children();
 for(unsigned int j=0;j<hs.size();++j){
  std::vector<std::string> strs;
  boost::split(strs,hs[j]->get_name(),boost::is_any_of("-"));
  if(strs[strs.size()-1]=="GFP"){
    atom::Selection s=atom::Selection(hs[j]);
    s.set_residue_index(65);
    Particles ps=s.get_selected_particles();
    core::XYZ ps_xyz=core::XYZ(ps[0]);
    algebra::Vector3D xyz=ps_xyz.get_coordinates();
    IMP_NEW(core::DistanceToSingletonScore,dtss,(hmonic,xyz));
    IMP_NEW(core::SingletonRestraint,sr,(dtss,ps_xyz));
    sr->set_name("GFP Position Restraint");
    m->add_restraint(sr);
  }
 }
}

void add_stay_close_restraint(Model *m,
 const atom::Hierarchy& ha, std::string protein_a,
 const atom::Hierarchy& hb, std::string protein_b,
 double kappa)
{
// Sphere pair score
  Pointer<core::SphereDistancePairScore> sps=get_sphere_pair_score(0.0,kappa);
// first selection
  atom::Selection sa=atom::Selection(ha);
  sa.set_molecule(protein_a);
  Particles pa=sa.get_selected_particles();
// second selection
  atom::Selection sb=atom::Selection(hb);
  sb.set_molecule(protein_b);
  Particles pb=sb.get_selected_particles();
// check if empty particles
  if(pa.size()==0 || pb.size()==0) {return;}
  Pointer<container::MinimumPairRestraint> mpr=
    do_bipartite_mindist(m,pa,pb,sps);
  mpr->set_name("Stay close restraint");
  m->add_restraint(mpr);
}

void add_stay_on_plane_restraint(Model *m,
 const atom::Hierarchy& h, std::string protein, int residue, double kappa)
{
 atom::Selection s=atom::Selection(h);
 s.set_molecule(protein);
 s.set_residue_index(residue);
 Particles ps=s.get_selected_particles();
 IMP_NEW(core::Harmonic,har,(0.0,kappa));
 IMP_NEW(membrane::AttributeDistancePairScore,adps,
  (har,core::XYZ::get_coordinate_key(2)));
 for(unsigned i=0;i<ps.size()-1;++i){
  for(unsigned j=i+1;j<ps.size();++j){
   IMP_NEW(core::PairRestraint,pr,(adps,ParticlePair(ps[i], ps[j])));
   pr->set_name("Stay on z-plane restraint");
   m->add_restraint(pr);
  }
 }
}

IMPMEMBRANE_END_NAMESPACE
