/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/display/geometry.h>
#include <IMP/atom/pdb.h>
#include <IMP/display/particle_geometry.h>
#include <IMP/display/Writer.h>
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/core/XYZR.h>
#include <IMP/atom/force_fields.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input.pdb output_graphics");
std::string file_type="auto";

int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}


int main(int argc, char **argv) {
  try {
    desc.add_options()
      ("help,h", "Translate an hdf5 file to graphics.")
      ("type,T", po::value< std::string >(&file_type),
       "The program to display with (one of pymol or chimera or auto).")
      ("input-file,i", po::value< std::string >(&input),
       "input pdb file")
      ("output-file,o", po::value< std::string >(&output),
       "output graphics file");
    po::positional_options_description p;
    p.add("input-file", 1);
    p.add("output-file", 1);
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
    bool exec=false;
    if (output.empty()) {
      exec=true;
      if (file_type=="auto") {
        print_help();
        return 1;
      }
      if (file_type=="pymol") {
        output= IMP::create_temporary_file_name("display", ".pym");
      } else if (file_type=="chimera") {
        output= IMP::create_temporary_file_name("display", ".py");
      } else {
        print_help();
        return 1;
      }
    }
    std::cout<< "writing to file " << output << std::endl;
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchies hs= IMP::atom::read_multimodel_pdb(input, m);
    IMP::Pointer<IMP::display::Writer> w
      = IMP::display::create_writer(output);
    for (unsigned int i=0; i< hs.size(); ++i) {
      IMP::atom::add_bonds(hs[i]);
      w->set_frame(i);
      IMP_NEW(IMP::atom::HierarchyGeometry, g, (hs[i]));
      w->add_geometry(g);
      IMP::atom::Bonds bonds= IMP::atom::get_internal_bonds(hs[i]);
      for (unsigned int j=0; j< bonds.size(); ++j) {
        IMP_NEW(IMP::core::EdgePairGeometry, g,
                (IMP::ParticlePair(bonds[j].get_bonded(0),
                              bonds[j].get_bonded(1))));
        g->set_name("bonds");
        w->add_geometry(g);
      }
    }
    if (exec) {
      if (file_type=="pymol") {
        std::cout << "launching pymol..." << std::endl;
        return system((std::string("pymol")+" "+output).c_str());
      } else {
        std::cout << "launching chimera..." << std::endl;
        return system((std::string("chimera")+" "+output).c_str());
      }
    } else {
      return 0;
    }
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
