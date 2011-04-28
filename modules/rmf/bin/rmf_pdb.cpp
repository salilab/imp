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
po::options_description desc("Usage: input.[rmf/pdb] output.[rmf/pdb].\n The"\
" RMF file's contents must be an atomic structure of a molecule.");
bool help=false;
int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}

std::string get_suffix(std::string name) {
  int pos= name.rfind(".");
  if (pos ==-1) {
    std::cerr << "Bad file name: " << name <<std::endl;
    print_help();
    exit(1);
  }
  std::string ret(name, pos);
  if (ret != ".pdb" && ret != ".rmf") {
    std::cerr << "The file names must end in .rmf or .pdb: "
              << name << std::endl;
    print_help();
    exit(1);
  }
  return ret;
}

int main(int argc, char **argv) {
  desc.add_options()
    ("help,h", "Translate and hdf5 file to pdb.")
    ("frame,f", po::value< int >(&frame),
     "Frame to use")
    ("input-file,i", po::value< std::string >(&input),
     "input hdf5 file")
    ("output-file,o", po::value< std::string >(&output),
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
  IMP_NEW(IMP::Model, m, ());
  IMP::atom::Hierarchies inhs;
  if (get_suffix(input) == ".pdb") {
    if (frame != 0) {
      std::cerr << "Multiframe pdbs not supported yet." << std::endl;
      print_help();
      exit(1);
    }
    IMP_CATCH_AND_TERMINATE(inhs= IMP::atom::Hierarchies(1,
                                          IMP::atom::read_pdb(input, m)));
  } else {
    IMP::rmf::RootHandle rh;
    IMP_CATCH_AND_TERMINATE(rh= IMP::rmf::RootHandle(input, false));
    inhs= IMP::rmf::create_hierarchies(rh, m);
    for (unsigned int i=0; i< inhs.size(); ++i) {
      IMP_CATCH_AND_TERMINATE(IMP::rmf::load_frame(rh, frame, inhs[i]));
    }
  }
  if (get_suffix(output) == ".pdb") {
    IMP_CATCH_AND_TERMINATE(IMP::atom::write_pdb(inhs, output));
  } else {
    IMP::rmf::RootHandle rh;
    IMP_CATCH_AND_TERMINATE(rh=IMP::rmf::RootHandle(output, true));
    for (unsigned int i=0; i< inhs.size(); ++i) {
      IMP::rmf::add_hierarchy(rh, inhs[i]);
    }
  }
  return 0;
}
