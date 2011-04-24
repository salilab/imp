/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/atom/pdb.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input_hdf5 output.pdb");
bool help=false;
int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}

int main(int argc, char **argv) {
  desc.add_options()
    ("help,h", "Translate and hdf5 file to pdb.")
    ("frame,f", po::value< int >(&frame),
     "Frame to use")
    ("input-file,i", po::value< std::string >(&input),
     "input hdf5 file")
    ("output-file,o", po::value< std::string >(&input),
     "output pdb file");;
  po::positional_options_description p;
  p.add("input-file", 1);
  p.add("output-file", 1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);
  if (vm.count("help") || input.empty() || output.empty()) {
    print_help();
    return 1;
  }
  IMP::rmf::RootHandle rh(input, false);
  IMP_NEW(IMP::Model, m, ());
  IMP::atom::Hierarchies hs=IMP::rmf::create_hierarchies(rh, m);
  for (unsigned int i=0; i< hs.size(); ++i) {
    IMP::rmf::load_frame(rh, frame, hs[i]);
  }
  IMP::TextOutput to(output);
  IMP::atom::write_pdb(hs, to);
  return 0;
}
