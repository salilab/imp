/**
 * \file em2d_score.cpp \brief A program to score docking models given a list
 * of transformations for a ligand
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/integrative_docking/internal/helpers.h>
#include <IMP/integrative_docking/internal/EM2DFitResult.h>

#include <IMP/em2d/internal/Image2D.h>
#include <IMP/em2d/internal/Projection.h>

#include <IMP/saxs/FormFactorTable.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/kernel/Model.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Mass.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace IMP::integrative_docking::internal;

#include <math.h>
#include <vector>
#include <string>
#include <fstream>

int main(int argc, char** argv) {
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;
  // input parameters
  float resolution = 10.0;
  int projection_number = 40;
  float pixel_size = 2.2;
  float area_threshold = 0.4;  // used 0.4 for benchmark and PCSK9
  bool residue_level = false;
  std::vector<std::string> image_files;
  std::string rpdb, lpdb, trans_file, out_file_name;
  po::options_description desc(
      "Usage: <pdb1> <pdb2> <trans file> <image1> \
<image2>...");
  desc.add_options()("help", "produce help message")(
      "input-files", po::value<std::vector<std::string> >(),
      "input PDB and image files")(
      "resolution,r", po::value<float>(&resolution)->default_value(10.0),
      "image resolution (default = 10.0)")(
      "pixel-size,s", po::value<float>(&pixel_size)->default_value(2.2),
      "images pixel size (default = 2.2)")(
      "projection-number,n",
      po::value<int>(&projection_number)->default_value(40),
      "number of projections for PDBs")
      ("area-threshold,a",
       po::value<float>(&area_threshold)->default_value(0.4),
       "maximal percentage of area difference for aligned images (default = "
       "0.4)")(
      "ca-only,c", "perform fast coarse grained profile calculation using \
CA atoms only (default = false)")(
      "output_file,o",
      po::value<std::string>(&out_file_name)->default_value("em2d_score.res"),
      "output file name, default name em2d_score.res");
  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (vm.count("ca-only")) residue_level = true;
  if (vm.count("input-files")) {
    std::vector<std::string> files =
        vm["input-files"].as<std::vector<std::string> >();
    if (files.size() < 4) {
      std::cout << desc << "\n";
      return 0;
    }
    rpdb = files[0];
    lpdb = files[1];
    trans_file = files[2];
    for (unsigned int i = 3; i < files.size(); i++)
      image_files.push_back(files[i]);
  } else {
    std::cout << desc << "\n";
    return 0;
  }
  std::cerr << "Receptor " << rpdb << " ligand " << lpdb << " trans file "
            << trans_file << std::endl;

  // read pdbs
  IMP::kernel::Particles particles, rparticles, lparticles;
  IMP::saxs::FormFactorType ff_type = IMP::saxs::HEAVY_ATOMS;
  if (residue_level) ff_type = IMP::saxs::CA_ATOMS;
  IMP::saxs::FormFactorTable ft;
  if (residue_level) {
    read_pdb_ca_atoms(rpdb, rparticles);
    read_pdb_ca_atoms(lpdb, lparticles);
  } else { // atoms
    read_pdb_atoms(rpdb, rparticles);
    read_pdb_atoms(lpdb, lparticles);
  }
  particles = rparticles;
  particles.insert(particles.end(), lparticles.begin(), lparticles.end());
  // save lparticles coordinates (they are going to move)
  std::vector<IMP::algebra::Vector3D> lcoordinates;
  for (unsigned int i = 0; i < lparticles.size(); i++) {
    lcoordinates.push_back(IMP::core::XYZ(lparticles[i]).get_coordinates());
  }

  // add radius
  for (unsigned int i=0; i<particles.size(); i++) {
    double r = ft.get_radius(particles[i], ff_type);
    IMP::core::XYZR::setup_particle(particles[i]->get_model(),
                                    particles[i]->get_index(), r);
  }

  // add mass
  if (residue_level) {
    for (unsigned int i=0; i<particles.size(); i++) {
      IMP::atom::Residue r =
        IMP::atom::get_residue(IMP::atom::Atom(particles[i]));
      double m = IMP::atom::get_mass(r.get_residue_type());
      IMP::atom::Mass(particles[i]).set_mass(m);
    }
  }

  // read tranformations
  std::vector<IMP::algebra::Transformation3D> transforms;
  read_trans_file(trans_file, transforms);

  // read images
  std::vector<IMP::em2d::internal::Image2D<> > images(image_files.size());
  double max_distance = 0.0;
  for (unsigned int i = 0; i < image_files.size(); i++) {
    images[i].read_PGM(image_files[i]);
    // increase images size in case some projections are larger
    images[i].get_largest_connected_component();
    images[i].pad(int(images[i].get_width() * 2.0),
                  int(images[i].get_height() * 2.0));
    images[i].center();
    images[i].average();
    images[i].stddev();
    images[i].compute_PCA();
    double curr_max_distance = images[i].max_distance();
    if (curr_max_distance > max_distance) max_distance = curr_max_distance;
  }
  max_distance *= pixel_size;
  std::cerr << "Max distance = " << max_distance << " threshold "
            << 0.95 * max_distance << std::endl;
  max_distance *= 0.9;

  // create receptor projections
  boost::ptr_vector<IMP::em2d::internal::Projection> rprojections;
  compute_projections(rparticles, projection_number, pixel_size, resolution,
                      rprojections, images[0].get_height());
  //  write_PGM(rprojections, "rprojections.pgm");

  // iterate transformations
  std::vector<EM2DFitResult> fit_results;
  for (unsigned int t = 0; t < transforms.size(); t++) {
    // std::cerr << "Screening transformation " << t << std::endl;

    // apply transformation
    transform(lparticles, transforms[t]);

    boost::ptr_vector<IMP::em2d::internal::Projection> projections;
    // create ligand projections
    compute_projections(particles, lparticles, projection_number,
                        pixel_size, resolution, projections,
                        images[0].get_height());
    // add receptor projections
    for (unsigned int i = 0; i < projections.size(); i++)
      projections[i].add(rprojections[i]);

    for (unsigned int i = 0; i < projections.size(); i++) {
      projections[i].get_largest_connected_component();
      projections[i].center();
      projections[i].average();
      projections[i].stddev();
      projections[i].compute_PCA();
    }
    //    cerr << projections.size() << " projections were created" << endl;
    // write_PGM(projections, "projections.pgm");

    float total_score = 0.0;
    for (unsigned int i = 0; i < images.size(); i++) {
      IMP::em2d::internal::ImageTransform best_transform;
      best_transform.set_score(0.0);
      int best_projection_id = 0;
      for (unsigned int j = 0; j < projections.size(); j++) {
        // if(j%50==0) cerr << "Start aligning projection number " << j << endl;
        // do not align images with more than X% area difference
        double area_score = std::abs(images[i].segmented_pixels() -
                                     projections[j].segmented_pixels()) /
                            (double)std::max(images[i].segmented_pixels(),
                                             projections[j].segmented_pixels());
        if (area_score > area_threshold) continue;

        IMP::em2d::internal::ImageTransform curr_transform =
          images[i].pca_align(projections[j]);
        if (curr_transform.get_score() > best_transform.get_score()) {
          best_transform = curr_transform;
          best_projection_id = projections[j].get_id();
        }
      }
      std::cerr << "Image " << i << " Best projection " << best_projection_id
                << " " << best_transform << std::endl;
      total_score += best_transform.get_score();
    }
    total_score /= images.size();
    EM2DFitResult fr(t + 1, total_score, false, transforms[t]);
    fit_results.push_back(fr);
    std::cerr << "Transform: " << t << " | " << total_score << " | "
              << transforms[t] << std::endl;
    // return back
    for (unsigned int ip = 0; ip < lparticles.size(); ip++) {
      IMP::core::XYZ(lparticles[ip]).set_coordinates(lcoordinates[ip]);
    }

    // out_file << t+1 << " | " << total_score << " | " << diameter
    //<< " | "  << transformation3d_to_rigidtrans3(transforms[t]) << std::endl;
  }

  set_z_scores(fit_results);

  // output
  std::ofstream out_file(out_file_name.c_str());
  out_file << "receptorPdb (str) " << rpdb << std::endl;
  out_file << "ligandPdb (str) " << lpdb << std::endl;
  EM2DFitResult::print_header(out_file);
  for (unsigned int i = 0; i < fit_results.size(); i++)
    out_file << fit_results[i] << std::endl;
  out_file.close();
}
