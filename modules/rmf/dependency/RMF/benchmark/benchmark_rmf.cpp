/** \example benchmark/benchmark_rmf.cpp
 * \brief Benchmark typical creation, traversal and loading with different RMF
 *        backends.
 *
 * Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <boost/iterator/iterator_facade.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/timer.hpp>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "RMF/BufferHandle.h"
#include "RMF/FileConstHandle.h"
#include "RMF/FileHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/NodeHandle.h"
#include "RMF/Vector.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/sequence.h"
#include "RMF/enums.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/log.h"

namespace {
#ifndef NDEBUG
const int scale = 1;
#else
const int scale = 5;
#endif

std::string show_size(unsigned int sz) {
  std::ostringstream oss;
  if (sz > 1000000) {
    oss << sz / 1000000 << "M";
  } else if (sz > 1000) {
    oss << sz / 1000 << "k";
  } else {
    oss << sz << "b";
  }
  return oss.str();
}

void benchmark_size(std::string path, std::string type) {
  unsigned int size = 0;
  if (boost::filesystem::is_directory(path)) {
    for (boost::filesystem::directory_iterator it(path);
         it != boost::filesystem::directory_iterator(); it++) {
      size += boost::filesystem::file_size(*it);
    }
  } else {
    size = boost::filesystem::file_size(path);
  }
  std::cout << type << ", size, " << show_size(size) << ", " << size
            << std::endl;
}

std::size_t create_residue(RMF::NodeHandle nh, RMF::decorator::AtomFactory af,
                           RMF::decorator::ParticleFactory pf) {
  std::size_t total_size = 0;
  for (unsigned int i = 0; i < 2 * scale; ++i) {
    RMF::NodeHandle child = nh.add_child("CA", RMF::REPRESENTATION);
    pf.get(child).set_static_mass(1);
    pf.get(child).set_static_radius(1.0 + i / 18.77);
    af.get(child).set_static_element(7);
    total_size += sizeof(int) * 1 + sizeof(float) * 2;
  }
  return total_size;
}
std::size_t create_chain(RMF::NodeHandle nh, RMF::decorator::ResidueFactory rf,
                         RMF::decorator::AtomFactory af,
                         RMF::decorator::ParticleFactory pf) {
  std::size_t total_size = 0;
  for (unsigned int i = 0; i < 60 * scale; ++i) {
    std::ostringstream oss;
    oss << i;
    RMF::NodeHandle child = nh.add_child(oss.str(), RMF::REPRESENTATION);
    rf.get(child).set_static_residue_type("cys");
    rf.get(child).set_static_residue_index(i);
    total_size += sizeof(int) + 4;
    total_size += create_residue(child, af, pf);
  }
  return total_size;
}
std::size_t create_hierarchy(RMF::FileHandle file) {
  RMF::decorator::ChainFactory cf(file);
  RMF::decorator::AtomFactory af(file);
  RMF::decorator::ResidueFactory rf(file);
  RMF::decorator::ParticleFactory pf(file);
  RMF::NodeHandle n = file.get_root_node();
  std::size_t total_size = 0;
  for (unsigned int i = 0; i < 3 * scale; ++i) {
    std::ostringstream oss;
    oss << i;
    RMF::NodeHandle child = n.add_child(oss.str(), RMF::REPRESENTATION);
    cf.get(child).set_static_chain_id(oss.str());
    total_size += oss.str().size();
    total_size += create_chain(child, rf, af, pf);
  }
  return total_size;
}

std::pair<double, std::size_t> create_frame(RMF::FileHandle fh,
                                            RMF::decorator::ParticleFactory ipf,
                                            const RMF::NodeIDs& atoms,
                                            int frame) {
  RMF::Vector3 ret(0, 0, 0);
  std::size_t total_size = 0;
  RMF_FOREACH(RMF::NodeID n, atoms) {
    RMF::Vector3 v((n.get_index() + 0 + frame) / 17.0,
                   (n.get_index() + 1 + frame) / 19.0,
                   (n.get_index() + 2 + frame) / 23.0);
    ret[0] += v[0];
    ret[1] += v[1];
    ret[2] += v[2];
    ipf.get(fh.get_node(n)).set_frame_coordinates(v);
    total_size += sizeof(float) * 3;
  }
  return std::make_pair(ret[0] + ret[1] + ret[2], total_size);
}

boost::tuple<std::size_t> create(RMF::FileHandle file, RMF::NodeIDs& atoms) {
  std::size_t hierarchy_size = create_hierarchy(file);
  RMF_FOREACH(RMF::NodeID n, file.get_node_ids()) {
    if (file.get_node(n).get_children().empty()) {
      atoms.push_back(n);
    }
  }
  return boost::make_tuple(hierarchy_size);
}

boost::tuple<double, std::size_t> create_frames(RMF::FileHandle file,
                                                const RMF::NodeIDs& atoms) {
  RMF::decorator::ParticleFactory ipf(file);
  double check_value = 0;
  std::size_t frame_size = 0;
  for (unsigned int i = 0; i < 20; ++i) {
    file.add_frame("frame", RMF::FRAME);
    std::pair<double, std::size_t> cur = create_frame(file, ipf, atoms, i);
    check_value += cur.first;
    frame_size += cur.second;
  }
  return boost::make_tuple(check_value, frame_size);
}

double traverse(RMF::FileConstHandle file) {
  double ret = 0;
  RMF::NodeConstHandles queue(1, file.get_root_node());
  RMF::decorator::ParticleFactory ipcf(file);
  do {
    RMF::NodeConstHandle cur = queue.back();
    queue.pop_back();
    if (ipcf.get_is(cur)) {
      ret += ipcf.get(cur).get_radius();
    }
    RMF::NodeConstHandles children = cur.get_children();
    queue.insert(queue.end(), children.begin(), children.end());
  } while (!queue.empty());
  return ret;
}

double load(RMF::FileConstHandle file, const RMF::NodeIDs& nodes) {
  RMF::decorator::IntermediateParticleFactory ipcf(file);
  RMF::Vector3 v(0, 0, 0);
  RMF_FOREACH(RMF::FrameID fr, file.get_frames()) {
    file.set_current_frame(fr);
    RMF_FOREACH(RMF::NodeID n, nodes) {
      RMF::Vector3 cur = ipcf.get(file.get_node(n)).get_coordinates();
      v[0] += cur[0];
      v[1] += cur[1];
      v[2] += cur[2];
    }
  }
  return v[0] + v[1] + v[2];
}

std::pair<std::size_t, std::size_t> benchmark_create(RMF::FileHandle file,
                                                     std::string type) {
  RMF::NodeIDs atoms;
  boost::timer timer;
  boost::tuple<std::size_t> cur = create(file, atoms);
  std::cout << type << ", create, " << timer.elapsed() << ", " << cur.get<0>()
            << std::endl;
  boost::timer frame_timer;
  boost::tuple<double, std::size_t> frames = create_frames(file, atoms);
  std::cout << type << ", create frame, " << frame_timer.elapsed() / 20.0
            << ", " << frames.get<0>() << std::endl;
  return std::make_pair(cur.get<0>(), frames.get<1>());
}

void benchmark_traverse(RMF::FileConstHandle file, std::string type) {
  file.set_current_frame(RMF::FrameID(0));
  boost::timer timer;
  double count = 0;
  double t;
  while (timer.elapsed() < 1) {
    t = traverse(file);
    ++count;
  }
  std::cout << type << ", traverse, " << timer.elapsed() / count << ", " << t
            << std::endl;
}

void benchmark_load(RMF::FileConstHandle file, std::string type) {
  RMF::NodeIDs nodes;
  RMF::decorator::ParticleFactory ipcf(file);
  RMF_FOREACH(RMF::NodeID n, file.get_node_ids()) {
    if (ipcf.get_is(file.get_node(n))) nodes.push_back(n);
  }
  boost::timer timer;
  double dist = load(file, nodes);
  std::cout << type << ", load, " << timer.elapsed() / 20.0 << ", " << dist
            << std::endl;
}

RMF::FileConstHandle benchmark_open(std::string path, std::string type) {
  boost::timer timer;
  RMF::FileConstHandle ret;
  double count = 0;
  while (timer.elapsed() < 1) {
    ret = RMF::open_rmf_file_read_only(path);
    ++count;
  }
  std::cout << type << ", open, " << timer.elapsed() / count << ", 0"
            << std::endl;
  return ret;
}
}  // namespace

int main(int, char**) {
  try {
    RMF::set_log_level("Off");
    std::string name_base = RMF::internal::get_unique_path();
#ifndef NDEBUG
    std::cout << name_base << std::endl;
#endif
    {
      const std::string name = name_base + ".rmf";
      {
        RMF::FileHandle fh = RMF::create_rmf_file(name);
        std::pair<std::size_t, std::size_t> sizes = benchmark_create(fh, "rmf");
        std::cout << "raw, total, " << show_size(sizes.first + sizes.second)
                  << ", " << (sizes.first + sizes.second) << std::endl;
        std::cout << "raw, frame, " << show_size(sizes.second) << ", "
                  << sizes.second << std::endl;
      }
      {
        RMF::FileConstHandle fh = benchmark_open(name, "rmf");
        benchmark_traverse(fh, "rmf");
        benchmark_load(fh, "rmf");
      }
      benchmark_size(name, "rmf");
    }
    {
      const std::string name = name_base + ".rmfz";
      {
        RMF::FileHandle fh = RMF::create_rmf_file(name);
        benchmark_create(fh, "rmfz");
      }
      {
        RMF::FileConstHandle fh = benchmark_open(name, "rmfz");
        benchmark_traverse(fh, "rmfz");
        benchmark_load(fh, "rmfz");
      }
      benchmark_size(name, "rmfz");
    }
    {
      RMF::BufferHandle buffer;
      {
        RMF::FileHandle fh = RMF::create_rmf_buffer(buffer);
        benchmark_create(fh, "buffer");
      }
      {
        boost::timer timer;
        RMF::FileConstHandle fh = RMF::open_rmf_buffer_read_only(buffer);
        std::cout << "buffer"
                  << ", open, " << timer.elapsed() << ", 0" << std::endl;
        benchmark_traverse(fh, "buffer");
        benchmark_load(fh, "buffer");
      }
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Exception thrown: " << e.what() << std::endl;
  }
  return 0;
}
