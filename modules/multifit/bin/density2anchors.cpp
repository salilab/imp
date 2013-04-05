/**
 *  \file gmm_em_main.cpp
 *  \brief Find an anchor graph segmentation of a density map
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
**/

#include <string>
#include <IMP/Model.h>
#include <IMP/base_types.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/atom/pdb.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/core/XYZ.h>
#include <IMP/statistics/VQClustering.h>
#include <IMP/multifit/DensityDataPoints.h>
#include <IMP/multifit/DataPointsAssignment.h>
#include <boost/algorithm/string.hpp>
#include <IMP/multifit/anchors_reader.h>
//others
#include <boost/program_options.hpp>
#include <boost/format.hpp>

using namespace IMP;
using namespace boost;
namespace {
int parse_input(int argc, char *argv[],
                std::string &density_filename,float &apix,int &num_means,
                float &t,
                float &xorigin, float &yorigin, float &zorigin,
                std::string &cmm_filename,std::string &pdb_filename,
                std::string &seg_filename,std::string &cmm_max_filename,
                std::string &txt_filename) {
  xorigin=INT_MAX;yorigin=INT_MAX,zorigin=INT_MAX;apix=INT_MAX;
  pdb_filename="";
  seg_filename="";
  cmm_max_filename="";
  txt_filename="";
  std::stringstream usage;
  usage<<"The program segments all voxels above a given threshold into K "<<
    "clusters and links"<<
    " between neighboring ones.\n\nUsage: density2anchors <density.mrc> "<<
    "<number of clusters> "<<
    "<density threshold> <output.pdb>\n\n";
  program_options::options_description optional_params("Allowed options"),
    po,ao,required_params("Hideen options");
  required_params.add_options()
    ("density",program_options::value<std::string>(&density_filename),
     "assembly density filename")
    ("num_means",program_options::value<int>(&num_means),
     "the number of clusters to use")
    ("t",program_options::value<float>(&t),"density cutoff")
    ("output-pdb",program_options::value<std::string>(&pdb_filename),
     "cluster centers as CA atoms in a PDB file");
  program_options::positional_options_description p;
  std::stringstream seg_help;
  seg_help<<"print each cluster as a MRC file <seg>_i.mrc, and write "<<
    "load_segmentation.cmd file to easily load all segments into Chimera";
  p.add("density", 1);
  p.add("num_means", 1);
  p.add("t", 1);
  p.add("output-pdb", 1);
  // Declare a group of options that will be
  // allowed from the command line
  optional_params.add_options()
    ("help", usage.str().c_str())
    ("apix",program_options::value<float>(&apix),
     "the a/pix of the density map")
    ("x",program_options::value<float>(&xorigin),
     "the X origin of the density map")
    ("y",program_options::value<float>(&yorigin),
     "the Y origin of the density map")
    ("z",program_options::value<float>(&zorigin),
     "the Z origin of the density map")
    ("cmm",program_options::value<std::string>(&cmm_filename),
     "write results in cmm format")
    ("cmm-max",program_options::value<std::string>(&cmm_max_filename),
     "Max point of each cluster stored in a CMM file.")
    ("seg",program_options::value<std::string>(&seg_filename),
     seg_help.str().c_str())
    ("txt",program_options::value<std::string>(&txt_filename),
     "anchors points in txt file format");

  program_options::options_description all;
  all.add(optional_params).add(required_params);

  program_options::variables_map vm;
  program_options::store(
  program_options::command_line_parser(argc,
                                       argv).options(all).positional(p).run(),
                                       vm);
  program_options::notify(vm);
  if (vm.count("help")) {
    std::cout <<optional_params<< "\n";
    return 1;
  }
  if (! (vm.count("density")+vm.count("num_means")+vm.count("t")+
           vm.count("output-pdb")== 4)){
      std::cout<<optional_params<<std::endl;
      return 1;
    }
  return 0;
}
}

int main(int argc, char *argv[]) {
 std::string density_filename;
 float apix,density_threshold;
 int num_means;
 float xorigin,yorigin,zorigin;
 std::string cmm_filename, pdb_filename,seg_filename;
 std::string cmm_max_filename,txt_filename;

 if (parse_input(argc, argv,density_filename,apix,num_means,density_threshold,
                 xorigin, yorigin, zorigin,
                 cmm_filename,pdb_filename,seg_filename,cmm_max_filename,
                 txt_filename)){
   exit(0);
 }
 set_log_level(SILENT);
 IMP_NEW(em::MRCReaderWriter,mrw,());
 em::DensityMap *dmap = em::read_map(density_filename.c_str(),mrw);
 if (apix == INT_MAX) {
   apix = dmap->get_spacing();
 }
 else {
   dmap->update_voxel_size(apix);
 }
 algebra::Vector3D v = dmap->get_origin();
 if (xorigin == INT_MAX) {
   xorigin = v[0];
 }
 if (yorigin == INT_MAX) {
   yorigin = v[1];
 }
 if (zorigin == INT_MAX) {
   zorigin = v[2];
 }
 dmap->set_origin(xorigin, yorigin, zorigin);
 set_log_level(VERBOSE);
 IMP_LOG_VERBOSE("============= parameters ============"<<std::endl);
 IMP_LOG_VERBOSE("density filename : " << density_filename <<std::endl);
 IMP_LOG_VERBOSE("a/pix : " << apix <<std::endl);
 IMP_LOG_VERBOSE("origin : (" << xorigin << "," << yorigin<<"," <<
         zorigin << ")" << std::endl);
 IMP_LOG_VERBOSE("density threshold : " << density_threshold <<std::endl);
 IMP_LOG_VERBOSE("number of centers : " << num_means <<std::endl);
 IMP_LOG_VERBOSE("output pdb file : " << pdb_filename <<std::endl);
 IMP_LOG_VERBOSE("output cmm file : " << cmm_filename <<std::endl);
 IMP_LOG_VERBOSE("output max cmm file : " << cmm_max_filename <<std::endl);
 IMP_LOG_VERBOSE("segment mrc files names : " << seg_filename << std::endl);
 IMP_LOG_VERBOSE("====================================="<<std::endl);
 set_log_level(SILENT);
 dmap->set_origin(xorigin,yorigin,zorigin);
 IMP_LOG_VERBOSE("start setting trn_em"<<std::endl);
 IMP_NEW(multifit::DensityDataPoints,ddp,(dmap,density_threshold));
 IMP_LOG_VERBOSE("initialize calculation of initial centers"<<std::endl);
 IMP::statistics::VQClustering vq(ddp,num_means);
 vq.run();
 multifit::DataPointsAssignment assignment(ddp,&vq);
 multifit::AnchorsData ad(
                          assignment.get_centers(),
                          *(assignment.get_edges()));
 multifit::write_pdb(pdb_filename,assignment);
 //also write cmm string into a file:
 if (! (cmm_filename == "")) {
   multifit::write_cmm(cmm_filename,"anchor_graph",ad);
 }
 /* if (not (cmm_max_filename == "")) {
   multifit::write_max_cmm(cmm_max_filename,dmap,"max_graph",assignment);
   }*/
  if (! (seg_filename == "")) {
    multifit::write_segments_as_mrc(dmap,assignment,apix,apix,
                                    density_threshold,seg_filename);
   }
  if (! (txt_filename == "")) {
    multifit::write_txt(txt_filename,ad);
   }

 return 0;
}
