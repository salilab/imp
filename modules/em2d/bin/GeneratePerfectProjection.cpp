/**
 *  \file  GeneratePerfectProjection.cpp
 *  \brief Generation of projections from models or density maps
 *  Copyright 2007-2014 IMP Inventors. All rights reserved. 
*/


#include <IMP/em2d/em2d_config.h>
#include <IMP/em2d/ProjectionFinder.h>
#include <IMP/em2d/Em2DRestraint.h>
#include <IMP/em2d/project.h>
#include <IMP/em2d/filenames_manipulation.h>

#include <IMP/em2d/internal/Image2D.h>

#include <IMP/em2d/opencv_interface.h>
#include <IMP/em2d/Image.h>
#include <IMP/em2d/image_processing.h>
#include <IMP/em2d/SpiderImageReaderWriter.h>

#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Mass.h>
#include <IMP/em2d/PCAFitRestraint.h>
#include <IMP/em2d/internal/Projector.h>
#include <IMP/saxs/FormFactorTable.h>
#include <IMP/em2d/internal/ProjectionSphere.h>
#include <IMP/algebra/SphericalVector3D.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/em/KernelParameters.h>

#include <IMP/core/XYZ.h>
#include <IMP/core/Gaussian.h>
#include <IMP/benchmark/Profiler.h>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <IMP/em2d/RegistrationResult.h>
#include <IMP/Model.h>

namespace po = boost::program_options;
using namespace IMP::em2d::internal;

const char* const DELIMITER = "|";
const unsigned int ONLY_COARSE_REGISTRATION = 0;
const unsigned int COMPLETE_REGISTRATION = 1;
const unsigned int ALIGN2D_PREPROCESSING = 1;

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <fftw3.h>
#include <time.h>
#include <iostream>


double maxi3(double x, double y, double z) {
  int max = x;
  if (y > max) {
    max = y;
  }
  if (z > max) {
    max = z;
  }
  return max;
}

void quasi_evenly_spherical_distribution(unsigned long N,
                                         IMP::algebra::SphericalVector3Ds& vs,
                                         double r) {
  vs.resize(N);
  double theta, psi;
  for (unsigned long k = 1; k <= N; ++k) {
    double h = (2.0 * (k - 1)) / (N - 1) - 1.0;
    theta = std::acos(h);
    if (k == 1 || k == N) {
      psi = 0;
    } else {
      psi = (vs[k - 2][2] + 3.6 / sqrt((double)N * (1.0 - h * h)));
      int div = psi / (2 * IMP::PI);
      psi -= div * 2 * IMP::PI;
    }
    // Set the values of the spherical vector
    vs[k - 1][0] = r;
    vs[k - 1][1] = theta;
    vs[k - 1][2] = psi;
  }
}

int main(int argc, char *argv[]){
  ///Command Line argument. some of them may not be necessary right now. 
  std::ifstream PDBFile(argv[1]); 
  int NoP = atoi(argv[2]);
  std::string PGMFile = argv[3];
  double resolution = atof(argv[4]);
  double apix = atof(argv[5]);

  //Read the PDB file
  IMP_NEW(IMP::Model, m, ());

  IMP::atom::PDBSelector* selector; 
  selector = new IMP::atom::NonWaterNonHydrogenPDBSelector();
  IMP::atom::Hierarchy h = IMP::atom::read_pdb(PDBFile, m, selector);
  IMP::core::GenericHierarchies particle2s = IMP::core::get_leaves(h);
  IMP::Particles ps = get_by_type(h, IMP::atom::ATOM_TYPE);

  IMP::em2d::internal::Image2D<> image(PGMFile);
  int rows = image.get_height();
  int cols = image.get_width();
  
  //Create the N evenly distributed projection
  IMP::algebra::SphericalVector3Ds vs;
  quasi_evenly_spherical_distribution(NoP, vs, 1.0);
  IMP::em2d::RegistrationResults results;
  
  
  for (unsigned int i = 0; i < NoP; ++i) {
    IMP::algebra::Rotation3D R = IMP::em2d::internal::get_rotation_from_projection_direction(vs[i]);
    IMP::algebra::Vector2D shift(0.0, 0.0);
    results.push_back(IMP::em2d::RegistrationResult(R, shift, i, 0));
  }
  //Option for projections
  IMP::em2d::ProjectingOptions opts = IMP::em2d::ProjectingOptions(apix, resolution);
  IMP::em2d::Images projections = IMP::em2d::get_projections(ps, results, rows, cols, opts);
  
  //Create an image file for each projection
  for(int projnum=0; projnum < NoP; projnum++){
    
    double MaxValue, MinValue;
    cv::minMaxLoc(projections[projnum]->get_data(), &MinValue, &MaxValue);
    std::stringstream stream;
    stream << "Projection-" << projnum << ".pgm";
    const std::string filename0 = stream.str();
    const char* filename = filename0.c_str();
    FILE * fp;
    fp = fopen(filename, "w");
    std::fprintf(fp, "P2\n");
    std::fprintf(fp,"%i %i\n", rows, cols);
    std::fprintf(fp,"255\n");
    
    for(int jj = 0; jj < rows; jj++){
      for(int kk = 0; kk < cols; kk++){
	double pixel = projections[projnum]->get_data().at<double>(jj,kk);
	std::fprintf(fp, "%i ", (int) ((pixel-MinValue)*(255.0)/(MaxValue-MinValue)+0));
	//std::cerr << ((pixel-MinValue)*(255.0)/(MaxValue-MinValue)+0) << std::endl;
	if(kk > 0 && kk % 12 == 0)  std::fprintf(fp, "\n");
      }
      std::fprintf(fp, "\n");
    }
    fclose(fp);
  }  
  
}
