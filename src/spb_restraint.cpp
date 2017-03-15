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
#include <IMP//nullptr.h>
#include <IMP/isd.h>
#include <IMP/em2d.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <list>

using namespace IMP;

IMPMEMBRANE_BEGIN_NAMESPACE

IMP::Pointer<core::DistancePairScore>
 get_pair_score(FloatRange dist, double kappa)
{
 IMP_NEW(core::HarmonicWell,hw,(dist,kappa));
 IMP_NEW(core::DistancePairScore,ps,(hw));
 return ps.release();
}

IMP::Pointer<core::SphereDistancePairScore>
 get_sphere_pair_score(FloatRange dist, double kappa)
{
 IMP_NEW(core::HarmonicWell,hw,(dist,kappa));
 IMP_NEW(core::SphereDistancePairScore,ps,(hw));
 return ps.release();
}

IMP::Pointer<core::SphereDistancePairScore>
 get_sphere_pair_score(double dist, double kappa)
{
 IMP_NEW(core::Harmonic,hu,(dist,kappa));
 IMP_NEW(core::SphereDistancePairScore,ps,(hu));
 return ps.release();
}

void add_SPBexcluded_volume
 (Model *m,RestraintSet *allrs,atom::Hierarchies& hhs,
  bool GFP_exc_volume,double kappa)
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
    atom::Hierarchies noGFPParticles=atom::get_leaves(hs[j]);
    for(unsigned int l1=0;l1<noGFPParticles.size();l1++)
     noGFP->add(noGFPParticles[l1]->get_index());
   }else{
    if(i==0){
    atom::Hierarchies GFP0Particles=atom::get_leaves(hs[j]);
    for(unsigned int l2=0;l2<GFP0Particles.size();l2++)
        GFP0->add(GFP0Particles[l2]->get_index());
   }
    names.push_back(hs[j]->get_name());
   }
  }
 }
// non GFPs

 IMP_NEW(core::ExcludedVolumeRestraint,evr1,(noGFP,kappa));
 evr1->set_name("Excluded Volume for non-GFPs");
 //m->add_restraint(evr1);
 allrs->add_restraint(evr1);

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
   Particles s0Particles=s0.get_selected_particles();

   for(unsigned int s0i=0;s0i<s0Particles.size();s0i++)
   lsc0->add(s0Particles[s0i]->get_index());
   IMP_NEW(core::ExcludedVolumeRestraint,evr2,(lsc0,kappa));
   evr2->set_name("Excluded Volume for "+GFP_name);
   //m->add_restraint(evr2);
   allrs->add_restraint(evr2);
  }

// all the GFPs in the primitive cell against all the other proteins
  double cut=1.0;
  double slack=10.0;
  IMP_NEW(container::CloseBipartitePairContainer,cbpc,(GFP0,noGFP,cut,slack));
  IMP_NEW(core::SoftSpherePairScore,ssps,(kappa));
  IMP_NEW(container::PairsRestraint,evr3,(ssps,cbpc));
  evr3->set_name("Excluded Volume GFPs vs. the rest of the world");
  //m->add_restraint(evr3);
  allrs->add_restraint(evr3);
  // print out GFP0 and noGFP particles

  /*for(unsigned int nogfpc=0;nogfpc<noGFP->get_number_of_particles();nogfpc++)
  { //noGFP->get_particle(nogfpc)->show() ;  std::cout << std::endl;
    //std::cout<< noGFP->get_particle(nogfpc)->get_index() << " " <<
    //core::XYZ(noGFP->get_particle(nogfpc)).get_coordinates() << std::endl ;
  }

  for(unsigned int gfpc=0;gfpc<GFP0->get_number_of_particles();gfpc++)
  { //GFP0->get_particle(gfpc)->show() ;  std::cout << std::endl;
  }
  */

 }

}

void add_internal_restraint(Model *m,RestraintSet *allrs,std::string name,
atom::Molecule protein_a,atom::Molecule protein_b,double kappa,double dist)
{
 IMP::Pointer<core::SphereDistancePairScore> ps=get_sphere_pair_score
(dist,kappa);
 atom::Selection sa=atom::Selection(protein_a);
 atom::Selection sb=atom::Selection(protein_b);
 sa.set_terminus(atom::Selection::C);
 sb.set_terminus(atom::Selection::N);
 Particle*  pa=sa.get_selected_particles()[0];
 Particle*  pb=sb.get_selected_particles()[0];
 IMP_NEW(core::PairRestraint,r,(
  m,ps,IMP::internal::get_index(ParticlePair(pa,pb))));
 r->set_name("IR " + name);
 //m->add_restraint(r);
 allrs->add_restraint(r);
}

void add_my_connectivity(Model *m,RestraintSet *allrs,std::string name,
atom::Molecule protein, double kappa)
{
 IMP::Pointer<core::SphereDistancePairScore> sdps=
get_sphere_pair_score(0.0,kappa);
 Particles ps=atom::get_leaves(protein);
 for(unsigned int i=0;i<ps.size()-1;++i){
  IMP_NEW(core::PairRestraint,r,(m,sdps,IMP::internal::get_index(
   ParticlePair(ps[i],ps[i+1]))));
  r->set_name("My connectivity " + name);
  //m->add_restraint(r);
  allrs->add_restraint(r);
 }
}

void add_restrain_coiledcoil_to_cterm(Model *m,RestraintSet *allrs,
  const atom::Hierarchy& hs,std::string protein_a,Particle *dist,
  double sigma0_dist)

 atom::Selection cterms=atom::Selection(hs);

 coiledcoilends.set_molecule(protein_a); // all Spc42 in unit cell
 cterms.set_molecule(protein_a);

 coiledcoilends.set_residue_index(135); //choose the last bead in coiled coil
 cterms.set_terminus(atom::Selection::C);

 Particles cce=coiledcoilends.get_selected_particles();
 Particles ct=cterms.get_selected_particles();

 for(unsigned int i=0; i<cce.size();i++)
 {
   for(unsigned int j=0;j<ct.size();j++)
   {
        bool samep=(atom::Hierarchy(cce[i]).get_parent() ==
               atom::Hierarchy(ct[j]).get_parent());
        if(!samep){continue;}
        // we want to restrain termini belonging to the same protein

        std::string name_restraint = "spc42c-" + protein_a + "-" +
  std::to_string(i) + ":" + std::to_string(j);

        //std::cout << name_restraint <<std::endl ;
 //std::cout << cce[i]->get_name() << " " << ct[j]->get_name()<< " "
 //<< isd::Scale(dist).get_scale() << " " <<sigma0_dist ;

        IMP_NEW(membrane::DistanceTerminiRestraint,dtr,(cce[i],ct[j],
    dist,sigma0_dist));

        dtr->set_name(name_restraint);

       // m->add_restraint(dtr);
       allrs->add_restraint(dtr);
   }
 }

}

void add_restrain_protein_length(Model *m,RestraintSet *allrs,
  const atom::Hierarchy& hs,std::string protein_a,Particle *dist,
  double sigma0_dist)
{
 atom::Selection nterms=atom::Selection(hs);
 atom::Selection cterms=atom::Selection(hs);

 nterms.set_molecule(protein_a); // all Spc29 in unit cell
 cterms.set_molecule(protein_a);

 nterms.set_terminus(atom::Selection::N);
 cterms.set_terminus(atom::Selection::C);

 Particles nt=nterms.get_selected_particles();
 Particles ct=cterms.get_selected_particles();

 for(unsigned int i=0; i<nt.size();i++)
 {
   for(unsigned int j=0;j<ct.size();j++)
   {
 bool samep=(atom::Hierarchy(nt[i]).get_parent() ==
               atom::Hierarchy(ct[j]).get_parent());
    if(!samep){continue;}
 // we want to restrain termini belonging to the same protein

        std::string name_restraint = protein_a + "-" +
  std::to_string(i) + ":" + std::to_string(j);

 //std::cout << name_restraint <<std::endl ;
 IMP_NEW(membrane::DistanceTerminiRestraint,dtr,
         (nt[i],ct[j],dist,sigma0_dist));

 dtr->set_name(name_restraint);

 //m->add_restraint(dtr);
 allrs->add_restraint(dtr);
   }
 }

}

IMP::Pointer<container::MinimumPairRestraint> do_bipartite_mindist
 (Model *m,Particles p1,Particles p2,
 IMP::Pointer<core::SphereDistancePairScore> dps,bool filter)
{
 IMP_NEW(container::ListPairContainer,lpc,(m));
 for(unsigned int i=0;i<p1.size();++i){
  for(unsigned int j=0;j<p2.size();++j){
   bool samep=(atom::Hierarchy(p1[i]).get_parent() ==
               atom::Hierarchy(p2[j]).get_parent());
   if(filter && samep){continue;}
   //else{lpc->add_particle_pair(ParticlePair(p1[i],p2[j]));}
   else{lpc->add(IMP::internal::get_index(ParticlePair(p1[i],p2[j])));}
  }
 }

 //if(lpc->get_number_of_particle_pairs()==0) {return NULL;}
 if((lpc->get_particle_pairs()).size()==0) {return nullptr;}

 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,lpc,1));
 return mpr.release();
}

IMP::Pointer<container::MinimumPairRestraint> do_bipartite_mindist
 (Model *m,Particles p1,Particles p2,
 IMP::Pointer<core::DistancePairScore> dps,bool filter)
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
 //if(lpc->get_number_of_particle_pairs()==0) {return NULL;}
 if((lpc->get_particle_pairs()).size()==0) {return nullptr;}


 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,lpc,1));
 return mpr.release();
}

IMP::Pointer<isd::FretRestraint> fret_restraint
(Model *m, atom::Hierarchies& hs,
 std::string protein_a, std::string residues_a,
 std::string protein_b, std::string residues_b, double fexp,
 FretParameters Fret, std::string cell_type, bool use_GFP,
 Particle *Kda, Particle *Ida, Particle *R0, Particle *Sigma0, Particle *pBl)
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
 // get particles
 Particles p1=sa.get_selected_particles();
 Particles p2=sb.get_selected_particles();
 if(p1.size()==0 || p2.size()==0) {return nullptr;}
 IMP_NEW(isd::FretRestraint,fr,(p1,p2,Kda,Ida,R0,Sigma0,pBl,fexp,m_d,m_a));
 fr->set_name(name);
 return fr.release();
}

IMP::Pointer<isd::FretRestraint> fret_restraint
(Model *m, atom::Hierarchies& hs,
 std::string protein_a, std::string residues_a,
 std::string protein_b, int residues_b, double fexp,
 FretParameters Fret, std::string cell_type, bool use_GFP,
 Particle *Kda, Particle *Ida, Particle *R0, Particle *Sigma0, Particle *pBl)
{
 std::stringstream out;
 out << residues_b;
 std::string name=protein_a+"-"+residues_a+" "+protein_b+"-"+out.str();
// donor and acceptor multiplicity
 double m_d=1.;
 double m_a=7.;
 atom::Hierarchies hhs;
 hhs.push_back(hs[0]);
 if(cell_type=="rhombus"){m_d=1./3.;}
// Selections
 atom::Selection sa=atom::Selection(hhs);
 atom::Selection sb=atom::Selection(hs);
 // A
 if(use_GFP){
  protein_a=protein_a+"-"+residues_a+"-GFP";
  sa.set_molecule(protein_a);
  sa.set_residue_index(65);
 } else {
  sa.set_molecule(protein_a);
  if(residues_a=="C") {sa.set_terminus(atom::Selection::C);}
  if(residues_a=="N") {sa.set_terminus(atom::Selection::N);}
 }
 // B
 sb.set_molecule(protein_b);
 sb.set_residue_index(residues_b);
 // get particles
 Particles p1=sa.get_selected_particles();
 Particles p2=sb.get_selected_particles();
 if(p1.size()==0 || p2.size()==0) {return nullptr;}
 IMP_NEW(isd::FretRestraint,fr,(p1,p2,Kda,Ida,R0,Sigma0,pBl,fexp,m_d,m_a));
 fr->set_name(name);
 return fr.release();
}

IMP::Pointer<isd::FretRestraint> fret_restraint
(Model *m, atom::Hierarchies& hs,
 std::string protein_a, int residues_a,
 std::string protein_b, std::string residues_b, double fexp,
 FretParameters Fret, std::string cell_type, bool use_GFP,
 Particle *Kda, Particle *Ida, Particle *R0, Particle *Sigma0, Particle *pBl)
{
 std::stringstream out;
 out << residues_a;
 std::string name=protein_a+"-"+out.str()+" "+protein_b+"-"+residues_b;
// donor and acceptor multiplicity
 double m_d=1.;
 double m_a=7.;
 atom::Hierarchies hhs;
 hhs.push_back(hs[0]);
 if(cell_type=="rhombus"){m_d=1./3.;}
// Selections
 atom::Selection sa=atom::Selection(hhs);
 atom::Selection sb=atom::Selection(hs);
 // A
 sa.set_molecule(protein_a);
 sa.set_residue_index(residues_a);
 // B
 if(use_GFP){
  protein_b=protein_b+"-"+residues_b+"-GFP";
  sb.set_molecule(protein_b);
  sb.set_residue_index(65);
 } else {
  sb.set_molecule(protein_b);
  if(residues_b=="C") {sb.set_terminus(atom::Selection::C);}
  if(residues_b=="N") {sb.set_terminus(atom::Selection::N);}
 }
 // get particles
 Particles p1=sa.get_selected_particles();
 Particles p2=sb.get_selected_particles();
 if(p1.size()==0 || p2.size()==0) {return nullptr;}

 //for(unsigned int i=0;i<p1.size();i++)
 // p1[i]->show();

 //std::cout << name << " " << p1.size() << " " << p2.size() <<std::endl;

 IMP_NEW(isd::FretRestraint,fr,(p1,p2,Kda,Ida,R0,Sigma0,pBl,fexp,m_d,m_a));
 fr->set_name(name);
 return fr.release();
}

IMP::Pointer<container::MinimumPairRestraint> y2h_restraint
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
 if(p1.size()==0 || p2.size()==0) {return nullptr;}
 IMP::Pointer<core::SphereDistancePairScore> sps=
get_sphere_pair_score(0.0,kappa);
 return do_bipartite_mindist(m,p1,p2,sps);
}

IMP::Pointer<container::MinimumPairRestraint> y2h_restraint
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
 if(p1.size()==0 || p2.size()==0) {return nullptr;}
 IMP::Pointer<core::SphereDistancePairScore> sps=
get_sphere_pair_score(0.0,kappa);
 return do_bipartite_mindist(m,p1,p2,sps);
}

IMP::Pointer<container::MinimumPairRestraint> y2h_restraint
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
 if(p1.size()==0 || p2.size()==0) {return nullptr;}
 IMP::Pointer<core::SphereDistancePairScore> sps=
get_sphere_pair_score(0.0,kappa);
 return do_bipartite_mindist(m,p1,p2,sps);
}

IMP::Pointer<container::MinimumPairRestraint> y2h_restraint
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
 if(p1.size()==0 || p2.size()==0) {return nullptr;}
 IMP::Pointer<core::SphereDistancePairScore> sps=
get_sphere_pair_score(0.0,kappa);
 return do_bipartite_mindist(m,p1,p2,sps);
}

void add_link (Model *m,RestraintSet *allrs,
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
  IMP::Pointer<container::MinimumPairRestraint> y2h=
   y2h_restraint(m,hs[index_a[i]],protein_a,residues_a,
                   hhb,protein_b,residues_b,kappa);
  if(y2h!=nullptr){
    //m->add_restraint(y2h);}
    allrs->add_restraint(y2h);}
 }
}

void add_link (Model *m,RestraintSet *allrs,
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
  IMP::Pointer<container::MinimumPairRestraint> y2h=
   y2h_restraint(m,hs[index_a[i]],protein_a,residues_a,
                   hhb,protein_b,residues_b,kappa);
  //if(y2h!=NULL){m->add_restraint(y2h);}
  if(y2h!=nullptr) {
    //m->add_restraint(y2h);
    allrs->add_restraint(y2h);
  }
 }
}

void add_symmetry_restraint
(Model *m,atom::Hierarchies& hs,algebra::Transformation3Ds transformations,
 Particle *SideXY, Particle *SideZ)
{
 Particles ps0=atom::get_leaves(hs[0]);
 std::vector<core::RigidBody> rbs0=get_rigid_bodies(ps0);
 for(unsigned int i=1;i<transformations.size();++i){
  // apply transformation symmetry using sm
  IMP_NEW(membrane::TransformationSymmetry,sm,
          (transformations[i],SideXY,SideXY,SideZ));
  Particles ps1=atom::get_leaves(hs[i]);
  IMP_NEW(container::ListSingletonContainer,lc,(m));
  for(unsigned int j=0;j<ps1.size();++j){
   if(!core::RigidMember::get_is_setup(ps1[j])){
    core::Reference::setup_particle(ps1[j],ps0[j]);
    //std::cout << core::XYZ(ps1[j]).get_coordinates() <<  std::endl;
    lc->add(ps1[j]->get_index());
   }
  }
  IMP_NEW(container::SingletonsConstraint,c,(sm,NULL,lc));
  m->add_score_state(c);
  // rigid bodies
  std::vector<core::RigidBody> rbs1=get_rigid_bodies(ps1);
  IMP_NEW(container::ListSingletonContainer,rblc,(m));
  for(unsigned int j=0;j<rbs1.size();++j){
   core::Reference::setup_particle(rbs1[j].get_particle(),
                                   rbs0[j].get_particle());
   rblc->add(rbs1[j].get_particle_index());
   //std::cout <<  core::XYZ(rbs1[j].get_particle()).get_coordinates()
   //<<  std::endl;
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
(Model *m, RestraintSet *allrs,container::ListSingletonContainer *lsc,
 Particle *a, Particle *b)
{
 Particles ps=lsc->get_particles();
 for(unsigned i=0; i<ps.size(); ++i){
  IMP_NEW(membrane::UniformBoundedRestraint,ubr,(ps[i], FloatKey("z"), a, b));
  //m->add_restraint(ubr);
  allrs->add_restraint(ubr);
 }
}

std::vector<core::RigidBody> get_rigid_bodies(Particles ps)
{
 std::vector<core::RigidBody> rbs;
 for(unsigned i=0;i<ps.size();++i){
  if(core::RigidMember::get_is_setup(ps[i])){
   core::RigidBody rb = core::RigidMember(ps[i]).get_rigid_body();
   std::vector<core::RigidBody>::iterator it = find(rbs.begin(), rbs.end(), rb);
   if(it==rbs.end()){rbs.push_back(rb);}
  }
 }
 return rbs;
}

void add_tilt_restraint
 (Model *m,RestraintSet *allrs,Particle *p,FloatRange tilt_range,double kappa)
{
 algebra::Vector3D laxis=algebra::Vector3D(0.0,0.0,1.0);
 algebra::Vector3D zaxis=algebra::Vector3D(0.0,0.0,1.0);
 IMP_NEW(core::HarmonicWell,well,(tilt_range, kappa));
 IMP_NEW(TiltSingletonScore,tss,(well,laxis,zaxis));
 IMP_NEW(core::SingletonRestraint,sr,(tss,p));
 //m->add_restraint(sr);
 allrs->add_restraint(sr);
 sr->set_name("Tilt restraint");
}

void add_tilt (Model *m,RestraintSet *allrs,const atom::Hierarchy& h,
 std::string name, IntRange range, double tilt, double kappa)
{
 atom::Selection s=atom::Selection(h);
 s.set_molecule(name);
 Ints indices;
 for(int i=range.first;i<=range.second;++i){indices.push_back(i);}
 s.set_residue_indexes(indices);
 Particles ps=s.get_selected_particles();
 if(ps.size()>0){
  std::vector<core::RigidBody> rbs=get_rigid_bodies(ps);
  for(unsigned int i=0;i<rbs.size();++i){
   add_tilt_restraint(m,allrs,rbs[i],FloatRange(0.0,tilt),kappa);
   }
 }
}

void add_GFP_restraint(Model *m, RestraintSet *allrs,
 const atom::Hierarchy& h, double kappa)
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
    //m->add_restraint(sr);
    allrs->add_restraint(sr);
  }
 }
}

void add_stay_close_restraint(Model *m,RestraintSet *allrs,
 const atom::Hierarchy& h, std::string protein, double kappa)
{
// Sphere pair score
  IMP::Pointer<core::SphereDistancePairScore> sps=
get_sphere_pair_score(0.0,kappa);
// selection
  atom::Selection s=atom::Selection(h);
  s.set_molecule(protein);
  Particles ps=s.get_selected_particles();
// check if empty particles
  if(ps.size()==0) {return;}
// container
  IMP_NEW(container::ListPairContainer,lpc,(m));
  for(unsigned int i=0;i<ps.size()-1;++i){
   for(unsigned int j=i+1;j<ps.size();++j){
    bool samep=(atom::Hierarchy(ps[i]).get_parent() ==
                atom::Hierarchy(ps[j]).get_parent());
    if(!samep){lpc->add_particle_pair(ParticlePair(ps[i],ps[j]));}
   }
  }
  //if(lpc->get_number_of_particle_pairs()==0) {return;}
  if((lpc->get_indexes()).size()==0) {return;}
  IMP_NEW(container::MinimumPairRestraint,mpr,(sps,lpc,1));
  mpr->set_name("Stay close restraint");
  //m->add_restraint(mpr);
  allrs->add_restraint(mpr);
}

void add_stay_close_restraint(Model *m,RestraintSet *allrs,
 const atom::Hierarchy& h, std::string protein, int residue, double kappa)
{
// Sphere pair score
  IMP::Pointer<core::SphereDistancePairScore> sps=
get_sphere_pair_score(0.0,kappa);
// selection
  atom::Selection s=atom::Selection(h);
  s.set_molecule(protein);
  s.set_residue_index(residue);
  Particles ps=s.get_selected_particles();
// check if empty particles
  if(ps.size()==0) {return;}
// container
  IMP_NEW(container::ListPairContainer,lpc,(m));
  for(unsigned int i=0;i<ps.size()-1;++i){
   for(unsigned int j=i+1;j<ps.size();++j){
    bool samep=(atom::Hierarchy(ps[i]).get_parent() ==
                atom::Hierarchy(ps[j]).get_parent());
    if(!samep){lpc->add_particle_pair(ParticlePair(ps[i],ps[j]));}
   }
  }
  //if(lpc->get_number_of_particle_pairs()==0) {return;}
  if((lpc->get_indexes()).size()==0) {return;}

  IMP_NEW(container::MinimumPairRestraint,mpr,(sps,lpc,1));
  mpr->set_name("Stay close restraint");
  //m->add_restraint(mpr);
  allrs->add_restraint(mpr);
}

void add_stay_on_plane_restraint(Model *m,RestraintSet *allrs,
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
   IMP_NEW(core::PairRestraint,pr,(adps,
ParticleIndexPair(ps[i]->get_index(), ps[j]->get_index())));
   pr->set_name("Stay on z-plane restraint");
   //m->add_restraint(pr);
   allrs_>add_restraint(pr);
  }
 }
}

void add_diameter_rgyr_restraint(Model *m,RestraintSet *allrs,
 const atom::Hierarchy& h, std::string protein,
 double diameter, double rgyr, double kappa)
{
 atom::Hierarchies hs=h.get_children();
 for(unsigned int j=0;j<hs.size();++j){
  if(hs[j]->get_name()==protein){
   Particles ps = atom::get_leaves(hs[j]);
   IMP_NEW(membrane::DiameterRgyrRestraint,dr,(ps,diameter,rgyr,kappa));
   dr->set_name("Diameter and Radius of Gyration Restraint");
   //m->add_restraint(dr);
   allrs->add_restraint(dr);
  }
 }
}

IMP::Pointer<membrane::EM2DRestraint> em2d_restraint
(Model *m, atom::Hierarchies& hs,
 std::string protein, EM2DParameters EM2D, Particle *Sigma)
{
 atom::Selection s=atom::Selection(hs);
 s.set_molecule(protein);
 Particles ps=s.get_selected_particles();
 if(ps.size()==0) {return nullptr;}
 IMP_NEW(membrane::EM2DRestraint,er,(ps, Sigma, EM2D.filename,
                                  EM2D.pixel_size, EM2D.resolution));
 er->set_name("EM2D restraint");
 return er.release();
}

IMP::Pointer<membrane::EM2DRestraint> em2d_restraint
(Model *m, atom::Hierarchies& hs, std::string protein,
 EM2DParameters EM2D, Floats sigma_grid, Floats fmod_grid)
{
 atom::Selection s=atom::Selection(hs);
 s.set_molecule(protein);
 Particles ps=s.get_selected_particles();
 if(ps.size()==0) {return nullptr;}
 IMP_NEW(membrane::EM2DRestraint,er,(ps, sigma_grid, fmod_grid, EM2D.filename,
                                EM2D.pixel_size, EM2D.resolution));

 er->set_name("EM2D restraint");
 return er.release();
}



IMPMEMBRANE_END_NAMESPACE
