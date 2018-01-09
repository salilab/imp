/**
 * Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <boost/lexical_cast.hpp>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "RMF/FileConstHandle.h"
#include "RMF/FileHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeHandle.h"
#include "RMF/Vector.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/shape.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/log.h"
#include "RMF/types.h"
#include "RMF/utility.h"
#include "common.h"

#define RMF_TRANSFORM(f, Name, attribute)       \
  if (f.get_is(nh)) {                           \
    RMF::decorator::Name d = f.get(nh);         \
    RMF::Vector3 cs = d.get_##attribute();      \
    for (unsigned int i = 0; i < 3; ++i) {      \
      cs[i] = scale * (cs[i] + translation[i]); \
    }                                           \
    d.set_##attribute(cs);                      \
  }

#define RMF_TRANSFORM_LIST(f, Name, attribute)          \
  if (f.get_is(nh)) {                                   \
    RMF::decorator::Name d = f.get(nh);                 \
    RMF::Vector3s cs = d.get_##attribute();             \
    for (unsigned int j = 0; j < cs.size(); ++j) {      \
      for (unsigned int i = 0; i < 3; ++i) {            \
        cs[j][i] = scale * (cs[j][i] + translation[i]); \
      }                                                 \
    }                                                   \
    d.set_##attribute(cs);                              \
  }

namespace {
std::string description(
    "Transform an rmf file in place. IntermediateParticle, Ball, Cylinder, "
    "Segment, RigidParticle and ReferenceFrame nodes are handled.");

void transform(RMF::NodeHandle nh,
               RMF::decorator::IntermediateParticleFactory ipf,
               RMF::decorator::RigidParticleFactory rpf,
               RMF::decorator::ReferenceFrameFactory rff,
               RMF::decorator::BallFactory bf,
               RMF::decorator::CylinderFactory cf,
               RMF::decorator::SegmentFactory sf, double scale,
               const RMF::Floats& translation) {
  RMF_TRANSFORM(ipf, IntermediateParticle, coordinates)
  else RMF_TRANSFORM(rpf, RigidParticle, coordinates);
  if (ipf.get_is(nh)) {
    RMF::decorator::IntermediateParticle d = ipf.get(nh);
    d.set_radius(scale * d.get_radius());
  }
  RMF_TRANSFORM(rff, ReferenceFrame, translation);
  RMF_TRANSFORM(bf, Ball, coordinates);
  RMF_TRANSFORM_LIST(cf, Cylinder, coordinates_list);
  RMF_TRANSFORM_LIST(sf, Segment, coordinates_list);
  RMF::NodeHandles children = nh.get_children();
  for (unsigned int i = 0; i < children.size(); ++i) {
    transform(children[i], ipf, rpf, rff, bf, cf, sf, scale, translation);
  }
}
}

int main(int argc, char** argv) {
  try {
    double scale = 1;
    std::string translation_str;
    options.add_options()("scale,s",
                          boost::program_options::value<double>(&scale),
                          "What to scale the model by")(
        "translation,t",
        boost::program_options::value<std::string>(&translation_str),
        "What to translate the model by as \"x y z\"");
    RMF_ADD_INPUT_FILE("rmf");
    RMF_ADD_OUTPUT_FILE("rmf");
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
      translation = RMF::Floats(0, 3);
    }
    RMF_INFO("Scaling model by " << scale);
    RMF_INFO("Translating model by " << translation[0] << " " << translation[1]
                                     << " " << translation[2]);

    RMF::FileConstHandle rhi = RMF::open_rmf_file_read_only(input);
    RMF::FileHandle rh = RMF::create_rmf_file(output);
    RMF::clone_file_info(rhi, rh);
    RMF::clone_hierarchy(rhi, rh);
    RMF::clone_static_frame(rhi, rh);
    RMF::decorator::IntermediateParticleFactory ipf(rh);
    RMF::decorator::RigidParticleFactory rpf(rh);
    RMF::decorator::ReferenceFrameFactory rff(rh);
    RMF::decorator::BallFactory bf(rh);
    RMF::decorator::CylinderFactory cf(rh);
    RMF::decorator::SegmentFactory sf(rh);
    RMF_FOREACH(RMF::FrameID frame, rhi.get_frames()) {
      RMF::clone_loaded_frame(rhi, rh);
      RMF_INFO("Processing frame " << frame);
      rh.set_current_frame(frame);
      transform(rh.get_root_node(), ipf, rpf, rff, bf, cf, sf, scale,
                translation);
    }
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    print_help_and_exit(argv);
  }
}
