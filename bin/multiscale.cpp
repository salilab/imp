/**
 *  \file multiscale.cpp
 *  \brief Multiscale 2D sampling
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/base_types.h>
#include <IMP/domino.h>
#include <IMP/display.h>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <time.h>

using namespace IMP;

struct Parameters {
double ds;
double diameter;
double cm_dist;
double inter_dist;
int    niter;
int    nTMH;
int    stride;
bool   do_plot;
bool   do_statistics;
bool   do_native;
bool   do_random;
std::vector< std::pair<int,int> > TM_inter;
std::vector< std::pair<int,int> > TM_dist;
std::vector< double > TM_x0;
std::vector< double > native;
};

#define OPTION(type, name)                                   \
 type name=-1;                                              \
 desc.add_options()(#name, value< type >(&name), #name);

#define CHECK(type, name)                                    \
 IMP_USAGE_CHECK(name >=0, #name << " is " << name);

Parameters get_parameters(TextInput in){
#if BOOST_VERSION >= 104100
 using namespace boost::program_options;
 options_description desc;
 std::vector<std::string> inter,distance,native;
 bool do_plot,do_statistics,do_random;

 desc.add_options()("interacting", value< std::vector<std::string> >(),"ciao");
 desc.add_options()("display",     value< bool >(&do_plot),            "ciao");
 desc.add_options()("statistics",  value< bool >(&do_statistics),      "ciao");
 desc.add_options()("random",      value< bool >(&do_random),          "ciao");
 desc.add_options()("distance",    value< std::vector<std::string> >(),"ciao");
 desc.add_options()("native",      value< std::vector<std::string> >(),"ciao");


 OPTION(double, ds);
 OPTION(double, diameter);
 OPTION(double, cm_dist);
 OPTION(double, inter_dist);
 OPTION(int,    niter);
 OPTION(int,    number);
 OPTION(int,    stride);

 variables_map vm;
 store(parse_config_file(in.get_stream(), desc, false), vm);
 notify(vm);

 CHECK(double, ds);
 CHECK(double, diameter);
 CHECK(double, cm_dist);
 CHECK(double, inter_dist);
 CHECK(int,    niter);
 CHECK(int,    number);
 CHECK(int,    stride);

 Parameters ret;

 ret.ds=ds;
 ret.diameter=diameter;
 ret.cm_dist=cm_dist;
 ret.inter_dist=inter_dist;
 ret.niter=niter;
 ret.nTMH=number;
 ret.stride=stride;
 ret.do_plot=do_plot;
 ret.do_statistics=do_statistics;
 ret.do_random=do_random;
 ret.do_native=false;

 if (vm.count("interacting")){
  inter = vm["interacting"].as< std::vector<std::string> >();
  for(unsigned int i=0;i<inter.size();++i){
   std::vector<std::string> strs;
   boost::split(strs, inter[i], boost::is_any_of("\t "));
   int pos1 = atoi(strs[0].c_str());
   int pos2 = atoi(strs[1].c_str());
   ret.TM_inter.push_back(std::pair<int,int> (pos1,pos2));
  }
 }

 if (vm.count("distance")){
  distance = vm["distance"].as< std::vector<std::string> >();
  for(unsigned int i=0;i<distance.size();++i){
   std::vector<std::string> strs;
   boost::split(strs, distance[i], boost::is_any_of("\t "));
   int    pos1  = atoi(strs[0].c_str());
   int    pos2  = atoi(strs[1].c_str());
   double x0    = atof(strs[2].c_str());
   ret.TM_dist.push_back(std::pair<int,int> (pos1,pos2));
   ret.TM_x0.push_back(x0);
  }
 }

 if (vm.count("native")){
  ret.do_native=true;
  native = vm["native"].as< std::vector<std::string> >();
  for(unsigned int i=0;i<native.size();++i){
   std::vector<std::string> strs;
   boost::split(strs, native[i], boost::is_any_of("\t "));
   int    pos1  = atoi(strs[0].c_str());
   int    pos2  = atoi(strs[1].c_str());
   double dist  = atof(strs[2].c_str());
   int idpair=pos1*number-pos1*(pos1+1)/2+pos2-(pos1+1);
   if(idpair==i){
    ret.native.push_back(dist);
   }else{
    std::cerr << "Wrong order for native distances" << std::endl;
    exit;
   }
  }
 }

return ret;
#else
 IMP_FAILURE("Need newer boost");
#endif
}

core::PairRestraint* add_distance_restraint
 (Model *m, Particle *p0, Particle *p1,
  double x0, std::string name, double max_score)
{
 IMP_NEW(core::HarmonicUpperBound,hub,(x0,1.0));
 IMP_NEW(core::DistancePairScore,df,(hub));
 IMP_NEW(core::PairRestraint,dr,(df, ParticlePair(p0, p1)));
 dr->set_name(name);
 m->add_restraint(dr);
 m->set_maximum_score(dr, max_score);
 return dr.release();
}

void add_distance_restraint2
 (Model *m, Particle *p0, Particle *p1,
  double x0, double kappa, double max_score)
{
 IMP_NEW(core::Harmonic,h,(x0,kappa));
 IMP_NEW(core::DistancePairScore,df,(h));
 IMP_NEW(core::PairRestraint,dr,(df, ParticlePair(p0, p1)));
 m->add_restraint(dr);
 m->set_maximum_score(dr, max_score);
 dr->set_name("DISTANCE");
}

void add_diameter_restraint
(Model *m, Particles all,
 double diameter, double max_score)
{
 int n=all.size();
 for(int i=0;i<n-1;++i){
  for(int j=i+1;j<n;++j){
    IMP_NEW(core::HarmonicUpperBound,hub,(diameter,1.0));
    IMP_NEW(core::DistancePairScore,df,(hub));
    IMP_NEW(core::PairRestraint,dr,(df, ParticlePair(all[i], all[j])));
    std::string name0=core::XYZR(all[i])->get_name();
    std::string name1=core::XYZR(all[j])->get_name();
    dr->set_name("Diameter"+name0+"-"+name1);
    m->add_restraint(dr);
    m->set_maximum_score(dr,max_score);
  }
 }
}

domino::ListSubsetFilterTable* setup
(Model *m, Particles all, RestraintSet *rset,
domino::ParticleStatesTable *pst, domino::DominoSampler *s,
algebra::Vector3Ds cover, algebra::Vector3Ds cover_x, double scale)
{
 IMP_NEW(domino::XYZStates,st1,(cover_x));
 IMP_NEW(domino::XYZStates,st2,(cover));
 algebra::Vector3Ds origin = algebra::Vector3Ds();
 origin.push_back(algebra::Vector3D(0.0,0.0,0.0));
 IMP_NEW(domino::XYZStates,st0,(origin));

 for(int i=0;i<all.size();++i){
  if(i==0) pst->set_particle_states(all[i],st0);
  if(i==1) pst->set_particle_states(all[i],st1);
  if(i>1)  pst->set_particle_states(all[i],st2);
 }

 RestraintSet *rs=m->get_root_restraint_set();
 for(unsigned int i=0;i<rs->get_number_of_restraints();++i)
   rs->get_restraint(i)->set_maximum_score(0.9*scale*scale);

 IMP_NEW(domino::ListSubsetFilterTable,lsft,(pst));
 IMP_NEW(domino::RestraintScoreSubsetFilterTable,rssft,(m, pst));
 IMP_NEW(domino::ExclusionSubsetFilterTable,esft,(pst));
 domino::SubsetFilterTables fs=domino::SubsetFilterTables();
 fs.push_back(lsft);
 fs.push_back(rssft);
 if(scale<=12.0) fs.push_back(esft);
 domino::InteractionGraph ig=domino::get_interaction_graph(rset,pst);
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

IMP_NEW(Model,m,());
Particles all;

// read parameters from file
Parameters mydata=get_parameters("config.ini");

double ds=mydata.ds;
double max_score_=0.9*ds*ds;
double diameter=mydata.diameter;
double cm_dist=mydata.cm_dist;
double inter_dist=mydata.inter_dist;
int    niter=mydata.niter;
int    nTMH=mydata.nTMH;
int    stride;
bool   do_plot=mydata.do_plot;
bool   do_statistics=mydata.do_statistics;
std::vector< std::pair<int,int> > TM_inter=mydata.TM_inter;
srand ( time(NULL) );

std::cout << "creating representation" << std::endl;
for(int i=0;i<nTMH;++i){
    IMP_NEW(Particle,p,(m));
    core::XYZR xyz=core::XYZR::setup_particle(p,
    algebra::Sphere3D(algebra::Vector3D(0,0,0),3.0));
    std::stringstream ss;
    ss << i;
    xyz->set_name("TM"+ss.str());
    all.push_back(p);
}

std::cout << "creating restraints" << std::endl;
IMP_NEW(RestraintSet,rset,());
//distance between adjacent helices
for(int i=0;i<nTMH-1;++i){
 std::stringstream ss1, ss2;
 ss1 << i;
 ss2 << i+1;
 core::PairRestraint* dr=add_distance_restraint(m,all[i],all[i+1],cm_dist,
  "distance TM"+ss1.str()+"-TM"+ss2.str(),max_score_);
 rset->add_restraint(dr);
}
// interacting helices
for(int i=0;i<TM_inter.size();++i){
 int i0=TM_inter[i].first;
 int i1=TM_inter[i].second;
 std::string name0=all[i0]->get_name();
 std::string name1=all[i1]->get_name();
 core::PairRestraint* dr=
 add_distance_restraint(m,all[i0],all[i1],inter_dist,
  "interaction "+name0+"-"+name1,max_score_);
 rset->add_restraint(dr);
}

// diameter
add_diameter_restraint(m,all,diameter,max_score_);

// additional distance restraints
for(int i=0;i<mydata.TM_dist.size();++i){
 int i0=mydata.TM_dist[i].first;
 int i1=mydata.TM_dist[i].second;
 double x0=mydata.TM_x0[i];
 add_distance_restraint2(m,all[i0],all[i1],x0,1.0,max_score_);
}

std::cout << "creating states" << std::endl;
algebra::BoundingBox2D bb=
algebra::BoundingBox2D(algebra::Vector2D(-diameter,-diameter),
                       algebra::Vector2D(diameter,diameter));

algebra::BoundingBox2D bbx=
algebra::BoundingBox2D(algebra::Vector2D(-diameter,0.0),
                       algebra::Vector2D(diameter,0.0));

std::vector<algebra::Vector3Ds> cover, cover_x;
for(int i=0;i<niter;++i)
{
 std::vector<algebra::Vector2D> cur=
 algebra::get_grid_interior_cover_by_spacing(bb, ds/pow(2,i));
 algebra::Vector3Ds grid=algebra::Vector3Ds();
 for(int j=0;j<cur.size();++j)
  grid.push_back(algebra::Vector3D((cur[j])[0],(cur[j])[1],0.0));
 cover.push_back(grid);

 std::vector<algebra::Vector2D> cur_x=
 algebra::get_grid_interior_cover_by_spacing(bbx, ds/pow(2,i));
 algebra::Vector3Ds grid_x=algebra::Vector3Ds();
 for(int j=0;j<cur_x.size();++j)
  if((cur_x[j])[0]>=0.0)
   grid_x.push_back(algebra::Vector3D((cur_x[j])[0],(cur_x[j])[1],0.0));
 cover_x.push_back(grid_x);
}

// let's go with domino
IMP_NEW(domino::ParticleStatesTable,pst,());
IMP_NEW(domino::DominoSampler,s,(m,pst));

domino::ListSubsetFilterTable* lf=setup
(m,all,rset,pst,s,cover[0],cover_x[0],ds);

domino::Subset subs=domino::Subset(pst->get_particles());
domino::Assignments ass=s->get_sample_assignments(subs);

std::cout << "for scale " << ds << " got " << ass.size() <<
 " out of " << pow(cover[0].size(),nTMH-2)*cover_x[0].size() << std::endl;

//next iterations
for(int curi=1;curi<niter;++curi){
 double scale = ds/pow(2,curi);
 std::vector<Ints> mapping=
  get_mapping(cover[curi-1], cover[curi]);
 std::vector<Ints> mapping_x=
  get_mapping(cover_x[curi-1], cover_x[curi]);

 IMP_NEW(domino::ParticleStatesTable,pst,());
 IMP_NEW(domino::DominoSampler,s,(m,pst));

 domino::ListSubsetFilterTable* lf=setup
  (m,all,rset,pst,s,cover[curi],cover_x[curi],scale);

 domino::Assignments cac;

 if(mydata.do_random){
  stride=1;
 }else{
  stride=mydata.stride;
 }

 for(unsigned int j=0;j<ass.size();j=j+stride){
  //int rr=rand() % mydata.stride + 1;
  //std::cout << j << " " << rr <<std::endl;
  if(mydata.do_random && (rand() % mydata.stride + 1)!=1) continue;
  domino::Assignment a=ass[j];
  unsigned int outof=1;
  for(int i=0;i<a.size();++i)
  {
   int s=a[i];
   Particle *p=subs[i];
   std::string name=core::XYZR(p)->get_name();
   Ints allowed;
   if       (name=="TM0"){ allowed.push_back(s);
   }else if (name=="TM1"){ allowed= mapping_x[s];
   }else{                  allowed= mapping[s];
   }
   //std::cout << j << " " << name << " " << allowed << std::endl;
   lf->set_allowed_states(p,allowed);
   outof*=allowed.size();
  }
  domino::Assignments ccac=s->get_sample_assignments(subs);
  if (ccac.size()>0){
   cac.insert( cac.end(), ccac.begin(), ccac.end() );
   //std::cout << "doing " << j << " solutions " << ccac.size() <<
   //  " out of " << outof << " tot " << cac.size() << std::endl;
  }
 }

 ass= cac;
 std::cout << "for scale " << scale << " got " << ass.size() << " out of "
 << pow(cover[curi].size(),nTMH-2)*cover_x[curi].size() << std::endl;

 if(do_statistics){
  double mean_dist[nTMH*(nTMH-1)/2];
  double mean_dist2[nTMH*(nTMH-1)/2];
  double mean_drmsd=0.0;
  double mean_drmsd2=0.0;
  double min_drmsd=10000.0;
  int    min_index;
  for(int i=0;i<nTMH*(nTMH-1)/2;++i){
   mean_dist[i]=0.0;
   mean_dist2[i]=0.0;
  }
  for(unsigned int i=0;i<ass.size();++i){
   domino::load_particle_states(subs, ass[i], pst);
   double drmsd=0.0;
   for(int i1=0;i1<nTMH-1;++i1){
    for(int i2=i1+1;i2<nTMH;++i2){
     int idpair=i1*nTMH-i1*(i1+1)/2+i2-(i1+1);
     core::XYZ p1=core::XYZ(all[i1]);
     core::XYZ p2=core::XYZ(all[i2]);
     double dist=core::get_distance(p1,p2);
     mean_dist[idpair]+=dist;
     mean_dist2[idpair]+=dist*dist;
     if(mydata.do_native) drmsd+=pow(dist-mydata.native[idpair],2);
    }
   }
   if(mydata.do_native){
    drmsd=sqrt(drmsd/(nTMH*(nTMH-1)/2));
    mean_drmsd+=drmsd;
    mean_drmsd2+=drmsd*drmsd;
    if(drmsd<min_drmsd){min_drmsd=drmsd; min_index=i;}
   }
  }
  for(int i1=0;i1<nTMH-1;++i1){
   for(int i2=i1+1;i2<nTMH;++i2){
    std::string name1=core::XYZR(all[i1])->get_name();
    std::string name2=core::XYZR(all[i2])->get_name();
    int idpair=i1*nTMH-i1*(i1+1)/2+i2-(i1+1);
    double ave=mean_dist[idpair]/double(ass.size());
    double ave2=mean_dist2[idpair]/double(ass.size());
    double sig=sqrt(ave2-ave*ave);
    std::cout << name1 << " " << name2 << " " << idpair << " "
              << ave << " " << sig << std::endl;
   }
  }
  if(mydata.do_native){
   double ave=mean_drmsd/double(ass.size());
   double ave2=mean_drmsd2/double(ass.size());
   double sig=sqrt(ave2-ave*ave);
   std::cout << "** DRMSD:: AVE " << ave << " SIGMA " << sig
   << " MIN " << min_drmsd << " INDEX " << min_index << std::endl;
   std::cout << std::endl;
  }
 }

if(do_plot && curi==niter-1){
  std::string file="solutions.pym";
  IMP_NEW(display::PymolWriter,sw,(file));
  int ii=0;
  for(unsigned int i=0;i<ass.size();++i){
   domino::load_particle_states(subs, ass[i], pst);
   sw->set_frame(ii);
   ii++;
   for(int j=0;j<(ass[i]).size();++j){
    Particle *p=subs[j];
    IMP_NEW(display::XYZRGeometry,g,(p));
    std::string name=core::XYZR(p)->get_name();
    g->set_name(name);
    g->set_color(display::get_jet_color(1.0/double(j+1)));
    sw->add_geometry(g);
   }
  }
 }
}

return 0;
}
