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
#include <IMP/rmf/links.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input_hdf5 output_graphics");
double restraint_max=-1;
std::string file_type="auto";

int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}


int main(int argc, char **argv) {
  try {
    desc.add_options()
      ("help,h", "Translate an hdf5 file to graphics.")
      ("recolor,c", "Recolor the hierarchies using the display colors.")
      ("frame,f", po::value< int >(&frame),
       "Frame to use. Do '-#' for every #th frame (eg -1 is every frame).")
      ("score,s", po::value< double >(&restraint_max),
       "The upper bound for the restraints scores to color the "\
       "restraints by score.")
      ("type,T", po::value< std::string >(&file_type),
       "The program to display with (one of pymol or chimera or auto).")
      ("input-file,i", po::value< std::string >(&input),
       "input hdf5 file")
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
    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchies hs= IMP::rmf::create_hierarchies(rh, m);
    IMP::ParticlesTemp ps= IMP::rmf::create_particles(rh, m);
    IMP::rmf::RMFRestraints rs= IMP::rmf::create_restraints(rh, m);
    IMP::display::Geometries gs= IMP::rmf::create_geometries(rh);
    int minframe, maxframe;
    if (frame>=0) {
      minframe=frame;
      maxframe=minframe+1;
    } else {
      minframe=0;
      maxframe= rh.get_number_of_frames()+1;
    }
    int step=1;
    if (frame<0) step=std::abs(frame);
    std::cout << "Reading frames [" << minframe << ", "
              << maxframe << ": " << step << ")" <<std::endl;

    IMP::Pointer<IMP::display::Writer> w
      = IMP::display::create_writer(output);
    for (int cur_frame=minframe; cur_frame < maxframe; cur_frame+=step) {
      if (cur_frame%10==0) {
        std::cout << cur_frame << " ";
      }
      w->set_frame((cur_frame-minframe)/step);
      IMP::rmf::load_frame(rh, cur_frame);
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
