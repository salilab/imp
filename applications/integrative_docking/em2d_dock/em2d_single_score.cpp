/**
 * \file em2d_single_score.cpp \brief A program to score a single
 * docking model in PDB file
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "Image2D.h"
#include "Projection.h"

#include <IMP/algebra/Vector3D.h>
#include <IMP/Model.h>
#include <IMP/atom/pdb.h>
#include <IMP/core/XYZ.h>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <math.h>
#include <vector>
#include <string>

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
}

int main(int argc, char **argv) {
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;
  // input parameters
  float resolution = 10.0;
  int projection_number = 40;
  float pixel_size = 2.2;
  float area_threshold = 0.4; // used 0.4 for benchmark and PCSK9
  std::vector<std::string> image_files;
  std::string pdb;
  po::options_description desc("Usage: <pdb> <image1> <image2>...");
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
    ("area-threshold,a", po::value<float>(&area_threshold)->default_value(0.4),
     "maximal percentage of area difference for \
aligned images (default = 0.4)");
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
    if(files.size() < 2) { std::cout << desc << "\n"; return 0; }
    pdb = files[0];
    for(unsigned int i=1; i<files.size(); i++) image_files.push_back(files[i]);
  } else {
    std::cout << desc << "\n"; return 0;
  }

  // read images
  std::vector<Image2D<> > images;
  for(unsigned int i=0; i<image_files.size(); i++) {
    Image2D<> image(image_files[i]);
    image.get_largest_connected_component();
    image.pad((int)(image.get_width()*1.4), (int)(image.get_height()*1.4));
    image.center();
    image.average();
    image.stddev();
    image.compute_PCA();
    images.push_back(image);
  }

  std::vector<Image2D<> > images2;
  for(unsigned int i=0; i<image_files.size(); i++) {
    Image2D<> image(image_files[i]);
    images2.push_back(image);
  }
  Image2D<>::write_PGM(images2, "images.pgm");
  //Image2D<>::write_PGM(images, "images_cc.pgm");

  // read pdbs and generate projections
  std::vector<IMP::algebra::Vector3D> points = read_points_from_pdb(pdb);
  boost::ptr_vector<Projection> projections;
  create_projections(points, projection_number, pixel_size, resolution,
                     projections, images[0].get_height());
  std::cerr << projections.size() << " projections were created" << std::endl;

  // process projections
  for(unsigned int i=0; i<projections.size(); i++) {
    projections[i].get_largest_connected_component();
    projections[i].center();
    projections[i].average();
    projections[i].stddev();
    projections[i].compute_PCA();

  }

  float total_score = 0;
  std::vector<Image2D<> > best_projections;
  for(unsigned int i=0; i<images.size(); i++) {
    ImageTransform best_transform;
    best_transform.set_score(0.0);
    int best_projection_id=0;
    for(unsigned int j=0; j<projections.size(); j++) {
      // do not align images with more than X% area difference
      double area_score = std::abs(images[i].segmented_pixels()-
                                   projections[j].segmented_pixels()) /
        (double)std::max(images[i].segmented_pixels(),
                         projections[j].segmented_pixels());
      if(area_score > area_threshold) continue;

      ImageTransform curr_transform = images[i].pca_align(projections[j]);
      curr_transform.set_area_score(area_score);
      if(curr_transform.get_score() > best_transform.get_score()) {
        best_transform = curr_transform;
        best_projection_id = projections[j].get_id();
      }
    }
    std::cerr << "Image " << i << " Best projection "
              << best_projection_id << " " << best_transform << std::endl;
    total_score+= best_transform.get_score();

    Image2D<> transformed_image;
    projections[best_projection_id].rotate_circular(transformed_image,
                                                    best_transform.get_angle());
    transformed_image.translate(best_transform.get_x(), best_transform.get_y());
    best_projections.push_back(transformed_image);
  }
  std::cerr << "Total score = " << total_score << std::endl;
  Image2D<>::write_PGM(best_projections, "best_projections.pgm");
}
