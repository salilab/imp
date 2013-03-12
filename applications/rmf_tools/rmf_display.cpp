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
#include <IMP/base/flags.h>

int main(int argc, char **argv) {
  try {
    bool recolor = false;
    double score = std::numeric_limits<double>::max();
    std::string file_type = "auto";
    boost::int64_t frame = 0;
    boost::int64_t frame_step = 0;
    IMP::base::AddBoolFlag abf("recolor",
                           "Recolor the hierarchies using the display colors",
                               &recolor);
    IMP::base::AddFloatFlag aff("score",
                                "The upper bound for the restraints scores to"\
                                " color the "\
                                "restraints by score.", &score);
    IMP::base::AddStringFlag asf("type",
                                 "pymol, chimera or auto (to use suffix)",
                                 &file_type);
    IMP::base::AddIntFlag aif("frame",
                              "The frame index or a negative number for every"
                              " f frames", &frame);
    IMP::base::AddIntFlag aiff("frame_step",
                              "If non-zero output every n frames", &frame_step);
    IMP::Strings io
      = IMP::base::setup_from_argv(argc, argv,
                                   "Export an RMF file to a viewer",
                                   "input.rmf [output]",
                                   -1);
    std::string output;
    if (io.size() > 1) {
      output = io[1];
    }
    if (frame < 0) {
      IMP::base::write_help();
      return 1;
    }
    bool exec=false;
    if (output.empty()) {
      exec=true;
      if (file_type=="auto") {
        IMP::base::write_help();
        return 1;
      }
      if (file_type=="pymol") {
        output= IMP::create_temporary_file_name("display", ".pym");
      } else if (file_type=="chimera") {
        output= IMP::create_temporary_file_name("display", ".py");
      } else {
        IMP::base::write_help();
        return 1;
      }
    }
    std::cout<< "writing to file " << output << std::endl;
    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(io[0]);
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchies hs= IMP::rmf::create_hierarchies(rh, m);
    IMP::ParticlesTemp ps= IMP::rmf::create_particles(rh, m);
    IMP::Restraints rs= IMP::rmf::create_restraints(rh, m);
    IMP::display::Geometries gs= IMP::rmf::create_geometries(rh);

    IMP::Pointer<IMP::display::Writer> w
      = IMP::display::create_writer(output);
    if (frame_step == 0) frame_step = std::numeric_limits<int>::max();
    int cur_frame=0;
    for (unsigned int frame_iteration = frame;
         frame_iteration < rh.get_number_of_frames();
         frame_iteration += frame_step) {
      w->set_frame(cur_frame++);
      IMP::rmf::load_frame(rh, frame_iteration);
      for (unsigned int i=0; i< hs.size(); ++i) {
        IMP_NEW(IMP::atom::HierarchyGeometry, g, (hs[i]));
        if (recolor) {
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
          if (recolor) {
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
