/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/frames.h>
#include <IMP/atom/pdb.h>
#include <RMF/FileHandle.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/benchmark/utility.h>
#include <IMP/base/flags.h>

namespace {
void test_it(std::string file_name, std::string test_name,
             IMP::atom::Hierarchy h) {
  RMF::FileHandle fh= RMF::create_rmf_file(file_name);
  IMP::rmf::add_hierarchies(fh, IMP::atom::Hierarchies(1, h));
  IMP::rmf::save_frame(fh, 0);
  IMP::rmf::save_frame(fh, 1);
  double time;
  int nframes=0;
  IMP_TIME({IMP::rmf::load_frame(fh, nframes%2); ++nframes;}, time);
  IMP::benchmark::report("rmf load", test_name, time, 1);
}
}

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv, "benchmark loading rmf files");
  try {
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchy h
        = IMP::atom::read_pdb(IMP::rmf::get_data_path("huge.pdb"), m);
    test_it(IMP::base::create_temporary_file_name("benchmark_load", ".rmf"),
            "rmf", h);
#ifdef RMF_USE_PROTOBUF
    test_it(IMP::base::create_temporary_file_name("benchmark_load", ".prmf"),
            "prmf", h);
#endif
  } catch (const IMP::base::Exception &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
