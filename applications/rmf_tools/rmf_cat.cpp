/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/rmf/particle_io.h>
#include <IMP/display/declare_Geometry.h>
#include <IMP/display/Writer.h>
#include <IMP/display/restraint_geometry.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/frames.h>

#include "common.h"

std::vector<std::string> inputs;
std::string output;
std::string description("Combine two or more rmf files.");

int main(int argc, char **argv) {
  try {
    positional_options.add_options()
      ("input-files,i",
       boost::program_options::value< std::vector<std::string> >(&inputs),
       "input rmf file");
    positional_names.push_back("input_1.rmf input_2.rmf ... output.rmf");
    positional_options_description.add("input-files", -1);
    process_options(argc, argv);
    if (inputs.size() < 3) {
      print_help_and_exit(argv);
    }

    output= inputs.back();
    inputs.pop_back();
    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(inputs[0]);
    RMF::FileHandle orh= RMF::create_rmf_file(output);
    IMP_NEW(IMP::Model, m, ());
    std::cout << "Loading structure" << std::endl;
    IMP::atom::Hierarchies hs= IMP::rmf::create_hierarchies(rh, m);
    IMP::ParticlesTemp ps= IMP::rmf::create_particles(rh, m);
    IMP::rmf::RMFRestraints rs= IMP::rmf::create_restraints(rh, m);
    IMP::display::Geometries gs= IMP::rmf::create_geometries(rh);

    std::cout << "Adding structure to output file" << std::endl;
    IMP::rmf::add_hierarchies(orh, hs);
    IMP::rmf::add_particles(orh, ps);
    IMP::rmf::add_restraints(orh, rs);
    IMP::rmf::add_geometries(orh, gs);

    int out_frame=0;
    for (unsigned int i=0; i< inputs.size(); ++i) {
      // avoid double open
      if (i != 0) {
        rh= RMF::open_rmf_file_read_only(inputs[i]);
        std::cout << "Linking " << i << std::endl;
        IMP::rmf::link_hierarchies(rh, hs);
        IMP::rmf::link_particles(rh, ps);
        IMP::rmf::link_restraints(rh, rs);
        IMP::rmf::link_geometries(rh, gs);
      }

      for (unsigned int j=0; j < rh.get_number_of_frames(); ++j) {
        IMP::rmf::load_frame(rh, j);
        IMP::rmf::save_frame(orh, out_frame);
        ++out_frame;
      }
    }
    return 0;
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
