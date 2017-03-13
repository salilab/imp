/**
 *  \file spb_analysis.cpp
 *  \brief SPB Analysis Tool
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/statistics.h>
#include <IMP/rmf.h>
#include <IMP/isd.h>
#include "mpi.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>
#include <string>
#include <IMP/base/Pointer.h>

using namespace IMP;
using namespace IMP::membrane;

Particles generate_new_particles
 (Model *m, const atom::Hierarchies& all_mol, double side,
 double off_x, double off_y,
 double xmin, double xmax, double ymin, double ymax)
{
 std::string gfp_name = "GFP";
 // first pick particles in hierarchies 0, 1 and 2
 atom::Hierarchies hhs;
 for(unsigned i=0;i<3;++i){hhs.push_back(all_mol[i]);}
 // select from them
 atom::Selection s=atom::Selection(hhs);
 Particles ps0=s.get_selected_particles();
 // now create new particles
 Particles ps;
 for(int i=-2; i<3; ++i){
  for(int j=-2; j<3; ++j){
   // cell translations
   double dx = static_cast<double>(i) * 1.5 * side;
   double dy = side * sqrt(3.) * (static_cast<double>(j)
     +static_cast<double>(i%2)/2.);
   for(unsigned k=0; k<ps0.size(); ++k){
    // old coordinates
    algebra::Vector3D xyz = core::XYZR(ps0[k]).get_coordinates();
    // radius
    double radius = core::XYZR(ps0[k]).get_radius();
    // mass
    double mass = atom::Mass(ps0[k]).get_mass();
    // and name
    std::string name =
     atom::Molecule(atom::Domain(ps0[k]).get_parent())->get_name();
    // skip GFP
    if(name.find(gfp_name) != std::string::npos){ continue; }
    // coordinates of the new particle
    double xnew = xyz[0] + dx + off_x;
    double ynew = xyz[1] + dy + off_y;
    double znew = xyz[2];
    // check if within boundaries
    if(xnew > xmin && xnew <= xmax && ynew > ymin && ynew <= ymax){
     // create new particle
     IMP_NEW(Particle,p,(m));
     // and decorate with old attributes
     core::XYZR d=core::XYZR::setup_particle(p);
     d.set_coordinates(algebra::Vector3D(xnew, ynew, znew));
     d.set_radius(radius);
     atom::Mass ma=atom::Mass::setup_particle(p,mass);
     // plus molecule
     atom::Molecule mol = atom::Molecule::setup_particle(p);
     mol->set_name(name);
     // and domain
     atom::Domain dom = atom::Domain::setup_particle(p, IntRange(0, 1));
     dom->set_name(atom::Domain(ps0[k])->get_name());
     // add to list
     ps.push_back(p);
    }
   }
  }
 }
 return ps;
};

std::vector< std::array<int,3> > get_indices(
  core::XYZR xyzr, MapParameters Map){

 // list of indices
 std::vector< std::array<int,3>  > ijk_s;
 // get positions
 algebra::Vector3D xyz = xyzr.get_coordinates();
 // number of bins forming the cube in which the bead is inscribed
 int nbin = static_cast<int>(xyzr.get_radius()/Map.resolution)+1;
 // indices of the bead center
 int i = static_cast<int>(floor((xyz[0] - Map.xmin)/Map.resolution));
 int j = static_cast<int>(floor((xyz[1] - Map.ymin)/Map.resolution));
 int k = static_cast<int>(floor((xyz[2] - Map.zmin)/Map.resolution));

 for(int ii=i-nbin; ii<=i+nbin; ++ii){
  if(ii<0 || ii>Map.nbinx){continue;}
  for(int jj=j-nbin; jj<=j+nbin; ++jj){
   if(jj<0 || jj>Map.nbiny){continue;}
   for(int kk=k-nbin; kk<=k+nbin; ++kk){
     if(kk<0 || kk>Map.nbinz){continue;}

     double xx = Map.xmin + static_cast<double>(ii) * Map.resolution;
     double yy = Map.ymin + static_cast<double>(jj) * Map.resolution;
     double zz = Map.zmin + static_cast<double>(kk) * Map.resolution;

     double dist = sqrt( (xyz[0]-xx) * (xyz[0]-xx) +
                         (xyz[1]-yy) * (xyz[1]-yy) +
                         (xyz[2]-zz) * (xyz[2]-zz) );

     // add indices to vector
     if( dist < xyzr.get_radius() ){
      std::array<int,3> ijk = {{ii, jj, kk}};
      ijk_s.push_back(ijk);
     }
   }
  }
 }
 return ijk_s;
}

std::map< std::pair<std::array<int,3>,std::string>, double> get_map
 (Particles ps, MapParameters Map){

 std::map< std::pair<std::array<int,3>,std::string>, double> new_map;

 // cycle on all the particles
 for(unsigned i=0; i<ps.size(); ++i){
  // get the name of the domain
  std::string name_ps = atom::Domain(ps[i])->get_name();
  // map the bead name to a density name
  std::string name_map;
  // associate each domain to a density map
  std::size_t found;
  found = name_ps.find("Spc110");
  if (found!=std::string::npos) name_map="Spc110p";
  found = name_ps.find("Cmd1");
  if (found!=std::string::npos) name_map="Cmd1p";
  found = name_ps.find("Cnm67");
  if (found!=std::string::npos) name_map="Cnm67p";
  found = name_ps.find("Spc42_CC");
  if (found!=std::string::npos) name_map="Spc42_CC";
  found = name_ps.find("Spc42p_c0");
  if (found!=std::string::npos) name_map="Spc42p_c0";
  found = name_ps.find("Spc42p_c1");
  if (found!=std::string::npos) name_map="Spc42p_c1";
  found = name_ps.find("Spc42p_c2");
  if (found!=std::string::npos) name_map="Spc42p_c2";
  found = name_ps.find("Spc42p_n0");
  if (found!=std::string::npos) name_map="Spc42p_n0";
  found = name_ps.find("Spc29p_n0");
  if (found!=std::string::npos) name_map="Spc29p_n0";
  found = name_ps.find("Spc29p_n1");
  if (found!=std::string::npos) name_map="Spc29p_n1";
  found = name_ps.find("Spc29p_n2");
  if (found!=std::string::npos) name_map="Spc29p_n2";
  found = name_ps.find("Spc29p_c0");
  if (found!=std::string::npos) name_map="Spc29p_c0";
  found = name_ps.find("Spc29p_c1");
  if (found!=std::string::npos) name_map="Spc29p_c1";
  found = name_ps.find("Spc29p_c2");
  if (found!=std::string::npos) name_map="Spc29p_c2";

  std::vector< std::array<int,3> > ijk_s = get_indices(core::XYZR(ps[i]), Map);
  for(unsigned j=0; j<ijk_s.size(); ++j){
   // creating the keys from indices and name
   std::pair<std::array<int,3>,std::string> key = std::make_pair(
     ijk_s[j], name_map);
   // adding mass
   new_map[key] += 1.0;
  }
 }
 return new_map;
}


void add_to_map (std::map< std::pair<std::array<int,3>,std::string>,
  double>& map_from,
                 std::map< std::pair<std::array<int,3>,std::string>,
   double>& map_to,double weight){

 std::map< std::pair<std::array<int,3>,std::string>, double>::iterator iit;

 for(iit=map_from.begin(); iit!=map_from.end(); ++iit){
  if(map_to.count((*iit).first)==0) map_to[(*iit).first]
   = weight * (*iit).second;
  else
   map_to[(*iit).first] += weight * (*iit).second;
 }
}


void print_map(std::map< std::pair<std::array<int,3>,std::string>, double>& map,
               MapParameters Map, double normalization, std::string label){

 FILE *mapfile;
 std::string suff = ".dx";
 mapfile = fopen((label+suff).c_str(),"w");
 // write header
 int totbin = (Map.nbinx+1) * (Map.nbiny+1) * (Map.nbinz+1);
 fprintf(mapfile, "# Data produce by spb_density\n");
 fprintf(mapfile, "%35s %3d %3d %3d\n", "object 1 class gridpositions counts"
   ,Map.nbinx+1,Map.nbiny+1,Map.nbinz+1);
 fprintf(mapfile, "%6s %6.3lf %6.3lf %6.3lf\n", "origin",
   Map.xmin,Map.ymin,Map.zmin);
 fprintf(mapfile, "%5s %1d %1d %1d\n", "delta",
   static_cast<int>(Map.resolution),0,0);
 fprintf(mapfile, "%5s %1d %1d %1d\n", "delta", 0,
   static_cast<int>(Map.resolution),0);
 fprintf(mapfile, "%5s %1d %1d %1d\n", "delta", 0,0,
   static_cast<int>(Map.resolution));
 fprintf(mapfile, "%37s %3d %3d %3d\n","object 2 class gridconnections counts"
   ,Map.nbinx+1,Map.nbiny+1,Map.nbinz+1);
 fprintf(mapfile, "%45s %7d %13s\n", "object 3 class array type double rank 0
   items",totbin,"data follows");

 int counter = 0;
 for(int i=0; i<=Map.nbinx; ++i){
  for(int j=0; j<=Map.nbiny; ++j){
   for(int k=0; k<=Map.nbinz; ++k){
    // make key
    std::array<int,3> ijk = {{i, j, k}};
    std::pair<std::array<int,3>,std::string> key = std::make_pair(ijk,label);
    // get entry
    Float m0 = 0.;
    if(map.count(key)>0){ m0 = map[key]; }
    // and normalize it
    m0 /= normalization;
    // write stuff
    fprintf(mapfile, "%12.6lf ", m0);
    // increase counter
    counter += 1;
    // new line
    if(counter%3==0) { fprintf(mapfile, "\n"); }
   }
  }
 }
 // end of file
 fprintf(mapfile, "\n");
 std::string aux = "\"";
 fprintf(mapfile, "%6s %s %11s\n", "object",
   (aux+label+aux).c_str(),"class field");
 fclose(mapfile);
}

int main(int argc, char* argv[])
{

// MPI stuff
MPI_Init(&argc,&argv);
int nproc, myrank;
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Status status;

// read input
std::string inputfile="config.ini";
int i=1;
while(i<argc){
 if(strcmp(argv[i],"-in")==0){++i; inputfile=argv[i];}
 ++i;
}

// parsing input
SPBParameters mydata=get_SPBParameters(inputfile,"0");

// create a new model
IMP_NEW(Model,m,());

// List of particles for layer restraint
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
// List of MonteCarloMovers for MC, not used here
core::MonteCarloMovers mvs;

//
// ISD PARTICLES
//
std::map<std::string, base::Pointer<Particle> > ISD_ps=
 add_ISD_particles(m,mydata,mvs);
// create list of particles from map
Particles ISD_ps_list;
std::map<std::string, base::Pointer<Particle> >::iterator itr;
for(itr = ISD_ps.begin(); itr != ISD_ps.end(); ++itr){
 ISD_ps_list.push_back((*itr).second);
}


//
// PROTEIN REPRESENTATION
//
atom::Hierarchies all_mol=
 create_representation(m,mydata,CP_ps,IL2_ps,mvs,
                       ISD_ps["SideXY"],ISD_ps["SideZ"],0);
//
// create list to link to rmf
//
atom::Hierarchies hhs;
for(unsigned int i=0;i<all_mol.size();++i){
 atom::Hierarchies hs=all_mol[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {hhs.push_back(hs[j]);}
}

//
// read file list
//
// vectors
std::vector<std::string> frame_files;
std::vector<std::string> frame_isdfiles;
Floats weights;
// and temp variables
std::string frame_file;
std::string frame_isdfile;
Float weight;
std::ifstream file_list;
// open file
file_list.open(mydata.Map.frames_list.c_str());
if(file_list.is_open()){
 // read file
 while (file_list >> frame_file >> frame_isdfile >> weight){
  frame_files.push_back(frame_file);
  frame_isdfiles.push_back(frame_isdfile);
  weights.push_back(weight);
 }
 // close file
 file_list.close();
}

// define boundaries of the density maps based on maximum cell dimension
mydata.Map.xmin = -1.5 * mydata.sideMax;
mydata.Map.xmax = +1.5 * mydata.sideMax;
mydata.Map.ymin = -sqrt(3.0) * mydata.sideMax;
mydata.Map.ymax = +sqrt(3.0) * mydata.sideMax;
mydata.Map.zmin = -300.0;
mydata.Map.zmax = +300.0;
// get number of bins
mydata.Map.nbinx = static_cast<int>(floor((mydata.Map.xmax
    -mydata.Map.xmin)/mydata.Map.resolution));
mydata.Map.nbiny = static_cast<int>(floor((mydata.Map.ymax
    -mydata.Map.ymin)/mydata.Map.resolution));
mydata.Map.nbinz = static_cast<int>(floor((mydata.Map.zmax
    -mydata.Map.zmin)/mydata.Map.resolution));

// initialize normalization
double normalization = 0.;

// global map
std::map< std::pair<std::array<int,3>,std::string>, double> map_total;

// cycle on file in file_list frames
for(unsigned ifile = myrank; ifile < frame_files.size(); ifile+=nproc){

 // open rmf for reference coordinates
 RMF::FileConstHandle rh =
   RMF::open_rmf_file_read_only(frame_files[ifile]);
 // linking hierarchies
 rmf::link_hierarchies(rh, hhs);

 // open rmf for ISD particles
 RMF::FileConstHandle rh_ISD =
   RMF::open_rmf_file_read_only(frame_isdfiles[ifile]);
 // linking particles
 rmf::link_particles(rh_ISD, ISD_ps_list);

 // load coordinates
rmf::load_frame(rh,RMF::FrameID(0));
 // and ISD particles
rmf::load_frame(rh_ISD,RMF::FrameID(0));

 // get weight
 Float weight = weights[ifile];
 normalization += weight;

 // side
 Float side = isd::Scale(ISD_ps["SideXY"]).get_scale()*mydata.sideMin;

 // get particles within boundaries
 Particles Ps = generate_new_particles(m, all_mol, side, 0.0, 0.0,
                                        mydata.Map.xmin, mydata.Map.xmax,
                                        mydata.Map.ymin, mydata.Map.ymax);

 std::map< std::pair<std::array<int,3>,std::string>,double> map_frame
  = get_map(Ps, mydata.Map);

 // add to global map
 add_to_map(map_frame, map_total, weight);

 // close RMFs
 rh     = RMF::FileHandle();
 rh_ISD = RMF::FileHandle();
}

// vector of strings with labels
std::vector<std::string> labels;
std::map< std::pair<std::array<int,3>,std::string>, double>::iterator iit;
for(iit=map_total.begin(); iit!=map_total.end(); ++iit){
   std::string l = ((*iit).first).second;
   int mycount = std::count (labels.begin(), labels.end(), l);
   if(mycount==0) labels.push_back(l);
}

// sort them
std::sort(labels.begin(), labels.end());


// wait for everybody to be done
MPI_Barrier(MPI_COMM_WORLD);

// gather total map
int tot_bin = (mydata.Map.nbinx+1)*(mydata.Map.nbiny+1)*(mydata.Map.nbinz+1);

for(unsigned l=0; l<labels.size(); ++l){

 double *map_array_send = new double [tot_bin];
 int index = 0;
 // put everything in an array for faster comm
 for(int i=0; i<=mydata.Map.nbinx; ++i){
  for(int j=0; j<=mydata.Map.nbiny; ++j){
   for(int k=0; k<=mydata.Map.nbinz; ++k){
    // get key
    std::array<int,3> ijk = {{i, j, k}};
    std::pair<std::array<int,3>,std::string> key =
     std::make_pair(ijk, labels[l]);
    // fill array at position index
    double m = 0.;
    if(map_total.count(key)>0){ m = map_total[key]; }
    map_array_send[index] = m;
    index += 1;
   }
  }
 }

 // prepare to send/receive
 double *map_array_recv = new double [tot_bin];
 MPI_Allreduce(map_array_send,map_array_recv,tot_bin,
   MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

 // back in the map
 index = 0;
 for(int i=0; i<=mydata.Map.nbinx; ++i){
  for(int j=0; j<=mydata.Map.nbiny; ++j){
   for(int k=0; k<=mydata.Map.nbinz; ++k){
    // get key
    std::array<int,3> ijk = {{i, j, k}};
    std::pair<std::array<int,3>,std::string> key =
     std::make_pair(ijk, labels[l]);
    // fill map
    if(map_array_recv[index]>0.){ map_total[key] =
     map_array_recv[index]; }
    index += 1;
   }
  }
 }
 // delete arrays
 delete[] map_array_send;
 delete[] map_array_recv;
}

// wait for everybody to be done
MPI_Barrier(MPI_COMM_WORLD);
// get normalization
double tot_norm = 0.;
MPI_Allreduce(&normalization,&tot_norm,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

// print maps in separate files
if(myrank==0){
 for(unsigned l=0; l<labels.size(); ++l){
  print_map(map_total, mydata.Map, tot_norm, labels[l]);
 }
}

// calculate half maximum
if(myrank==0){
 // open file for HM
 FILE *HMfile;
 std::string HMname = "HM.dat";
 HMfile = fopen(HMname.c_str(),"w");
 for(unsigned l=0; l<labels.size(); ++l){
  Float map_max=0.0;
  for(int i=0; i<=mydata.Map.nbinx; ++i){
   for(int j=0; j<=mydata.Map.nbiny; ++j){
    for(int k=0; k<=mydata.Map.nbinz; ++k){
     // get key
     std::array<int,3> ijk = {{i, j, k}};
     std::pair<std::array<int,3>,std::string> key =
      std::make_pair(ijk, labels[l]);
     // get entry
     Float m0 = 0.;
     if(map_total.count(key)>0){ m0 = map_total[key]; }
     // and normalize it
     m0 /= tot_norm;
     // find maximum
     if(m0>map_max) map_max = m0;
    }
   }
  }
  // print to file
  fprintf(HMfile, "%15s %12.6lf\n", labels[l].c_str(), map_max/2.0);
 }
 // close files
 fclose(HMfile);
}

MPI_Barrier(MPI_COMM_WORLD);
// finalize MPI
MPI_Finalize();

return 0;
}
