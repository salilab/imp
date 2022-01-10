/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/utility.h"

#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <iostream>
#include <string>

#include "RMF/FileConstHandle.h"
#include "RMF/FileHandle.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/NodeHandle.h"
#include "RMF/compiler_macros.h"
#include "RMF/decorator/alias.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/shape.h"
#include "RMF/CoordinateTransformer.h"
#include "RMF/exceptions.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/utility.h"
#include "internal/clone_shared_data.h"
#include "internal/shared_data_equality.h"
#include <limits>

RMF_ENABLE_WARNINGS

namespace RMF {

void clone_file_info(FileConstHandle input, FileHandle output) {
  internal::clone_file(input.shared_.get(), output.shared_.get());
}

void clone_hierarchy(FileConstHandle input, FileHandle output) {
  internal::clone_hierarchy(input.shared_.get(), output.shared_.get());
}

void clone_loaded_frame(FileConstHandle input, FileHandle output) {
  internal::clone_loaded_data(input.shared_.get(), output.shared_.get());
}

void clone_static_frame(FileConstHandle input, FileHandle output) {
  internal::clone_static_data(input.shared_.get(), output.shared_.get());
}

namespace {
bool get_equal_node_structure(NodeConstHandle in, NodeConstHandle out,
                              bool print_diff) {
  bool ret = true;
  if (in.get_type() != out.get_type()) {
    if (print_diff) {
      std::cout << "Node types differ at " << in << " vs " << out << std::endl;
    }
    ret = false;
  }
  if (in.get_name() != out.get_name()) {
    if (print_diff) {
      std::cout << "Node names differ at " << in << " vs " << out << std::endl;
    }
    ret = false;
  }
  NodeConstHandles inch = in.get_children();
  NodeConstHandles outch = out.get_children();
  if (inch.size() != outch.size()) {
    if (print_diff) {
      std::cout << "Node number of children differ at " << in << " vs " << out
                << std::endl;
    }
    ret = false;
  }
  for (unsigned int i = 0; i < std::min(inch.size(), outch.size()); ++i) {
    ret = get_equal_node_structure(inch[i], outch[i], print_diff) && ret;
  }
  return ret;
}
}

bool get_equal_structure(FileConstHandle in, FileConstHandle out,
                         bool print_diff) {
  bool ret = true;
  ret = get_equal_node_structure(in.get_root_node(), out.get_root_node(),
                                 print_diff) &&
        ret;
  return ret;
}

bool get_equal_current_values(FileConstHandle in, FileConstHandle out) {
  return internal::get_equal_current_values(in.shared_.get(),
                                            out.shared_.get());
}

bool get_equal_static_values(FileConstHandle in, FileConstHandle out) {
  return internal::get_equal_static_values(in.shared_.get(), out.shared_.get());
}

void test_throw_exception() {
  RMF_THROW(Message("Test exception"), UsageException);
}

namespace {
void handle_vector(const CoordinateTransformer &tr, const Vector3 &v, float r,
                   boost::array<RMF::Vector3, 2> &bb) {
  Vector3 trv = tr.get_global_coordinates(v);
  for (unsigned int i = 0; i < 3; ++i) {
    bb[0][i] = std::min(trv[i] - r, bb[0][i]);
    bb[1][i] = std::max(trv[i] + r, bb[1][i]);
  }
}
void get_bounding_box_impl(NodeConstHandle root, CoordinateTransformer tr,
                           decorator::IntermediateParticleFactory ipf,
                           decorator::BallFactory bf,
                           decorator::SegmentFactory sf,
                           decorator::CylinderFactory cf,
                           decorator::GaussianParticleFactory gpf,
                           decorator::ReferenceFrameFactory rff,
                           boost::array<RMF::Vector3, 2> &bb) {
  if (rff.get_is(root)) {
    tr = CoordinateTransformer(tr, rff.get(root));
  }
  if (ipf.get_is(root)) {
    RMF::decorator::IntermediateParticleConst pc = ipf.get(root);
    double r = pc.get_radius();
    RMF::Vector3 c = pc.get_coordinates();
    handle_vector(tr, c, r, bb);
  }
  if (gpf.get_is(root)) {
    Vector3 var = gpf.get(root).get_variances();
    handle_vector(tr, Vector3(0, 0, 0),
                  *std::max_element(var.begin(), var.end()) * 5, bb);
  }
  if (bf.get_is(root)) {
    handle_vector(tr, bf.get(root).get_coordinates(), bf.get(root).get_radius(),
                  bb);
  }
  if (sf.get_is(root)) {
    RMF_FOREACH(const Vector3 & v, sf.get(root).get_coordinates_list()) {
      handle_vector(tr, v, cf.get_is(root) ? cf.get(root).get_radius() : 0.0f,
                    bb);
    }
  }
  RMF_FOREACH(NodeConstHandle ch, root.get_children()) {
    get_bounding_box_impl(ch, tr, ipf, bf, sf, cf, gpf, rff, bb);
  }
}
}

boost::array<RMF::Vector3, 2> get_bounding_box(NodeConstHandle root) {
  boost::array<RMF::Vector3, 2> ret;
  float v = std::numeric_limits<float>::max();
  ret[0] = RMF::Vector3(v, v, v);
  ret[1] = RMF::Vector3(-v, -v, -v);
  FileConstHandle fh = root.get_file();
  get_bounding_box_impl(
      root, CoordinateTransformer(), decorator::IntermediateParticleFactory(fh),
      decorator::BallFactory(fh), decorator::SegmentFactory(fh),
      decorator::CylinderFactory(fh), decorator::GaussianParticleFactory(fh),
      decorator::ReferenceFrameFactory(fh), ret);
  return ret;
}

float get_diameter(NodeConstHandle root) {
  boost::array<RMF::Vector3, 2> bb = get_bounding_box(root);
  float max = 0;
  for (unsigned int i = 0; i < 3; ++i) {
    max = std::max(bb[1][i] - bb[0][i], max);
  }
  return max;
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
