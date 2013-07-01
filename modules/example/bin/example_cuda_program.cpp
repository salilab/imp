/** \example example_cuda_program.cpp
    \brief Show how to call a cuda from from a .cpp file.

    Copyright 2007-2013 IMP Inventors. All rights reserved.
*/


#include <IMP/algebra/VectorD.h>
#include <IMP/base/flags.h>
#include <IMP/Model.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/pdb.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <boost/scoped_array.hpp>
#include <IMP/base/utility.h>

#include <iostream>

#if IMP_HAS_CUDA
// Required to include CUDA vector types
#include <cuda_runtime.h>
#include <vector_types.h>
#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////
// declaration, forward
extern "C" void get_distance_histogram_gpu(int argc, char **argv,
                                           int len, float3 *coords,
                                           int hist_size, float *nhis);
namespace {
  void get_distance_histogram(const IMP::algebra::Vector3Ds &in,
                              IMP::Floats& hist, float delta) {
    float one_over_delta = 1.0/delta;
    for (unsigned int i = 0; i < in.size(); ++i) {
      for (unsigned int j = i+1; j < in.size(); ++j) {
        float dist = IMP::algebra::get_distance(in[i], in[j]);
        int index = dist / delta;
        hist[index]+=1.0;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int
main(int argc, char **argv)
{
  IMP::Strings file
      = IMP::base::setup_from_argv(argc, argv,
                                   "Compute a distance histogram with CUDA.",
                                   "file.pdb", 1);
  IMP::base::Pointer<IMP::Model> model = new IMP::Model();

  // check if file exists
  std::ifstream in_file(file[0]);
  if(!in_file) {
    std::cerr << "Can't open file " << file[0] << std::endl;
    exit(1);
  }

  // read pdb
  IMP::atom::Hierarchy mhd =
    IMP::atom::read_pdb(file[0], model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                        // don't add radii
                        true, true);
  IMP::Particles particles
    = IMP::get_as<IMP::Particles>(get_by_type(mhd, IMP::atom::ATOM_TYPE));
  if(particles.size() > 0) { // pdb file
    std::cout << particles.size() << " atoms were read from PDB file "
              << file[0] << std::endl;
  }
  //unsigned int num = 1024;

  IMP::algebra::Vector3Ds coordinates(particles.size());
  boost::scoped_array<float3> coordinates2(new float3[coordinates.size()]);

  for(unsigned int i = 0; i < coordinates.size(); i++) {
    coordinates[i] = IMP::core::XYZ(particles[i]).get_coordinates();
    coordinates2[i].x = coordinates[i][0];
    coordinates2[i].y = coordinates[i][1];
    coordinates2[i].z = coordinates[i][2];
  }

  // estimate Dmax
  IMP::algebra::BoundingBox3D bb(coordinates);
  float dmax = IMP::algebra::get_distance(bb.get_corner(0), bb.get_corner(1));
  float delta = 0.5;
  int hist_size =  (int)std::ceil(dmax / delta) + 1;

  std::cerr << "dmax estimate " << dmax
            << " hist_size " << hist_size << std::endl;

  boost::scoped_array<float> distance_hist_gpu(new float[hist_size]);
  {
    boost::timer gpu_time;

    get_distance_histogram_gpu(argc, argv,
                               coordinates.size(),
                               coordinates2.get(), hist_size,
                               distance_hist_gpu.get());

    std::cout << "GPU " << gpu_time.elapsed() << std::endl;
  }

  IMP::Floats distance_hist_cpu(hist_size, 0.0);
  {
    boost::timer cpu_time;

    get_distance_histogram(coordinates, distance_hist_cpu, delta);

    std::cout << "CPU " << cpu_time.elapsed() << std::endl;
  }

  std::cout << distance_hist_cpu.size() << " bins" << std::endl;

  int differ = 0;
  for (unsigned int i = 0; i < distance_hist_cpu.size(); ++i) {
    float error = distance_hist_gpu[i] - distance_hist_cpu[i];

    if (error > .01) {
      ++differ;
      std::cout << i << " GPU = " << distance_hist_gpu[i]
                << " CPU = " << distance_hist_cpu[i]
                << " diff = " << error
                << " % " << error/distance_hist_cpu[i] << std::endl;
    }
  }

  std::cout << "Errors: " << differ << " out of " << hist_size << std::endl;
  return 0;
}

#else
int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv, "No CUDA available");
  std::cerr << "No CUDA support." << std::endl;
  return 1;
}
#endif
