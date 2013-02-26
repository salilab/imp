/**
 * \file em2d_score.cpp \brief A program to score docking models given a list
 * of transformations for a ligand
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "Image2D.h"
#include "Projection.h"
#include "FitResult.h"

#include <IMP/algebra/Vector3D.h>
#include <IMP/Model.h>
#include <IMP/atom/pdb.h>
#include <IMP/core/XYZ.h>
#include <IMP/utility.h>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <math.h>
#include <vector>
#include <string>
#include <fstream>

namespace {
std::vector<IMP::algebra::Vector3D> read_points_from_pdb(
                                                 std::string pdb_file_name) {
  // check if file exists
  std::ifstream in_file(pdb_file_name.c_str());
  if(!in_file) {
    std::cerr << "Can't open file " << pdb_file_name << std::endl;
    exit(1);
  }
  std::vector<IMP::algebra::Vector3D> points;
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd =
    IMP::atom::read_pdb(pdb_file_name, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                        true, true);
  IMP::ParticlesTemp particles = get_by_type(mhd, IMP::atom::ATOM_TYPE);
  for(unsigned int i=0; i<particles.size(); i++)
    points.push_back(IMP::core::XYZ(particles[i]).get_coordinates());
  std::cerr << points.size() << " atoms were read from file "
            << pdb_file_name << std::endl;
  return points;
}

void read_trans_file(const std::string file_name,
                     std::vector<IMP::algebra::Transformation3D>& transforms) {
  std::ifstream trans_file(file_name.c_str());
  if(!trans_file) {
    std::cerr << "Can't find Transformation file " << file_name << std::endl;
    exit(1);
  }

  IMP::algebra::Vector3D rotation_vec, translation;
  int trans_number;
  while(trans_file >> trans_number >> rotation_vec >> translation) {
    IMP::algebra::Rotation3D rotation =
      IMP::algebra::get_rotation_from_fixed_xyz(rotation_vec[0],
                                                rotation_vec[1],
                                                rotation_vec[2]);
    IMP::algebra::Transformation3D trans(rotation, translation);
    transforms.push_back(trans);
  }
  trans_file.close();
  std::cout << transforms.size() << " transforms were read from "
            << file_name << std::endl;
}

void compute_zscores(std::vector<FitResult>& fit_results) {
  float average = 0.0;
  float std = 0.0;
  int counter = 0;
  for(unsigned int i=0; i<fit_results.size(); i++) {
    if(!fit_results[i].is_filtered()) {
      counter++;
      average += fit_results[i].get_score();
      std += IMP::square(fit_results[i].get_score());
    }
  }
  average /= counter;
  std /= counter;
  std -= IMP::square(average);
  std = sqrt(std);

  // update z_scores
  for(unsigned int i=0; i<fit_results.size(); i++) {
    if(!fit_results[i].is_filtered()) {
      float z_score = (fit_results[i].get_score() - average)/std;
      fit_results[i].set_z_score(z_score);
    }
  }
}
}

int main(int argc, char **argv) {
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;
  // input parameters
  float resolution = 10.0;
  int projection_number = 40;
  float pixel_size = 2.2;
  float area_threshold = 0.4; // used 0.4 for benchmark and PCSK9
  float min_diameter = 0.0; IMP_UNUSED(min_diameter);
  std::vector<std::string> image_files;
  std::string rpdb, lpdb, trans_file, out_file_name;
  po::options_description desc("Usage: <pdb1> <pdb2> <trans file> <image1> \
<image2>...");
  desc.add_options()
    ("help", "produce help message")
    ("input-files", po::value<std::vector<std::string> >(),
     "input PDB and image files")
    ("resolution,r", po::value<float>(&resolution)->default_value(10.0),
     "image resolution (default = 10.0)")
    ("pixel-size,s", po::value<float>(&pixel_size)->default_value(2.2),
     "images pixel size (default = 2.2)")
    ("projection-number,n",
     po::value<int>(&projection_number)->default_value(40),
     "number of projections for PDBs")
    // ("min-diameter,d", po::value<float>(&min_diameter)->default_value(0.0),
    //  "minimal diameter of the complex from the images (default = 0.0)")
    ("area-threshold,a", po::value<float>(&area_threshold)->default_value(0.4),
     "maximal percentage of area difference for aligned images (default = 0.4)")
    ("output_file,o",
     po::value<std::string>(&out_file_name)->default_value("em2d_score.res"),
     "output file name, default name em2d_score.res");
  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
     po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);
  if(vm.count("help")) { std::cout << desc << "\n"; return 0; }
  if(vm.count("input-files"))   {
    std::vector<std::string> files =
      vm["input-files"].as<std::vector<std::string> >();
    if(files.size() < 4) { std::cout << desc << "\n"; return 0; }
    rpdb = files[0];
    lpdb = files[1];
    trans_file = files[2];
    for(unsigned int i=3; i<files.size(); i++) image_files.push_back(files[i]);
  } else {
    std::cout << desc << "\n"; return 0;
  }
  std::cerr << "Receptor " << rpdb << " ligand " << lpdb
            << " trans file " << trans_file << std::endl;

  // read pdbs
  std::vector<IMP::algebra::Vector3D> rpoints = read_points_from_pdb(rpdb);
  std::vector<IMP::algebra::Vector3D> lpoints = read_points_from_pdb(lpdb);

  // read tranformations
  std::vector<IMP::algebra::Transformation3D> transforms;
  read_trans_file(trans_file, transforms);
  std::vector<IMP::algebra::Vector3D> points(rpoints),
    transformed_lpoints(lpoints);
  points.resize(rpoints.size()+lpoints.size());

  // read images
  std::vector<Image2D<> > images(image_files.size());
  double max_distance = 0.0;
  for(unsigned int i=0; i<image_files.size(); i++) {
    images[i].read_PGM(image_files[i]);
    // increase images size in case some projections are larger
    images[i].get_largest_connected_component();
    images[i].pad(int(images[i].get_width()*2.0),
                  int(images[i].get_height()*2.0));
    images[i].center();
    images[i].average();
    images[i].stddev();
    images[i].compute_PCA();
    double curr_max_distance = images[i].max_distance();
    if(curr_max_distance > max_distance) max_distance = curr_max_distance;
  }
  max_distance*=pixel_size;
  std::cerr << "Max distance = " << max_distance
            << " threshold " << 0.95*max_distance << std::endl;
  max_distance*=0.9;

  // create receptor projections
  boost::ptr_vector<Projection> rprojections;
  create_projections(rpoints, projection_number, pixel_size, resolution,
                     rprojections, images[0].get_height());
  //  write_PGM(rprojections, "rprojections.pgm");

  // iterate transformations
  std::vector<FitResult> fit_results;
  for(unsigned int t=0; t<transforms.size(); t++) {
    //std::cerr << "Screening transformation " << t << std::endl;

    // apply transformation
    for(unsigned int p_index=0; p_index<lpoints.size(); p_index++) {
      IMP::algebra::Vector3D p = transforms[t]*lpoints[p_index];
      points[rpoints.size()+p_index] = p;
      transformed_lpoints[p_index] = p;
    }

    // double diameter = 0.0;
    // if(min_diameter > 0.0) {
    //   // check diameter first
    //   diameter = compute_max_distance(points);
    //   if(diameter < max_distance) {
    //     FitResult fr(t+1, 0.0, true, transforms[t]);
    //     fit_results.push_back(fr);
    //     continue;
    //   }
    // }

    boost::ptr_vector<Projection> projections;
    // create ligand projections
    create_projections(points, transformed_lpoints, projection_number,
                       pixel_size, resolution, projections,
                       images[0].get_height());
    // add receptor projections
    for(unsigned int i=0; i<projections.size(); i++)
      projections[i].add(rprojections[i]);

    for(unsigned int i=0; i<projections.size(); i++) {
      projections[i].get_largest_connected_component();
      projections[i].center();
      projections[i].average();
      projections[i].stddev();
      projections[i].compute_PCA();
    }
    //    cerr << projections.size() << " projections were created" << endl;
    //write_PGM(projections, "projections.pgm");

    float total_score = 0.0;
    for(unsigned int i=0; i<images.size(); i++) {
      ImageTransform best_transform;
      best_transform.set_score(0.0);
      int best_projection_id=0;
      for(unsigned int j=0; j<projections.size(); j++) {
        //if(j%50==0) cerr << "Start aligning projection number " << j << endl;
        // do not align images with more than X% area difference
        double area_score =
          std::abs(images[i].segmented_pixels() -
                   projections[j].segmented_pixels()) /
          (double)std::max(images[i].segmented_pixels(),
                           projections[j].segmented_pixels());
        if(area_score > area_threshold) continue;

        ImageTransform curr_transform = images[i].pca_align(projections[j]);
        if(curr_transform.get_score() > best_transform.get_score()) {
          best_transform = curr_transform;
          best_projection_id = projections[j].get_id();
        }
      }
      std::cerr << "Image " << i << " Best projection " << best_projection_id
                << " " << best_transform << std::endl;
      total_score+= best_transform.get_score();
    }
    total_score /= images.size();
    FitResult fr(t+1, total_score, false, transforms[t]);
    fit_results.push_back(fr);
    std::cerr << "Transform: " << t << " | " << total_score
              << " | " << transforms[t] << std::endl;
    //out_file << t+1 << " | " << total_score << " | " << diameter
    //<< " | "  << transformation3d_to_rigidtrans3(transforms[t]) << std::endl;
  }

  compute_zscores(fit_results);

  // output
  std::ofstream out_file(out_file_name.c_str());
  out_file << "receptorPdb (str) " << rpdb << std::endl;
  out_file << "ligandPdb (str) " << lpdb << std::endl;
  FitResult::print_header(out_file);
  for(unsigned int i=0; i<fit_results.size(); i++)
    out_file << fit_results[i] << std::endl;
  out_file.close();
}
