/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
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

namespace {
std::string description("Display an rmf file in pymol.");
double restraint_max=-1;
std::string file_type="auto";

int frame=0;
}

int main(int argc, char **argv) {
  try {
    options.add_options()
      ("recolor,c", "Recolor the hierarchies using the display colors.")
      ("score,s", boost::program_options::value< double >(&restraint_max),
       "The upper bound for the restraints scores to color the "\
       "restraints by score.")
      ("type,T", boost::program_options::value< std::string >(&file_type),
       "The program to display with (one of pymol or chimera or auto).");
    IMP_ADD_INPUT_FILE("rmf");
    IMP_ADD_OUTPUT_FILE("graphics");
    IMP_ADD_FRAMES;
    boost::program_options::variables_map vm(process_options(argc, argv));
    bool exec=false;
    if (output.empty()) {
      exec=true;
      if (file_type=="auto") {
        print_help_and_exit(argv);
        return 1;
      }
      if (file_type=="pymol") {
        output= IMP::create_temporary_file_name("display", ".pym");
      } else if (file_type=="chimera") {
        output= IMP::create_temporary_file_name("display", ".py");
      } else {
        print_help_and_exit(argv);
        return 1;
      }
    }
    std::cout<< "writing to file " << output << std::endl;
    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchies hs= IMP::rmf::create_hierarchies(rh, m);
    IMP::ParticlesTemp ps= IMP::rmf::create_particles(rh, m);
    IMP::Restraints rs= IMP::rmf::create_restraints(rh, m);
    IMP::display::Geometries gs= IMP::rmf::create_geometries(rh);

    IMP::Pointer<IMP::display::Writer> w
      = IMP::display::create_writer(output);
    IMP_FOR_EACH_FRAME(rh.get_number_of_frames()) {
      w->set_frame(frame_iteration);
      IMP::rmf::load_frame(rh, current_frame);
      for (unsigned int i=0; i< hs.size(); ++i) {
        IMP_NEW(IMP::atom::HierarchyGeometry, g, (hs[i]));
        if (vm.count("recolor")) {
          g->set_color(IMP::display::get_display_color(i));
        }
        w->add_geometry(g);
      }
      for (unsigned int i=0; i< ps.size(); ++i) {
        /*if (frame!= 0) {
          IMP::rmf::load_configuration(rh, hs[i], frame);
          }*/
        if (IMP::core::XYZR::particle_is_instance(ps[i])) {
          IMP::core::XYZR d(ps[i]);
          IMP_NEW(IMP::core::XYZRGeometry, g, (ps[i]));
          if (vm.count("recolor")) {
            g->set_color(IMP::display::get_display_color(i));
          }
          w->add_geometry(g);
        }
      }
      w->add_geometry(gs);
      for (unsigned int i=0; i< ps.size(); ++i) {
        IMP_NEW(IMP::display::RestraintGeometry, g, (rs[i]));
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
