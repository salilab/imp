/**
 *  \file spb.cpp
 *  \brief SPB in C++
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/domino.h>
#include <boost/program_options.hpp>
#include <IMP/base_types.h>
#include <string>
#include <iostream>
using namespace IMP;

// various parameters
const double ds=40.0;
double       side=80.0;
const int    niter=3;
bool         do_statistics=true;
bool         do_random=true;
bool         do_save_ass=false;
const int    skip=100;
std::string  cell_type="hexagon";
int          num_cells;
int          num_copies;
double       error_bound;

algebra::Vector2Ds do_compress(algebra::Vector2Ds points,double xf,double yf)
{
 algebra::Vector2Ds ret=algebra::Vector2Ds();
 for(int i=0;i<points.size();++i){
  ret.push_back(algebra::Vector2D(points[i][0]*xf,points[i][1]*yf));
 }
 return ret;
}

algebra::Vector2Ds do_shear(algebra::Vector2Ds points,double bh,double h)
{
 algebra::Vector2Ds ret=algebra::Vector2Ds();
 double b=bh/h;
 for(int i=0;i<points.size();++i){
  ret.push_back(algebra::Vector2D(points[i][0]-b*points[i][1],points[i][1]));
 }
 return ret;
}

algebra::Vector3Ds grid_cell(double side,double ds,double z)
{
 algebra::BoundingBox2D bb=algebra::BoundingBox2D(algebra::Vector2D(0.0,0.0),
                                               algebra::Vector2D(side,side));
 algebra::Vector2Ds cur=algebra::get_grid_interior_cover_by_spacing(bb, ds);

 if(cell_type!="square"){
  algebra::Vector2Ds cur_comp=do_compress(cur,1.0,sqrt(3.0)/2.0);
  cur=do_shear(cur_comp,side/2.0,side*sqrt(3.0)/2.0);
 }

 algebra::Vector3Ds positions=algebra::Vector3Ds();
 algebra::Vector3Ds positions2=algebra::Vector3Ds();
 for(int i=0;i<cur.size();++i){
  positions.push_back(algebra::Vector3D(cur[i][0],cur[i][1],z));
 }
 if(cell_type=="hexagon"){
  algebra::Vector3D tra=algebra::Vector3D(0.0,0.0,0.0);
  for(int i=0;i<3;++i){
   algebra::Rotation3D rot=
   algebra::get_rotation_about_axis(algebra::Vector3D(0.0,0.0,1.0),
                                    (double)i * 2.0 * IMP::PI / 3.0);
   algebra::Transformation3D tr=algebra::Transformation3D(rot,tra);
   for(int j=0;j<positions.size();++j){
    positions2.push_back(tr.get_transformed(positions[j]));
   }
  }
 }
 if(positions2.size()>0){
  positions.insert(positions.end(),positions2.begin(),positions2.end());
 }
 return positions;
}

atom::Hierarchies create_hierarchies(Model *m,int ncells,std::string name)
{
 atom::Hierarchies hs=atom::Hierarchies();
 for(int i=0;i<ncells;++i){
  IMP_NEW(Particle,p,(m));
  atom::Hierarchy h=atom::Hierarchy::setup_particle(p);
  std::stringstream out;
  out << i;
  h->set_name(name+" hierarchy, cell " + out.str());
  hs.push_back(h);
 }
 return hs;
}

atom::Molecule create_protein(Model *m,std::string name,double mass,
 int nbeads,int copy,int start_residue=-1,int length=-1)
{
 if(length==-1) {length=(int) mass*1000.0/110;}
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
  core::XYZR d=core::XYZR::setup_particle(pp);
  d.set_radius(rg);
  atom::Mass mm=atom::Mass::setup_particle(pp,ms);
  protein.add_child(dom);
 }
 if(nbeads>1 && copy==0){
  //con=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c) \
  //                             for c in protein.get_children()],1.0)
  //con.set_name("Connectivity Restraint for "+name)
  //model.add_restraint(con)
  //model.set_maximum_score(con, error_bound)
 }
 return protein;
}

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
 sa.set_terminus(atom::Selection::N);
 Particle*  pa=sa.get_selected_particles()[0];
 Particle*  pb=sb.get_selected_particles()[0];
 IMP_NEW(core::PairRestraint,r,(ps,ParticlePair(pa, pb)));
 r->set_name("IR " + name);
 m->add_restraint(r);
 m->set_maximum_score(r,error_bound);
}

void add_distance_restraint(Model *m,std::string name,Particle* p1,
Particle* p2,core::TableRefiner *tbr,FloatRange dist)
{
 core::DistancePairScore* ps=get_pair_score(dist);
 IMP_NEW(core::KClosePairsPairScore,kps,(ps,tbr,1.0));
 IMP_NEW(core::PairRestraint,mpr,(kps,ParticlePair(p1,p2)));
 mpr->set_name(name);
 m->add_restraint(mpr);
 m->set_maximum_score(mpr,error_bound);
}

atom::Molecule create_merged_protein
(Model *m,std::string name,atom::Molecule protein_a,
atom::Molecule protein_b,int copy,double dist=-1.0)
{
 IMP_NEW(Particle,p,(m));
 atom::Molecule h=atom::Molecule::setup_particle(p);
 h->set_name(name);
 if (copy==0 and dist >=0.0){
  add_internal_restraint(m,name,protein_a,protein_b,dist);
 }
 ParticlesTemp psa=protein_a.get_leaves();
 for(int i=0;i<psa.size();++i){
  protein_a.remove_child(atom::Domain(psa[i]));
  h.add_child(atom::Domain(psa[i]));
 }
 ParticlesTemp psb=protein_b.get_leaves();
 for(int i=0;i<psb.size();++i){
  protein_b.remove_child(atom::Domain(psb[i]));
  h.add_child(atom::Domain(psb[i]));
 }
 return h;
}

FloatRange get_range_from_fret_class(std::string r_class)
{
 if (r_class=="High")   {return FloatRange(-500.0,  41.0);}
 if (r_class=="Mod")    {return FloatRange(41.0, 55.5);}
 if (r_class=="Low")    {return FloatRange(55.5, 66.0);}
 if (r_class=="Lowest") {return FloatRange(66.0, 70.0);}
 if (r_class=="None")   {return FloatRange(70.0, 100000.0);}
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
 core::DistancePairScore* dps,bool filter=false)
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
 m->set_maximum_score(mpr, error_bound);
}

void do_allpairs_mindist(Model *m,Particles ps,
 core::DistancePairScore* dps,bool filter=false)
{
 IMP_NEW(container::ListSingletonContainer,lp,(ps));
 IMP_NEW(container::AllPairContainer,pc,(lp));
 if(filter){
  //f=IMP.atom.SameResiduePairFilter()
  //pc.add_pair_filter(f)
 }
 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,pc,1));
 m->add_restraint(mpr);
 m->set_maximum_score(mpr, error_bound);
}

void do_bipartite_mindist(Model *m,Particles p1,Particles p2,
 core::SphereDistancePairScore* dps,bool filter=false)
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
 m->set_maximum_score(mpr, error_bound);
}

void do_allpairs_mindist(Model *m,Particles ps,
 core::SphereDistancePairScore* dps,bool filter=false)
{
 IMP_NEW(container::ListSingletonContainer,lp,(ps));
 IMP_NEW(container::AllPairContainer,pc,(lp));
 if(filter){
  //f=IMP.atom.SameResiduePairFilter()
  //pc.add_pair_filter(f)
 }
 IMP_NEW(container::MinimumPairRestraint,mpr,(dps,pc,1));
 m->add_restraint(mpr);
 m->set_maximum_score(mpr, error_bound);
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

void add_symmetry_restraint(Model *m,atom::Hierarchies hs)
{
 algebra::Vector3Ds translations=algebra::Vector3Ds();
 algebra::Transformation3Ds transformations=
 algebra::Transformation3Ds();
 int num_rotations;
 double angle;
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

domino::EquivalenceSubsetFilterTable* add_equivalence_filters
(std::vector<std::string> all,atom::Hierarchy h,Particles eq_ps)
{
 IMP_NEW(domino::EquivalenceSubsetFilterTable,f,());
 for(int i=0;i<all.size();++i){
  std::string name=all[i];
  std::vector<Particles> ps;
  atom::HierarchiesTemp mol=h.get_children();
  for(int j=0;j<mol.size();++j){
   if(mol[j]->get_name()==name){
    ParticlesTemp pp=mol[j].get_leaves();
    ps.push_back(pp);
   }
  }
  int ncopies=ps.size();
  if(ncopies==1){
   for(int j=0;j<(ps[0]).size();++j){
    eq_ps.push_back(ps[0][j]);
   }
  }
  if(ncopies>1){
   for(int j=0;j<(ps[0]).size();++j){
    Particles pps;
    for(int k=0;k<ncopies;++k){
     pps.push_back(ps[k][j]);
    }
    f->add_set(pps);
    eq_ps.push_back(pps[0]);
   }
  }
 }
 return f.release();
}

domino::ListSubsetFilterTable* setup_sampler
(Model *m, atom::Hierarchy h, algebra::Vector3Ds cover, double scale,
 domino::ParticleStatesTable *pst, domino::DominoSampler *s,
 domino::EquivalenceSubsetFilterTable *esft)
{
 IMP_NEW(domino::XYZStates,states,(cover));
 ParticlesTemp ps=h.get_leaves();
 for(int i=0;i<ps.size();++i){
  pst->set_particle_states(ps[i],states);
 }
 RestraintSet *rs=m->get_root_restraint_set();
 for(unsigned int i=0;i<rs->get_number_of_restraints();++i){
  if(cell_type=="square"){
   rs->get_restraint(i)->set_maximum_score(pow(scale,2));
  }else{
   rs->get_restraint(i)->set_maximum_score(1.45*pow(scale,2));
  }
 }
 domino::SubsetFilterTables fs=domino::SubsetFilterTables();
 IMP_NEW(domino::ListSubsetFilterTable,lsft,(pst));
 IMP_NEW(domino::RestraintScoreSubsetFilterTable,rssft,(m, pst));
 rssft->set_use_caching(false);
 fs.push_back(lsft);
 fs.push_back(rssft);
 fs.push_back(esft);
 domino::InteractionGraph ig=domino::get_interaction_graph(rs,pst);
 domino::SubsetGraph      jt=domino::get_junction_tree(ig);
 s->set_subset_filter_tables(fs);
 return lsft.release();
}

std::vector<Ints> get_mapping
(algebra::Vector3Ds cover0, algebra::Vector3Ds cover1)
{
 IMP_NEW(algebra::NearestNeighbor3D,nn,(cover0));
 std::vector<Ints> ret;
 ret.resize(cover0.size());
 for(unsigned int i=0;i<cover1.size();++i){
  unsigned int nns = nn->get_nearest_neighbor(cover1[i]);
  ret[nns].push_back(i);
 }
 return ret;
}

int main(int  , char **)
{

// cell dependent parameters
if(cell_type=="rhombus"){
 num_cells=21;
 num_copies=2;
 error_bound=1.45*pow(ds,2);
}else if(cell_type=="hexagon"){
 num_cells=7;
 num_copies=6;
 error_bound=1.45*pow(ds,2);
}else if(cell_type=="square"){
 num_cells=9;
 num_copies=6;
 side=sqrt(1.5*pow(side,2)*sqrt(3.0));
 error_bound=pow(ds,2);
}

// create a new model
IMP_NEW(Model,m,());

return 0;
}
