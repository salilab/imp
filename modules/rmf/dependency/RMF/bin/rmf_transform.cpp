/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <RMF/log.h>
#include <RMF/decorators.h>
#include <sstream>
#include "common.h"

#define RMF_TRANSFORM(f, Name, attribute) \
 if (f.get_is(nh)) {\
   RMF::Name d = f.get(nh);\
   RMF::Floats cs = d.get_##attribute();\
   for (unsigned int i = 0; i < 3; ++i) { \
    cs[i] = scale * (cs[i] + translation[i]);\
   }\
   d.set_##attribute(cs);\
 }

 #define RMF_TRANSFORM_LIST(f, Name, attribute) \
 if (f.get_is(nh)) {\
   RMF::Name d = f.get(nh);\
   RMF::FloatsList cs = d.get_##attribute();\
   for (unsigned int i = 0; i < 3; ++i) { \
    for (unsigned int j = 0; j < cs[i].size(); ++j) { \
      cs[i][j] = scale * (cs[i][j] + translation[i]);\
    }\
  }\
  d.set_##attribute(cs);\
 }

namespace {
 std::string description("Transform an rmf file in place. IntermediateParticle, Ball, Cylinder, Segment, RigidParticle and ReferenceFrame nodes are handled.");

  void transform(RMF::NodeHandle nh,
                 RMF::IntermediateParticleFactory ipf,
                 RMF::RigidParticleFactory rpf,
                 RMF::ReferenceFrameFactory rff,
                 RMF::BallFactory bf,
                 RMF::CylinderFactory cf,
                 RMF::SegmentFactory sf,
                 double scale,
                 const RMF::Floats &translation) {
    RMF_TRANSFORM(ipf, IntermediateParticle, coordinates)
    else RMF_TRANSFORM(rpf, RigidParticle, coordinates);
    if (ipf.get_is(nh)) {
      RMF::IntermediateParticle d = ipf.get(nh);
      d.set_radius(scale*d.get_radius());
    }
    RMF_TRANSFORM(rff, ReferenceFrame, translation);
    RMF_TRANSFORM(bf, Ball, coordinates);
    RMF_TRANSFORM_LIST(cf, Cylinder, coordinates);
    RMF_TRANSFORM_LIST(sf, Segment, coordinates);
    RMF::NodeHandles children = nh.get_children();
    for (unsigned int i=0; i < children.size(); ++i) {
      transform(children[i], ipf, rpf, rff, bf, cf, sf, scale, translation);
    }
  }
}


int main(int argc, char **argv) {
  try {
    double scale=1;
    std::string translation_str;
    options.add_options()
      ("scale,s", boost::program_options::value< double >(&scale),
      "What to scale the model by")
      ("translation,t", boost::program_options::value< std::string >(&translation_str),
      "What to translate the model by as \"x y z\"");
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);
    RMF::Floats translation;
    if (!translation_str.empty()) {
      std::istringstream iss(translation_str);
      for (unsigned int i = 0; i < 3; ++i) {
        double c;
        iss >> c;
        if (!iss) {
          std::cerr << "Bad translation" << std::endl;
          print_help_and_exit(argv);
        }
        translation.push_back(c);
      }
    } else {
      translation= RMF::Floats(0, 3);
    }
    RMF_INFO(RMF::get_logger(), "Scaling model by " << scale);
    RMF_INFO(RMF::get_logger(), "Translating model by " << translation[0]
             << " " << translation[1] << " " << translation[2]);

    RMF::FileHandle rh = RMF::open_rmf_file(input);
    int num_frames= rh.get_number_of_frames();
    RMF_INFO(RMF::get_logger(), "File has " << num_frames << " frames");
    RMF::IntermediateParticleFactory ipf(rh);
    RMF::RigidParticleFactory rpf(rh);
    RMF::ReferenceFrameFactory rff(rh);
    RMF::BallFactory bf(rh);
    RMF::CylinderFactory cf(rh);
    RMF::SegmentFactory sf(rh);
    for (int i= RMF::ALL_FRAMES; i < num_frames; ++i) {
      RMF_INFO(RMF::get_logger(), "Processing frame " << i);
      rh.set_current_frame(i);
      transform(rh.get_root_node(), ipf, rpf, rff, bf, cf, sf, scale, translation);
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    print_help_and_exit(argv);
  }
}
