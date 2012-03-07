/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/rmf/geometry_io.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input.rmf");
bool help=false;
bool verbose=false;
int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}

int main(int argc, char **argv) {
  try {
    desc.add_options()
      ("help,h", "Print the contents of an rmf file to the terminal.")
      ("verbose,v", "Print lots of information about each node.")
      ("decorators,d", "Show what decorators recognize each node.")
      ("frame,f", po::value< int >(&frame),
       "Frame to use")
      ("input-file,i", po::value< std::string >(&input),
       "input hdf5 file");
    po::positional_options_description p;
    p.add("input-file", 1);
    po::variables_map vm;
    po::store(
              po::command_line_parser(argc,
                                      argv).options(desc).positional(p).run(),
              vm);
    po::notify(vm);
    if (vm.count("help") || input.empty()) {
      print_help();
      return 1;
    }
    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    std::string descr= rh.get_description();
    if (!descr.empty()) {
      std::cout << descr << std::endl;
    }
    if (vm.count("decorators")) {
      RMF::show_hierarchy_with_decorators(rh.get_root_node(),
                                          vm.count("verbose"),
                                          frame, std::cout);
    } else {
      RMF::show_hierarchy(rh.get_root_node(), vm.count("verbose"),
                          frame, std::cout);
    }
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
