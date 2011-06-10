/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/rmf/geometry_io.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input_hdf5");
bool help=false;
bool verbose=false;
int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}

int main(int argc, char **argv) {
  desc.add_options()
    ("help,h", "Translate an hdf5 file to graphics.")
    ("verbose,v", "Print lots of information about each node.")
    ("frame,f", po::value< int >(&frame),
     "Frame to use")
    ("input-file,i", po::value< std::string >(&input),
     "input hdf5 file");
  po::positional_options_description p;
  p.add("input-file", 1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);
  if (vm.count("help") || input.empty()) {
    print_help();
    return 1;
  }
  IMP::rmf::RootHandle rh(input, false);
  IMP::rmf::show_hierarchy(rh, std::cout, vm.count("verbose"), frame);
  return 0;
}
