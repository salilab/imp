/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/frames.h>
#include <IMP/algebra/SphereD.h>
#include <IMP/algebra/vector_generators.h>
#include <RMF/FileHandle.h>
#include <IMP/atom/pdb.h>
#include <IMP/base/file.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/command_line_macros.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <sys/stat.h>


void test_it(std::string file_name, std::string test_name,
             IMP::atom::Hierarchy h) {
  {
    IMP::ParticlesTemp leaves= IMP::atom::get_leaves(h);
    RMF::FileHandle fh= RMF::create_rmf_file(file_name);
    IMP::algebra::Sphere3D us= IMP::algebra::get_unit_sphere_d<3>();
    IMP::rmf::add_hierarchies(fh, IMP::atom::Hierarchies(1, h));
    for (unsigned int i=0; i< 1000; ++i) {
      IMP::rmf::save_frame(fh, i);
      for (unsigned int j=0; j< leaves.size(); ++j) {
        IMP::core::XYZR d(leaves[j]);
        d.set_coordinates(d.get_coordinates()+
                          IMP::algebra::get_random_vector_in(us));
      }
    }
  }
  // get size
  struct stat data;
  stat(file_name.c_str(), &data);
  IMP::benchmark::report("size", test_name, data.st_size, 0);
}

int main(int argc, char *argv[]) {
  IMP_BENCHMARK( );
  IMP_NEW(IMP::Model, m, ());
  IMP::atom::Hierarchy h
    = IMP::atom::read_pdb(IMP::rmf::get_data_path("huge.pdb"), m);
  test_it(IMP::base::create_temporary_file_name("benchmark_size", ".rmf"),
          "rmf", h);
#ifdef RMF_USE_PROTOBUF
  test_it(IMP::base::create_temporary_file_name("benchmark_size", ".prmf"),
          "prmf", h);
#endif
  return 0;
}
