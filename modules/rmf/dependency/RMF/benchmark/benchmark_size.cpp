/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include <boost/version.hpp>
#include <sstream>

namespace {

unsigned int get_size(boost::filesystem::path path) {
  if (boost::filesystem::is_directory(path)) {
    unsigned int ret = 0;
    for (boost::filesystem::directory_iterator it(path);
         it != boost::filesystem::directory_iterator(); it++) {
      ret += get_size(*it);
    }
    return ret;
  } else {
    return boost::filesystem::file_size(path);
  }
}

void fill_atoms(RMF::FileHandle fh, RMF::NodeHandle nh) {
  RMF::AtomFactory af(fh);
  for (unsigned int i = 0; i < 20; ++i) {
    RMF::NodeHandle cur = nh.add_child("atom", RMF::REPRESENTATION);
    RMF::Atom a = af.get(nh);
    a.set_radius(double(rand()) / RAND_MAX);
    a.set_mass(double(rand()) / RAND_MAX);
    a.set_element(14);
  }
}


void fill_residues(RMF::FileHandle fh, RMF::NodeHandle nh) {
  RMF::ResidueFactory rf(fh);
  for (unsigned int i = 0; i < 100; ++i) {
    RMF::NodeHandle cur = nh.add_child("residue", RMF::REPRESENTATION);
    RMF::Residue r = rf.get(nh);
    r.set_index(i);
    r.set_type("ALA");
    fill_atoms(fh, cur);
  }
}

void fill_chains(RMF::FileHandle fh, RMF::NodeHandle nh) {
  RMF::ChainFactory cf(fh);
  for (unsigned int i = 0; i < 10; ++i) {
    RMF::NodeHandle cur = nh.add_child("chain", RMF::REPRESENTATION);
    RMF::Chain c = cf.get(cur);
    c.set_chain_id(i);
    fill_residues(fh, cur);
  }
}

void fill_file(RMF::FileHandle fh) {
  RMF::NodeHandle nh = fh.get_root_node();
  fill_chains(fh, nh);
}

void add_coordinates(RMF::NodeHandle      nh,
                     RMF::ParticleFactory pf) {
  RMF::NodeHandles nhs = nh.get_children();
  if (nhs.size() > 0) {
    for (unsigned int i = 0; i < nhs.size(); ++i) {
      add_coordinates(nhs[i], pf);
    }
  } else {
    RMF::Particle p = pf.get(nh);
    RMF::Floats coords(3);
    coords[0] = double(rand()) / RAND_MAX;
    coords[1] = double(rand()) / RAND_MAX;
    coords[2] = double(rand()) / RAND_MAX;
    p.set_coordinates(coords);
  }
}

void add_frame(RMF::FileHandle fh) {
  RMF::FrameHandle fr = fh.get_current_frame().add_child("frame", RMF::FRAME);
  RMF::ParticleFactory pf(fh);
      add_coordinates(fh.get_root_node(), pf);
}

void benchmark(std::string type, boost::filesystem::path path,
               unsigned int nframes) {
  boost::timer timer;
  {
    RMF::FileHandle fh = RMF::create_rmf_file(path.string());
    fill_file(fh);
    for (unsigned int i = 0; i < nframes; ++i) {
      add_frame(fh);
    }
  }
  std::cout << type << " with " << nframes << " frames is " << get_size(path)
            << " in " << timer.elapsed() << std::endl;
}

}


int main(int, char **) {
  std::string suffixes[] = {"rmf", "rmf2", "rmfa"};
  try {
#if BOOST_VERSION > 104400
    boost::filesystem::path temp
      = boost::filesystem::unique_path(boost::filesystem::temp_directory_path()
                                       / "%%%%-%%%%-%%%%-%%%%");
    boost::filesystem::create_directory(temp);
#else
    boost::filesystem::path temp(".");
#endif
    std::cout << "File are in " << temp << std::endl;
    for (unsigned int i = 0; i < 3; ++i) {
      boost::filesystem::path path1 = temp / (std::string("file.1.") + suffixes[i]);
      benchmark(suffixes[i], path1,     1);
      boost::filesystem::path path100
        = temp / (std::string("file.100.") + suffixes[i]);
      benchmark(suffixes[i], path100, 100);
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception thrown: " << e.what() << std::endl;
  }
  return 0;
}
