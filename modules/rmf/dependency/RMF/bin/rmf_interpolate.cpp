/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <RMF/utility.h>
#include <RMF/decorators.h>
#include <boost/random.hpp>
/*#if BOOST_VERSION < 104100
#include <boost/nondet_random.hpp>
#else
#include <boost/random/random_device.hpp>
#endif*/
#include "common.h"
#include <ctime>

namespace {
  std::string description("Generate a new file that interpolates an old one.");

#define RMF_INTERPOLATE(factory, attribute, noise, normalize)           \
  {                                                                     \
    RMF::Floats c0 = factory##cf0.get(input0).get_##attribute();        \
    RMF::Floats c1 = factory##cf1.get(input1).get_##attribute();        \
    RMF::Floats result(c0.size());                                      \
    double mag2 = 0.0;                                                  \
    for (unsigned int i = 0; i < c0.size(); ++i) {                      \
      result[i] = (1.0 - frac) * c0[i] + frac * c1[i] + noise();        \
      mag2 += result[i] * result[i];                                    \
    }                                                                   \
    if (normalize) {                                                    \
      for (unsigned int i = 0; i < c0.size(); ++i) {                    \
        result[i] /= std::sqrt(mag2);                                   \
      }                                                                 \
    }                                                                   \
    factory##f.get(output).set_##attribute(result);                     \
  }

#define RMF_INTERPOLATE_LIST(factory, attribute, noise)                 \
  {                                                                     \
    RMF::FloatsList c0 = factory##cf0.get(input0).get_##attribute();    \
    RMF::FloatsList c1 = factory##cf1.get(input1).get_##attribute();    \
    RMF::FloatsList result(3, RMF::Floats(c0.size()));                  \
    for (unsigned int j = 0; j < 3; ++j) {                              \
      for (unsigned int i = 0; i < c0.size(); ++i) {                    \
        result[j][i] = (1.0 - frac) * c0[j][i] + frac * c1[j][i] + noise(); \
      }                                                                 \
    }                                                                   \
    factory##f.get(output).set_##attribute(result);                     \
  }

  void interpolate_frame(double frac, boost::variate_generator<boost::mt19937&,
                         boost::normal_distribution<> >& noise,
                         boost::variate_generator<boost::mt19937&,
                         boost::normal_distribution<> >& angle_noise,
                         RMF::IntermediateParticleConstFactory ipcf0,
                         RMF::IntermediateParticleConstFactory ipcf1,
                         RMF::IntermediateParticleFactory ipf,
                         RMF::BallConstFactory bcf0,
                         RMF::BallConstFactory bcf1,
                         RMF::BallFactory bf,
                         RMF::ReferenceFrameConstFactory rfcf0,
                         RMF::ReferenceFrameConstFactory rfcf1,
                         RMF::ReferenceFrameFactory rff,
                         RMF::CylinderConstFactory ccf0,
                         RMF::CylinderConstFactory ccf1,
                         RMF::CylinderFactory cf,
                         RMF::NodeConstHandle input0,
                         RMF::NodeConstHandle input1,
                         RMF::NodeHandle output) {
    if (ipcf0.get_is(input0)) {
      RMF_INTERPOLATE(ip, coordinates, noise, false);
    }
    if (bcf0.get_is(input0)) {
      RMF_INTERPOLATE(b, coordinates, noise, false);
    }
    if (rfcf0.get_is(input0)) {
      RMF_INTERPOLATE(rf, translation, noise, false);
      RMF_INTERPOLATE(rf, rotation, angle_noise, true);
    }
    if (ccf0.get_is(input0)) {
      RMF_INTERPOLATE_LIST(c, coordinates, noise);
    }
    RMF::NodeConstHandles ic0 = input0.get_children();
    RMF::NodeConstHandles ic1 = input1.get_children();
    RMF::NodeHandles oc = output.get_children();
    for (unsigned int i = 0; i < ic0.size(); ++i) {
      interpolate_frame(frac, noise, angle_noise,
                        ipcf0, ipcf1, ipf, bcf0, bcf1, bf,
                        rfcf0, rfcf1, rff, ccf0, ccf1, cf,
                        ic0[i], ic1[i], oc[i]);
    }
  }

  void interpolate_frames(int num, double noise, double angle_noise,
                          RMF::FileConstHandle input_file0,
                          RMF::FileConstHandle input_file1,
                          RMF::FileHandle output_file) {
    RMF::IntermediateParticleConstFactory ipcf0(input_file0);
    RMF::IntermediateParticleConstFactory ipcf1(input_file1);
    RMF::IntermediateParticleFactory ipf(output_file);
    RMF::BallConstFactory bcf0(input_file0);
    RMF::BallConstFactory bcf1(input_file1);
    RMF::BallFactory bf(output_file);
    RMF::CylinderConstFactory ccf0(input_file0);
    RMF::CylinderConstFactory ccf1(input_file1);
    RMF::CylinderFactory cf(output_file);
    RMF::ReferenceFrameConstFactory rfcf0(input_file0);
    RMF::ReferenceFrameConstFactory rfcf1(input_file1);
    RMF::ReferenceFrameFactory rff(output_file);
    //boost::random_device seed_gen;
    boost::mt19937 rng;
    //rng.seed(seed_gen());
    rng.seed(static_cast<boost::uint64_t>(std::time(NULL)));
    boost::normal_distribution<> nd(0.0, noise);
    boost::variate_generator<boost::mt19937&,
    boost::normal_distribution<> > normal_random(rng, nd);
    boost::normal_distribution<> a_nd(0.0, angle_noise);
    boost::variate_generator<boost::mt19937&,
    boost::normal_distribution<> > angle_normal_random(rng, a_nd);
    for (int i = 0; i < num; ++i) {
      output_file.get_current_frame().add_child("interpolated",
                                                RMF::FRAME).set_as_current_frame();
      double frac = static_cast<double>(i+1)/static_cast<double>(num+1);
      std:: cout << "fraction is " << frac
                 << " at " << output_file.get_current_frame()
                 << " from " << input_file0.get_current_frame()
                 << " and " << input_file1.get_current_frame() << std::endl;
      interpolate_frame(frac,
                        normal_random, angle_normal_random,
                        ipcf0, ipcf1, ipf, bcf0, bcf1, bf, rfcf0, rfcf1, rff,
                        ccf0, ccf1, cf,
                        input_file0.get_root_node(),
                        input_file1.get_root_node(),
                        output_file.get_root_node());
    }
  }
}

int main(int argc, char **argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    RMF_ADD_OUTPUT_FILE("rmf");
    int num_frames=0;
    double noise=0, angle_noise=0;
    int max_frames=-1;
    options.add_options()
      ("number-interpolated", boost::program_options::value< int >(&num_frames),
       "How many frames to insert between each original frame")
      ("coordinate_noise", boost::program_options::value< double >(&noise),
       "How much (gaussian) noise to add to each generated coordinate.")
      ("angular_noise", boost::program_options::value< double >(&angle_noise),
       "How much (gaussian) noise to add to each generated quaternion.")
      ("frame_limit", boost::program_options::value< int >(&max_frames),
       "Limits how many frames are output, for testing.");
    process_options(argc, argv);
    RMF::FileConstHandle irh0 = RMF::open_rmf_file_read_only(input);
    RMF::FileConstHandle irh1 = RMF::open_rmf_file_read_only(input);
    RMF::FileHandle orh = RMF::create_rmf_file(output);
    RMF::copy_structure(irh0, orh);
    int out_frame = 0;
    irh0.set_current_frame(RMF::ALL_FRAMES);
    orh.set_current_frame(RMF::ALL_FRAMES);
    RMF::copy_frame(irh0, orh);
    for (unsigned int j = 0; j < irh0.get_number_of_frames(); ++j) {
      std::cout << "Processing frame " << j << std::endl;
      irh0.set_current_frame(j);
      orh.get_current_frame().add_child(irh0.get_current_frame().get_name(),
                                        RMF::FRAME).set_as_current_frame();
      RMF::copy_frame(irh0, orh);
      if (j + 1 < irh0.get_number_of_frames()) {
        irh1.set_current_frame(j+1);
        interpolate_frames(num_frames, noise, angle_noise, irh0, irh1, orh);
      }
      if (max_frames > 0
          && orh.get_number_of_frames() > static_cast<unsigned int>(max_frames)) {
        return 1;
      }
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
