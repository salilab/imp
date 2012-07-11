/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/frames.h>
#include <RMF/FileHandle.h>
#include <IMP/base/file.h>
#include <RMF/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/benchmark/command_line_macros.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <sys/stat.h>


void test_it(RMF::FileConstHandle input, std::string file_name,
             std::string test_name, int nf) {
  double dt=0;
  IMP_TIME_N({
               RMF::FileHandle fh= RMF::create_rmf_file(file_name);
               RMF::copy_structure(input, fh);
               //RMF::copy_values(input, fh);
               for ( int i=0; i< nf; ++i) {
                 RMF::copy_frame(input, fh, i, i);
               }
    }, dt, 1);
  // get size
  struct stat data;
  stat(file_name.c_str(), &data);
  IMP::benchmark::report("size", test_name, data.st_size, dt);
}

int main(int argc, char *argv[]) {
  IMP_BENCHMARK( );
  // ick
  try {
    std::string name= IMP::benchmark::get_data_path("trajectory.rmf");
    {
      RMF::FileConstHandle fh= RMF::open_rmf_file_read_only(name);
      test_it(fh, IMP::base::create_temporary_file_name("benchmark_size",
                                                        ".rmf"),
              "rmf", 200);
    }
    {
      RMF::FileConstHandle fh= RMF::open_rmf_file_read_only(name);
      test_it(fh, IMP::base::create_temporary_file_name("benchmark_size",
                                                        ".rmf"),
              "single rmf", 1);
    }
#ifdef RMF_USE_PROTOBUF
    {
      RMF::FileConstHandle fh= RMF::open_rmf_file_read_only(name);
      test_it(fh, IMP::base::create_temporary_file_name("benchmark_size",
                                                        ".prmf"),
              "prmf", 200);
    }
#endif
  } catch (const RMF::Exception& e) {
    std::cerr << "Exception found " << e.what() << std::endl;
    return 1;
  } catch (const IMP::base::IOException &e) {
    std::cerr << "Exception found " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
