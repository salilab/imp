/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/hdf5/atom_io.h>
#include <IMP/hdf5/RootHandle.h>
#include <IMP/hdf5/particle_io.h>
#include <IMP/display/geometry.h>
#include <IMP/display/particle_geometry.h>
#include <IMP/display/Writer.h>
#include <IMP/hdf5/geometry_io.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input_hdf5 output_graphics");
bool help=false;
bool recolor=false;
int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}

int main(int argc, char **argv) {
  desc.add_options()
    ("help,h", "Translate an hdf5 file to graphics.")
    ("recolor,c", "Recolor the hierarchies using the display colors.")
    ("frame,f", po::value< int >(&frame),
     "Frame to use. Do '-1' for all frames, if that is done the output "\
     "file name must contain %1%.")
    ("input-file,i", po::value< std::string >(&input),
     "input hdf5 file")
    ("output-file,o", po::value< std::string >(&output),
     "output graphics file");
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
  IMP::hdf5::RootHandle rh(input, false);
  IMP_NEW(IMP::Model, m, ());
  IMP::atom::Hierarchies hs= IMP::hdf5::read_all_hierarchies(rh, m);
  IMP::ParticlesTemp ps= IMP::hdf5::read_all_particles(rh, m);
  int minframe, maxframe;
  if (frame>=0) {
    minframe=frame;
    maxframe=minframe+1;
  } else {
    minframe=0;
    IMP::hdf5::FloatKey xk
      =rh.get_key<IMP::hdf5::FloatTraits>(IMP::hdf5::Physics, "cartesian x");
    std::cout << xk << std::endl;
    maxframe= rh.get_number_of_frames(xk)+1;
  }
  std::cout << "Reading frames [" << minframe << ", "
            << maxframe << ")" <<std::endl;
  for (int cur_frame=minframe; cur_frame < maxframe; ++cur_frame) {
    std::string name=output;
    bool append=false;
    if (frame<0 && name.find("%1%")==std::string::npos) {
      append=cur_frame >minframe;
    } else if (frame<0) {
      std::ostringstream oss;
      oss << boost::format(output)%cur_frame;
      name=oss.str();
    }
    IMP::Pointer<IMP::display::Writer> w
      = IMP::display::create_writer(name, append);
    for (unsigned int i=0; i< hs.size(); ++i) {
      IMP::hdf5::load_configuration(rh, hs[i], cur_frame);
      IMP_NEW(IMP::display::HierarchyGeometry, g, (hs[i]));
      if (vm.count("recolor")) {
        g->set_color(IMP::display::get_display_color(i));
      }
      w->add_geometry(g);
    }
    for (unsigned int i=0; i< ps.size(); ++i) {
      /*if (frame!= 0) {
        IMP::hdf5::load_configuration(rh, hs[i], frame);
        }*/
      if (IMP::core::XYZR::particle_is_instance(ps[i])) {
        IMP::core::XYZR d(ps[i]);
        IMP_NEW(IMP::display::XYZRGeometry, g, (ps[i]));
        if (vm.count("recolor")) {
          g->set_color(IMP::display::get_display_color(i));
        }
        w->add_geometry(g);
      }
    }
    IMP::display::Geometries gs=
      IMP::hdf5::read_all_geometries(rh, cur_frame);
    for (unsigned int i=0; i< gs.size(); ++i) {
      w->add_geometry(gs[i]);
    }
  }
  return 0;
}
