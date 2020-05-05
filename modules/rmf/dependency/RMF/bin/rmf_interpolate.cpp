/**
 * Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <math.h>
#include <ctime>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "RMF/FileConstHandle.h"
#include "RMF/FileHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/NodeHandle.h"
#include "RMF/Vector.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/shape.h"
#include "RMF/enums.h"
#include "RMF/types.h"
#include "RMF/utility.h"
#include "common.h"

namespace {
std::string description("Generate a new file that interpolates an old one.");

#define RMF_INTERPOLATE(factory, attribute, noise, normalize, D)   \
  {                                                                \
    RMF_VECTOR<D> c0 = factory##cf0.get(input0).get_##attribute(); \
    RMF_VECTOR<D> c1 = factory##cf1.get(input1).get_##attribute(); \
    RMF_VECTOR<D> result;                                          \
    double mag2 = 0.0;                                             \
    for (unsigned int i = 0; i < D; ++i) {                         \
      result[i] = (1.0 - frac) * c0[i] + frac * c1[i] + noise();   \
      mag2 += result[i] * result[i];                               \
    }                                                              \
    if (normalize) {                                               \
      for (unsigned int i = 0; i < D; ++i) {                       \
        result[i] /= std::sqrt(mag2);                              \
      }                                                            \
    }                                                              \
    factory##f.get(output).set_##attribute(result);                \
  }

#define RMF_INTERPOLATE_LIST(factory, attribute, noise)                     \
  {                                                                         \
    RMF::Vector3s c0 = factory##cf0.get(input0).get_##attribute();          \
    RMF::Vector3s c1 = factory##cf1.get(input1).get_##attribute();          \
    RMF::Vector3s result(3, RMF::Vector3());                                \
    for (unsigned int i = 0; i < c0.size(); ++i) {                          \
      for (unsigned int j = 0; j < 3; ++j) {                                \
        result[i][j] = (1.0 - frac) * c0[i][j] + frac * c1[i][j] + noise(); \
      }                                                                     \
    }                                                                       \
    factory##f.get(output).set_##attribute(result);                         \
  }

void interpolate_frame(
    double frac, boost::variate_generator<boost::mt19937&,
                                          boost::normal_distribution<> >& noise,
    boost::variate_generator<boost::mt19937&, boost::normal_distribution<> >&
        angle_noise,
    RMF::decorator::IntermediateParticleFactory ipcf0,
    RMF::decorator::IntermediateParticleFactory ipcf1,
    RMF::decorator::IntermediateParticleFactory ipf,
    RMF::decorator::BallFactory bcf0, RMF::decorator::BallFactory bcf1,
    RMF::decorator::BallFactory bf, RMF::decorator::ReferenceFrameFactory rfcf0,
    RMF::decorator::ReferenceFrameFactory rfcf1,
    RMF::decorator::ReferenceFrameFactory rff,
    RMF::decorator::CylinderFactory ccf0, RMF::decorator::CylinderFactory ccf1,
    RMF::decorator::CylinderFactory cf, RMF::NodeConstHandle input0,
    RMF::NodeConstHandle input1, RMF::NodeHandle output) {
  if (ipcf0.get_is(input0)) {
    RMF_INTERPOLATE(ip, coordinates, noise, false, 3);
  }
  if (bcf0.get_is(input0)) {
    RMF_INTERPOLATE(b, coordinates, noise, false, 3);
  }
  if (rfcf0.get_is(input0)) {
    RMF_INTERPOLATE(rf, translation, noise, false, 3);
    RMF_INTERPOLATE(rf, rotation, angle_noise, true, 4);
  }
  if (ccf0.get_is(input0)) {
    RMF_INTERPOLATE_LIST(c, coordinates_list, noise);
  }
  RMF::NodeConstHandles ic0 = input0.get_children();
  RMF::NodeConstHandles ic1 = input1.get_children();
  RMF::NodeHandles oc = output.get_children();
  for (unsigned int i = 0; i < ic0.size(); ++i) {
    interpolate_frame(frac, noise, angle_noise, ipcf0, ipcf1, ipf, bcf0, bcf1,
                      bf, rfcf0, rfcf1, rff, ccf0, ccf1, cf, ic0[i], ic1[i],
                      oc[i]);
  }
}

void interpolate_frames(int num, double noise, double angle_noise,
                        RMF::FileConstHandle input_file0,
                        RMF::FileConstHandle input_file1,
                        RMF::FileHandle output_file) {
  RMF::decorator::IntermediateParticleFactory ipcf0(input_file0);
  RMF::decorator::IntermediateParticleFactory ipcf1(input_file1);
  RMF::decorator::IntermediateParticleFactory ipf(output_file);
  RMF::decorator::BallFactory bcf0(input_file0);
  RMF::decorator::BallFactory bcf1(input_file1);
  RMF::decorator::BallFactory bf(output_file);
  RMF::decorator::CylinderFactory ccf0(input_file0);
  RMF::decorator::CylinderFactory ccf1(input_file1);
  RMF::decorator::CylinderFactory cf(output_file);
  RMF::decorator::ReferenceFrameFactory rfcf0(input_file0);
  RMF::decorator::ReferenceFrameFactory rfcf1(input_file1);
  RMF::decorator::ReferenceFrameFactory rff(output_file);
  // boost::random_device seed_gen;
  boost::mt19937 rng;
  // rng.seed(seed_gen());
  rng.seed(static_cast<boost::uint64_t>(std::time(NULL)));
  boost::normal_distribution<> nd(0.0, noise);
  boost::variate_generator<boost::mt19937&, boost::normal_distribution<> >
      normal_random(rng, nd);
  boost::normal_distribution<> a_nd(0.0, angle_noise);
  boost::variate_generator<boost::mt19937&, boost::normal_distribution<> >
      angle_normal_random(rng, a_nd);
  for (int i = 0; i < num; ++i) {
    output_file.add_frame("interpolated", RMF::FRAME);
    double frac = static_cast<double>(i + 1) / static_cast<double>(num + 1);
    std::cout << "fraction is " << frac << " at "
              << output_file.get_current_frame() << " from "
              << input_file0.get_current_frame() << " and "
              << input_file1.get_current_frame() << std::endl;
    interpolate_frame(frac, normal_random, angle_normal_random, ipcf0, ipcf1,
                      ipf, bcf0, bcf1, bf, rfcf0, rfcf1, rff, ccf0, ccf1, cf,
                      input_file0.get_root_node(), input_file1.get_root_node(),
                      output_file.get_root_node());
  }
}
}

int main(int argc, char** argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    RMF_ADD_OUTPUT_FILE("rmf");
    int num_frames = 0;
    double noise = 0, angle_noise = 0;
    int max_frames = -1;
    options.add_options()(
        "number-interpolated", boost::program_options::value<int>(&num_frames),
        "How many frames to insert between each original frame")(
        "coordinate_noise", boost::program_options::value<double>(&noise),
        "How much (gaussian) noise to add to each generated coordinate.")(
        "angular_noise", boost::program_options::value<double>(&angle_noise),
        "How much (gaussian) noise to add to each generated quaternion.")(
        "frame_limit", boost::program_options::value<int>(&max_frames),
        "Limits how many frames are output, for testing.");
    process_options(argc, argv);
    RMF::FileConstHandle irh0 = RMF::open_rmf_file_read_only(input);
    RMF::FileConstHandle irh1 = RMF::open_rmf_file_read_only(input);
    RMF::FileHandle orh = RMF::create_rmf_file(output);
    orh.set_producer("rmf_interpolate");
    orh.set_description(std::string("Interpolation between frames of ") +
                        input);
    RMF::clone_hierarchy(irh0, orh);
    RMF::clone_static_frame(irh0, orh);
    for (unsigned int j = 0; j < irh0.get_number_of_frames() - 1; ++j) {
      std::cout << "Processing frame " << j << std::endl;
      irh0.set_current_frame(RMF::FrameID(j));
      irh1.set_current_frame(RMF::FrameID(j + 1));
      orh.add_frame(irh0.get_name(RMF::FrameID(j)), RMF::FRAME);
      RMF::clone_loaded_frame(irh0, orh);
      if (j + 1 < irh0.get_number_of_frames()) {
        interpolate_frames(num_frames, noise, angle_noise, irh0, irh1, orh);
      }
      if (max_frames > 0 &&
          orh.get_number_of_frames() > static_cast<unsigned int>(max_frames)) {
        return 1;
      }
    }
    irh0.set_current_frame(RMF::FrameID(irh0.get_number_of_frames() - 1));
    RMF::clone_loaded_frame(irh0, orh);

    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
