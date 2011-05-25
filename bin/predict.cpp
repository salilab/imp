/**
 *  \file predict.cpp
 *  \brief Predict interaction
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <boost/program_options.hpp>
#include <IMP/base_types.h>
#include <string>
#include <iostream>
using namespace IMP;

// Parameters structure
struct Parameters {
std::string    score_name;
double tilt;
double swing;
double rot;
double x;
double tiltmax;
double swingmax;
double rotmax;
double xmin;
double xmax;
int num;
std::vector<char>                 seq;
std::vector<double>               topo;
std::vector<std::string>          name;
std::vector<double>               hl;
std::vector<std::pair<int,int> >  resid;
};

// convert degrees to radians
double radians (double d);
inline double radians (double d) {return d * IMP::PI/ 180.0;}

#define OPTION(type, name)                                   \
 type name=-1;                                              \
 desc.add_options()(#name, value< type >(&name), #name);

#define CHECK(type, name)                                    \
 IMP_USAGE_CHECK(name >=0, #name << " is " << name);

Parameters get_parameters(TextInput in){
#if BOOST_VERSION >= 104100
 using namespace boost::program_options;
 options_description desc;
 std::string score_name,sequence;
 std::vector<std::string> res;

 desc.add_options()("score_name",  value< std::string >(&score_name),  "ciao");
 desc.add_options()("sequence",    value< std::string >(&sequence),    "ciao");
 desc.add_options()("residues",    value< std::vector<std::string> >(),"ciao");
 desc.add_options()("topology",    value< std::vector<double> >(),     "ciao");

 OPTION(double, tilt);
 OPTION(double, swing);
 OPTION(double, rot);
 OPTION(double, x);
 OPTION(double, tiltmax);
 OPTION(double, swingmax);
 OPTION(double, rotmax);
 OPTION(double, xmin);
 OPTION(double, xmax);
 OPTION(int,    number);

 variables_map vm;
 store(parse_config_file(in.get_stream(), desc, false), vm);
 notify(vm);

 CHECK(double, tilt);
 CHECK(double, swing);
 CHECK(double, rot);
 CHECK(double, x);
 CHECK(double, tiltmax);
 CHECK(double, swingmax);
 CHECK(double, rotmax);
 CHECK(double, xmin);
 CHECK(double, xmax);
 CHECK(int,    number);

 Parameters ret;

// General Parameters
 ret.score_name=score_name;
// Grid Parameters
 ret.tilt=radians(tilt);
 ret.swing=radians(swing);
 ret.rot=radians(rot);
 ret.x=x;
 ret.tiltmax=radians(tiltmax);
 ret.swingmax=radians(swingmax);
 ret.rotmax=radians(rotmax);
 ret.xmin=xmin;
 ret.xmax=xmax;
// HelixData
 ret.num=number;

 std::vector<char> v(sequence.begin(), sequence.end());
 ret.seq = v;

 if (vm.count("residues")){
  res = vm["residues"].as< std::vector<std::string> >();
  for(unsigned int i=0;i<res.size();++i){
   std::vector<std::string> strs;
   boost::split(strs, res[i], boost::is_any_of("\t "));
   ret.resid.push_back
   (std::pair<int,int> (atoi(strs[0].c_str()),atoi(strs[1].c_str())));
  }
 }

 if (vm.count("topology")){
  ret.topo = vm["topology"].as< std::vector<double> >();
 }

if(ret.num!=ret.topo.size())
IMP_FAILURE("Check the topology list!");
if(ret.num!=ret.resid.size())
IMP_FAILURE("Check the residues list!");

// storing half length and name
for(int i=0;i<ret.num;++i){
 ret.hl.push_back(ret.topo[i]*
  double(ret.resid[i].second-ret.resid[i].first)*1.51/2.0);
 std::stringstream ss;
 ss << i;
 ret.name.push_back("TM"+ss.str());
 //std::cout << i << " " << ret.resid[i].first << " " <<
 //ret.resid[i].second << " " << ret.topo[i] << " " << ret.name[i] << std::endl;
}

return ret;
#else
 IMP_FAILURE("Need newer boost");
#endif
}

core::TableRefiner* create_helix(Model *m,
 atom::Hierarchy protein, Parameters *TM)
{
int nres,jseq;
double x,y,z;
atom::ResidueType restype;
IMP_NEW(core::TableRefiner,tbr,());

for(int i=0;i<TM->num;++i){
 IMP_NEW(Particle,pm,(m));
 atom::Molecule tm=atom::Molecule::setup_particle(pm);
 tm->set_name(TM->name[i]);
 nres=TM->resid[i].second-TM->resid[i].first+1;
 core::XYZs atoms;
// cycle on the number of residues
 //std::cout << i << " " << nres << std::endl;
 for(int j=0;j<nres;++j){
  // set up residue
  IMP_NEW(Particle,pr,(m));
  // residue index and type
  jseq=TM->resid[i].first+j;
  restype=atom::get_residue_type(TM->seq[jseq-1]);
  //std::cout << jseq << " " << restype << std::endl;
  atom::Residue r=atom::Residue::setup_particle(pr,restype,jseq);
  //set up atom
  IMP_NEW(Particle,pa,(m));
  atom::Atom a=atom::Atom::setup_particle(pa,atom::AT_CA);
  x=2.3*cos(radians(100.0)*double(j));
  y=2.3*sin(radians(100.0)*double(j));
  z=TM->topo[i]*1.51*(double(j)-double((nres-1))/2.0);
  core::XYZR ad=core::XYZR::setup_particle
  (pa,algebra::Sphere3D(algebra::Vector3D(x,y,z),2.273));
  r.add_child(a);
  tm.add_child(r);
  atoms.push_back(ad);
 }
 protein.add_child(tm);
 // create rigid body
 IMP_NEW(Particle,prb,(m));
 core::RigidBody rb=core::RigidBody::setup_particle(prb,atoms);
 rb->set_name(TM->name[i]);
 // TableRefiner
 tbr->add_particle(prb,atoms);
 }
return tbr.release();
}

double dihedral
(algebra::Vector3D x0, algebra::Vector3D x1,
 algebra::Vector3D x2, algebra::Vector3D x3)
{
 double cosangle, angle, scalar_product, mag_product;
 algebra::Vector3D rij = x0 - x1;
 algebra::Vector3D rkj = x2 - x1;
 algebra::Vector3D rkl = x2 - x3;

 algebra::Vector3D v1 =  algebra::get_vector_product(rij, rkj);
 algebra::Vector3D v2 =  algebra::get_vector_product(rkj, rkl);

 scalar_product = v1.get_scalar_product(v2);
 mag_product = v1.get_magnitude() * v2.get_magnitude();

 if ( mag_product < 0.001 ){
  cosangle = 0.0;
 }else{
  cosangle = scalar_product / mag_product;
 }
 cosangle = std::max(std::min(cosangle, 1.0), -1.0);
 angle = acos(cosangle);
 algebra::Vector3D v0 = algebra::get_vector_product(v1, v2);
 double sign = rkj.get_scalar_product(v0);
 if (sign < 0.0) angle = -angle;
 return angle;
}

container::PairsRestraint* add_dope_restraint
(Model *m, Particle *rb0, Particle *rb1,
 core::TableRefiner *tbr, std::string sname)
{
 IMP_NEW(container::ListSingletonContainer,sc0,(m));
 IMP_NEW(container::ListSingletonContainer,sc1,(m));
 sc0->add_particles(tbr->get_refined(rb0));
 sc1->add_particles(tbr->get_refined(rb1));
 IMP_NEW(container::AllBipartitePairContainer,cpc,(sc0,sc1));
 IMP_NEW(atom::DopePairScore,dps,(15.0,atom::get_data_path(sname)));
 IMP_NEW(container::PairsRestraint,dope,(dps,cpc));
 m->add_restraint(dope);
 return dope.release();
}

core::ExcludedVolumeRestraint* add_excluded_volume
(Model *m, Particle *rb0, Particle *rb1, core::TableRefiner *tbr)
{
 IMP_NEW(container::ListSingletonContainer,lsc,(m));
 lsc->add_particles(tbr->get_refined(rb0));
 lsc->add_particles(tbr->get_refined(rb1));
 IMP_NEW(core::ExcludedVolumeRestraint,evr,(lsc,1000.0));
 m->add_restraint(evr);
 return evr.release();
}


void do_transform
(core::RigidBody rb, double psi, double theta, double phi, double dx)
{
 algebra::Rotation3D rot0=
 algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0), IMP::PI/2.0);
 algebra::Rotation3D rotz=
 algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1), psi);
 algebra::Rotation3D tilt=
 algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0), theta);
 algebra::Rotation3D swing=
 algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1), phi);
 algebra::Rotation3D rot=
 algebra::compose(swing,algebra::compose(tilt,algebra::compose(rotz,rot0)));
 algebra::Transformation3D tr=
 algebra::Transformation3D(rot,algebra::Vector3D(dx,0.0,0.0));
 rb.set_reference_frame(algebra::ReferenceFrame3D(tr));
 }

int main(int  , char **)
{

// parsing input
//std::cout << "Parsing input file" << std::endl;
Parameters mydata=get_parameters("config.ini");

// create a new model
IMP_NEW(Model,m,());
// root hierarchy
IMP_NEW(Particle,ph,(m));
atom::Hierarchy all=atom::Hierarchy::setup_particle(ph);

// create representation
//std::cout << "Creating representation" << std::endl;
core::TableRefiner* tbr=create_helix(m,all,&mydata);

// list of rbs
core::RigidBodies rbs;
for(int i=0;i<mydata.num;++i){
 atom::Selection s=atom::Selection(all);
 s.set_molecule(mydata.name[i]);
 rbs.push_back(core::RigidMember
 (s.get_selected_particles()[0]).get_rigid_body());
}

// create restraints
atom::add_dope_score_data(all);
std::vector<container::PairsRestraint*>     dopes;
std::vector<core::ExcludedVolumeRestraint*> evs;

for(int i=0;i<mydata.num-1;++i){
 for(int j=i+1;j<mydata.num;++j){
  dopes.push_back(add_dope_restraint(m,rbs[i],rbs[j],tbr,mydata.score_name));
  evs.push_back(add_excluded_volume(m,rbs[i],rbs[j],tbr));
 }
}

double om0[]={-156.5, 146.4, -37.9, 13.8, 178.0, 25.5, -161.1,
                44.8, 127.4, -60.2, -129.2, 2.4, 161.0};
double sig_om0[]={10.1, 13.6, 7.50, 16.6, 20.8, 11.2, 10.3,
                 8.8, 12.3, 14.8, 12.9, 16.2, 17.6};
double dd0[]={8.61, 8.57, 7.93, 9.77, 9.14, 8.55, 9.30,
              7.96, 9.40, 8.61, 8.97, 8.55, 8.75};
double sig_dd0[]={0.89, 0.99, 0.88, 1.18, 1.47, 1.05, 1.57,
                1.13, 1.0, 1.04, 1.65, 0.78, 1.33};
double nsig=3.0;
int ncl=13;
std::vector<double> om_b,om_e,dd_b,dd_e;

for(int i=0;i<ncl;++i){
 dd_b.push_back(dd0[i]-nsig*sig_dd0[i]);
 dd_e.push_back(dd0[i]+nsig*sig_dd0[i]);
 om_b.push_back(std::max(radians(om0[i]-nsig*sig_om0[i]),-IMP::PI));
 om_e.push_back(std::min(radians(om0[i]+nsig*sig_om0[i]),IMP::PI));
}

std::vector<double> score, minscore;
for(int i=0;i<mydata.num-1;++i){
 for(int j=i+1;j<mydata.num;++j){
  score.push_back(0.0);
  minscore.push_back(1000000.0);
 }
}

int itilt=int(mydata.tiltmax/mydata.tilt);
int iswing=int(mydata.swingmax/mydata.swing);
int irot=int(mydata.rotmax/mydata.rot);
int ix=int((mydata.xmax-mydata.xmin)/mydata.x);

//std::cout << itilt << " " << iswing << " " << irot << " " << ix << std::endl;
double t0,p0,s0,t1,p1,s1,xx1;
algebra::Vector3D A, B;
int idpair;

for(int id1=0;id1<mydata.num-1;++id1){
 for(int id2=id1+1;id2<mydata.num;++id2){
  idpair= id1*mydata.num-id1*(id1+1)/2+id2-(id1+1);
  for(int i0=0;i0<itilt+1;++i0){
   t0=double(i0)*mydata.tilt;
   for(int j0=0;j0<iswing;++j0){
    if ( i0 == 0 && j0 != 0 ) break;
    p0=double(j0)*mydata.swing;
    algebra::Vector3D B0=
    algebra::Vector3D(-mydata.hl[id1]*sin(t0)*cos(p0),
                      -mydata.hl[id1]*sin(t0)*sin(p0),
                      -mydata.hl[id1]*cos(t0));
    algebra::Vector3D E0=
    algebra::Vector3D(mydata.hl[id1]*sin(t0)*cos(p0),
                      mydata.hl[id1]*sin(t0)*sin(p0),
                      mydata.hl[id1]*cos(t0));
    algebra::Segment3D B0E0 = algebra::Segment3D(B0,E0);
    for(int x1=0;x1<ix+1;++x1){
     xx1=mydata.xmin+double(x1)*mydata.x;
     for(int i1=0;i1<itilt+1;++i1){
      t1=double(i1)*mydata.tilt;
      for(int j1=0;j1<iswing;++j1){
       if ( i1 == 0 && j1 != 0 ) break;
       p1=double(j1)*mydata.swing;
       algebra::Vector3D B1=
       algebra::Vector3D(-mydata.hl[id2]*sin(t1)*cos(p1)+xx1,
                         -mydata.hl[id2]*sin(t1)*sin(p1),
                         -mydata.hl[id2]*cos(t1));
       algebra::Vector3D E1=
       algebra::Vector3D(mydata.hl[id2]*sin(t1)*cos(p1)+xx1,
                         mydata.hl[id2]*sin(t1)*sin(p1),
                         mydata.hl[id2]*cos(t1));
       algebra::Segment3D B1E1 = algebra::Segment3D(B1,E1);

       algebra::Segment3D shorto=algebra::get_shortest_segment(B0E0,B1E1);
       algebra::Vector3D  T0=shorto.get_point(0);
       algebra::Vector3D  T1=shorto.get_point(1);
       double             length=shorto.get_length();
       double             sign=1.0;
       A=E0;
       B=E1;
       if((E0-T0).get_magnitude() < (B0-T0).get_magnitude()){
        A=B0;
        sign *= -1.0;
       }
       if((E1-T1).get_magnitude() < (B1-T1).get_magnitude()){
        B=B1;
        sign *= -1.0;
       }
       double omega=dihedral(A,T0,T1,B) + (sign-1.0)*IMP::PI/2.0;
       if ( omega < -IMP::PI ) omega +=2.0*IMP::PI;

       for(unsigned int i=0;i<om_b.size();++i){
        if ( omega > om_b[i] && omega < om_e[i] &&
             length > dd_b[i] && length < dd_e[i] ){
         for(int k0=0;k0<irot;++k0){
           s0=double(k0)*mydata.rot;
           do_transform(rbs[id1],s0,t0,p0,0.0);
           for(int k1=0;k1<irot;++k1){
            s1=double(k1)*mydata.rot;
            do_transform(rbs[id2],s1,t1,p1,xx1);
            double tmpscore=evs[idpair]->evaluate(false);
            if ( tmpscore < 0.01 ){
             double dopescore=dopes[idpair]->evaluate(false);
             score[idpair] += exp(-dopescore);
             if ( dopescore < minscore[idpair] ) minscore[idpair]=dopescore;
            }
           }
         }
         break;
        }
       }
      }
     }
    }
   }
  }
 }
}

std::cout << "** FINAL SCORE" << std::endl;
for(int id1=0;id1<mydata.num-1;++id1){
 for(int id2=id1+1;id2<mydata.num;++id2){
        idpair= id1*mydata.num-id1*(id1+1)/2+id2-(id1+1);
        std::cout << id1 << " " << id2 << " " << -log(score[idpair])
        << " " << minscore[idpair] << std::endl;
 }
}


return 0;
}
