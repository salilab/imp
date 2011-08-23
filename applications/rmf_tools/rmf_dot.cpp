/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/internal/graph_utility.h>

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
  namespace {
    desc.add_options()
    ("help,h",
     "Print the contents of an rmf file to the terminal as a dot graph.")
    ("verbose,v", "Print lots of information about each node.")
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
    IMP::rmf::RootHandle rh=IMP::rmf::open_rmf_file(input);
    IMP::rmf::NodeTree nt= IMP::rmf::get_node_tree(rh);
    IMP::internal::show_as_graphviz(nt, std::cout);
    return 0;
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
