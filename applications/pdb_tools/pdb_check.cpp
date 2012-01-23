/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/atom/pdb.h>
#include <IMP/atom/force_fields.h>
#include <IMP/exception.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input;
po::options_description desc("Usage: input.pdb.\n");
bool help=false;
int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}


int main(int argc, char **argv) {
  try {
    desc.add_options()
    ("help,h",
     "Print messages about any errors encountered in parsing the pdb file.")
        ("input-file,i", po::value< std::string >(&input),
         "input pdb file");
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
    IMP_NEW(IMP::Model, m, ());
    m->set_log_level(IMP::SILENT);
    IMP::set_log_level(IMP::VERBOSE);
    IMP::atom::Hierarchies inhs;
    IMP_CATCH_AND_TERMINATE(inhs= IMP::atom::read_multimodel_pdb(input, m));
    for (unsigned int i=0; i< inhs.size(); ++i) {
      IMP::atom::add_bonds(inhs[i]);
    }
    return 0;
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
