/**
 *  \file  create_single_particle_images.cpp
 *  \brief Generation of projections from models or density maps or pure noisy images
 *  Copyright 2007-2021 IMP Inventors. All rights reserved. 
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

#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Mass.h>
#include <IMP/em2d/PCAFitRestraint.h>
#include <IMP/em2d/internal/Projector.h>
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
#include <IMP/random.h>
#include <boost/random/normal_distribution.hpp>

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
#include <sstream>

namespace {
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
  
  void Generate_Noiseless_Projections(std::string PDBFile, int NoP, int isize, int resolution, int apix, std::string rootname){
    //Read the PDB file
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::PDBSelector* selector; 
    selector = new IMP::atom::NonWaterNonHydrogenPDBSelector();
    IMP::atom::Hierarchy h = IMP::atom::read_pdb(PDBFile, m, selector);
    IMP::core::GenericHierarchies particle2s = IMP::core::get_leaves(h);
    IMP::Particles ps = get_by_type(h, IMP::atom::ATOM_TYPE);
    
    int rows = isize;
    int cols = isize;
    
    //Create the N evenly distributed projection
    IMP::algebra::SphericalVector3Ds vs;
    quasi_evenly_spherical_distribution(NoP, vs, 1.0);
    IMP::em2d::RegistrationResults results;
    
    for (int i = 0; i < NoP; ++i) {
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
      stream << rootname << "-" << projnum << ".pgm";
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

  void Generate_Noisy_Projections(std::string PDBFile, int NoP, int isize, int resolution, int apix, float stddev, std::string rootname){
    
    srand( time(NULL) );

    //Read the PDB file
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::PDBSelector* selector;
    selector = new IMP::atom::NonWaterNonHydrogenPDBSelector();
    IMP::atom::Hierarchy h = IMP::atom::read_pdb(PDBFile, m, selector);
    IMP::core::GenericHierarchies particle2s = IMP::core::get_leaves(h);
    IMP::Particles ps = get_by_type(h, IMP::atom::ATOM_TYPE);
    int rows = isize;
    int cols = isize;
    int pheight = isize;
    int pwidth = isize;
    int pwidth2 = (int) (pwidth/2.0+1.0);
    int diffw = (int) (pwidth - cols)/2.0;
    int diffh = (int) (pheight - rows)/2.0;

    //Create N evenly distributed projection
    IMP::algebra::SphericalVector3Ds vs;
    quasi_evenly_spherical_distribution(NoP, vs, 1.0);
    IMP::em2d::RegistrationResults results;

    for (int i = 0; i < NoP; ++i) {
      IMP::algebra::Rotation3D R = IMP::em2d::internal::get_rotation_from_projection_direction(vs[i]);
      IMP::algebra::Vector2D shift(0.0, 0.0);
      results.push_back(IMP::em2d::RegistrationResult(R, shift, i, 0));
    }

    //Option for projections
    IMP::em2d::ProjectingOptions opts = IMP::em2d::ProjectingOptions(apix, resolution);
    IMP::em2d::Images projections = IMP::em2d::get_projections(ps, results, rows, cols, opts);

    ///Allocate input and output arrays memory
    double *fftin = (double*)fftw_malloc(sizeof(double)*pwidth*pheight);
    fftw_complex *fftou = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)
                                                     *pwidth*(pheight/2+1));
    double *fftre = (double*)fftw_malloc(sizeof(double)*pwidth*pheight);

    ///Create plan for fftw
    fftw_plan p = fftw_plan_dft_r2c_2d(pheight, pwidth, fftin, fftou,
                                       FFTW_MEASURE);
    fftw_plan q = fftw_plan_dft_c2r_2d(pheight, pwidth, fftou, fftre,
                                       FFTW_MEASURE);

    //Create an image file for each projection
    for(int projnum=0; projnum < NoP; projnum++){
      
      ///Create RNG; stddev here is 
      boost::normal_distribution<double> mrng(0.0,stddev*rows*sqrt(2.));
      boost::variate_generator<IMP::RandomNumberGenerator &,
                               boost::normal_distribution<double> >
                          sampler(IMP::random_number_generator, mrng);


      for(int jj = 0; jj < pheight; jj++){
        for(int kk = 0; kk < pwidth; kk++){
	  fftin[kk+pwidth*jj] = projections[projnum]->get_data().at<double>(jj,kk);
	}
      }
      fftw_execute(p);

      for(int jj = 0; jj < pheight; jj++){
        for(int kk = 0; kk < (pwidth2); kk++){
          double rndnmb1 = sampler();
          double rndnmb2 = sampler();
	  fftou[jj*(pwidth2)+kk][0] /= pheight*pwidth;
	  fftou[jj*(pwidth2)+kk][1] /= pheight*pwidth;
	  fftou[jj*(pwidth2)+kk][0]+=rndnmb1; 
	  fftou[jj*(pwidth2)+kk][1]+=rndnmb2;
        }
      }
      fftw_execute(q);
  
      std::stringstream stream;
      stream << rootname << "-" << projnum << ".pgm";
      const std::string filename0 = stream.str();
      const char* filename = filename0.c_str();
      FILE * fp;

      double MinValue = 255;
      double MaxValue = 0;
      std::vector<double> out_image(rows * cols);

      for(int jj = 0, imi = 0; jj < rows; jj++){
        for(int kk = 0; kk < cols; kk++){
          out_image[imi] = (double) fftre[(diffh+jj)*pwidth+(diffw+kk)]/(pheight*pwidth);
	  if(out_image[imi] < MinValue) MinValue = out_image[imi];
          else if(out_image[imi] > MaxValue) MaxValue = out_image[imi];
          imi++;
        }
      }

      fp = fopen(filename, "w");
      std::fprintf(fp, "P2\n");
      std::fprintf(fp,"%i %i\n", rows, cols);
      std::fprintf(fp,"255\n");
      for(int jj = 0, imi = 0; jj < rows; jj++){
        for(int kk = 0; kk < cols; kk++){
          double pixel = out_image[imi++];
	  std::fprintf(fp, "%i ", (int) ((pixel-MinValue)*(255.0)/(MaxValue-MinValue)+0));
          if(kk > 0 && kk % 12 == 0)  std::fprintf(fp, "\n");
        }
	std::fprintf(fp, "\n");
      }
      fclose(fp);
    }
    
    fftw_destroy_plan(p);
    fftw_destroy_plan(q);
    fftw_free(fftin);
    fftw_free(fftou);
    fftw_free(fftre);
  }
  
  void Generate_Pure_Noise_Images(int isize, int NoP, float stddev, std::string rootname){
    srand( time(NULL) );
    
    int rows = isize;
    int cols = isize;
    int pheight = isize;
    int pwidth = isize;
    int pwidth2 = (int) (pwidth/2.0+1.0);
    int diffw = (int) (pwidth - cols)/2.0;
    int diffh = (int) (pheight - rows)/2.0;
    
    ///Allocate input and output arrays memory
    double *fftin = (double*)fftw_malloc(sizeof(double)*pwidth*pheight);
    fftw_complex *fftou = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)
                                                     *pwidth*(pheight/2+1));
    double *fftre = (double*)fftw_malloc(sizeof(double)*pwidth*pheight);
    
    boost::normal_distribution<double> mrng(0.0,stddev);
    boost::variate_generator<IMP::RandomNumberGenerator &,
                             boost::normal_distribution<double> >
                        sampler(IMP::random_number_generator, mrng);
    
    for(int projnum=0; projnum < NoP; projnum++){
      
      fftw_plan p = fftw_plan_dft_r2c_2d(pheight, pwidth, fftin, fftou,
                                         FFTW_MEASURE);
      fftw_plan q = fftw_plan_dft_c2r_2d(pheight, pwidth, fftou, fftre,
                                         FFTW_MEASURE);
      
      for(int jj = 0; jj < pheight; jj++){
	for(int kk = 0; kk < pwidth; kk++){
	  fftin[kk+pwidth*jj] = 0;
	}
      }
      fftw_execute(p);    
      
      for(int jj = 0; jj < pheight; jj++){
	for(int kk = 0; kk < (pwidth2); kk++){
	  double rndnmb1 = sampler();
	  double rndnmb2 = sampler();
	  fftou[jj*(pwidth2)+kk][0]+=rndnmb1;
	  fftou[jj*(pwidth2)+kk][1]+=rndnmb2;
	}
      }    
      fftw_execute(q);
      
      std::stringstream stream;
      stream << rootname << "-" << projnum << ".pgm";
      const std::string filename0 = stream.str();
      const char* filename = filename0.c_str();
      FILE * fp;
      
      double MinValue = 255;
      double MaxValue = 0;
      std::vector<double> out_image(rows * cols);
      
      for(int jj = 0, imi = 0; jj < rows; jj++){
	for(int kk = 0; kk < cols; kk++){
	  out_image[imi] = (double) fftre[(diffh+jj)*pwidth+(diffw+kk)]/(pheight*pwidth); 
	  if(out_image[imi] < MinValue) MinValue = out_image[imi];
	  else if(out_image[imi] > MaxValue) MaxValue = out_image[imi]; 
          imi++;
	}
      }
      
      fp = fopen(filename, "w");
      std::fprintf(fp, "P2\n");
      std::fprintf(fp,"%i %i\n", rows, cols);
      std::fprintf(fp,"255\n");
      for(int jj = 0, imi = 0; jj < rows; jj++){
	for(int kk = 0; kk < cols; kk++){
	  double pixel = out_image[imi++];
	  std::fprintf(fp, "%i ", (int) ((pixel-MinValue)*(255.0)/(MaxValue-MinValue)+0));
	  if(kk > 0 && kk % 12 == 0)  std::fprintf(fp, "\n");
	}
	std::fprintf(fp, "\n");
      }
      fclose(fp);
      fftw_destroy_plan(p);
      fftw_destroy_plan(q);
    }    
    
    fftw_free(fftin);
    fftw_free(fftou);
    fftw_free(fftre);                    
    
  }
}

int main(int argc, char *argv[]){
  ///Command Line argument. some of them may not be necessary right now. 
  int NoP = 10;
  int isize = 256;
  float resolution = 10.0;
  float apix = 3.3;
  float stddev = 15.0;
  std::string pdb = ""; 
  std::string rootname = "Projs";
  std::string process = "None";

  po::options_description desc("Usage: <pdb> <Number of Projections> <Image Size> <Resolution> <Apix> <Standard Deviation SSNR> <Output Rootname> <process>");
  desc.add_options()
    (
     "help", "produce help message"
     ) 
    (
     "pdbfile,i", po::value<std::string>(&pdb)->default_value(""),
     "input PDB")
    (
     "resolution,r", po::value<float>(&resolution)->default_value(10.0),
     "image resolution (default = 10.0)")
    (
     "pixel-size,a", po::value<float>(&apix)->default_value(2.2),
     "images pixel size (default = 3.3)")
    (
     "projection-number,n", po::value<int>(&NoP)->default_value(10),
     "number of projections for PDBs")
    (
     "image-size,l", po::value<int>(&isize)->default_value(256),
     "size of the output images")
    (
     "root-name,o", po::value<std::string>(&rootname)->default_value("Projs"),
     "Output rootname")
    (
     "sigma,s", po::value<float>(&stddev)->default_value(15.0),
     "Standard deviation of SSNR")
    (
     "process,p", po::value<std::string>(&process)->default_value("None"),
     "What kind of projection do you want? PureNoise, Noiseless, Noisy")
    ;
  
  po::positional_options_description p;
  p.add("help", -1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }

  if(process == "PureNoise") Generate_Pure_Noise_Images(isize, NoP, stddev, rootname);
  else if(process == "Noiseless") Generate_Noiseless_Projections(pdb, NoP, isize, resolution, apix, rootname);
  else if(process == "Noisy") Generate_Noisy_Projections(pdb, NoP, isize, resolution, apix, stddev, rootname);
  else {std::cout << "Error! Choose a Process: PureNoise, Noisy, Noiseless! \n"; return 0;}
}

