/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/rmf/particle_io.h>
#include <IMP/display/geometry.h>
#include <IMP/display/particle_geometry.h>
#include <IMP/display/Writer.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/rmf/restraint_io.h>
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

IMP::core::XYZRs get_xyzr_particles(IMP::rmf::NodeHandle nh,
                                    int frame) {
  IMP::ParticlesTemp ps= IMP::rmf::get_restraint_particles(nh, frame);
  IMP::core::XYZRs ret;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (IMP::core::XYZR::particle_is_instance(ps[i])) {
      ret.push_back(IMP::core::XYZR(ps[i]));
    }
  }
  return ret;
}

void set_color(IMP::rmf::NodeHandle nh,
               int frame, IMP::display::Geometry *g) {
  if (restraint_max==-1) {
    return;
  } else {
    double score= IMP::rmf::get_restraint_score(nh, frame);
    if (score <= -std::numeric_limits<double>::max()) return;
    double nscore= score/restraint_max;
    if (nscore<0) nscore=0;
    if (nscore>1) nscore=1;
    g->set_color( IMP::display::get_jet_color(nscore));
  }
}

IMP::display::Geometry *create_restraint_geometry(IMP::rmf::NodeHandle nh,
                                                  int frame) {
  double score=IMP::rmf::get_restraint_score(nh, frame);
  if (score < -std::numeric_limits<double>::max()) return NULL;
  IMP::rmf::NodeHandles children=nh.get_children();
  IMP::display::Geometries gs;
  for (unsigned int i=0; i< children.size(); ++i) {
    IMP::display::Geometry* g
      = create_restraint_geometry(children[i], frame);
    if (g) {
      IMP::Pointer<IMP::display::Geometry> gp(g);
      gs.push_back(g);
    }
  }
  IMP::core::XYZRs ds= get_xyzr_particles(nh, frame);
  if (ds.size()==2) {
    IMP::algebra::Segment3D s(ds[0].get_coordinates(), ds[1].get_coordinates());
    gs.push_back(new IMP::display::SegmentGeometry(s));
    gs.back()->set_name(nh.get_name());
  } else {
    for (unsigned int i=0; i< ds.size(); ++i) {
      gs.push_back(new IMP::display::SphereGeometry(ds[i].get_sphere()));
      gs.back()->set_name(nh.get_name());
    }
  }
  if (gs.empty()) {
    return NULL;
  } else if (gs.size()==1) {
    set_color(nh, frame, gs[0]);
    IMP::Pointer<IMP::display::Geometry>
      ret(static_cast<IMP::display::Geometry*>(gs[0]));
    gs.clear();
    return ret.release();
  } else {
    IMP_NEW(IMP::display::CompoundGeometry, ret, (gs, nh.get_name()));
    set_color(nh, frame, ret);
    return ret.release();
  }
}

void add_restraints(IMP::rmf::RootHandle rh,
                    int frame,
                    IMP::display::Writer *w) {
  IMP::rmf::NodeHandles children = rh.get_children();
  for (unsigned int i=0; i< children.size(); ++i) {
    IMP::display::Geometry* g= create_restraint_geometry(children[i],
                                                         frame);
    if (g) {
      IMP::Pointer<IMP::display::Geometry> gp(g);
      g->set_color(IMP::display::get_display_color(i));
      w->add_geometry(g);
    }
  }
}

int main(int argc, char **argv) {
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
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
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

  IMP::rmf::RootHandle rh= IMP::rmf::open_rmf_file(input);
  IMP_NEW(IMP::Model, m, ());
  IMP::atom::Hierarchies hs= IMP::rmf::create_hierarchies(rh, m);
  IMP::ParticlesTemp ps= IMP::rmf::create_particles(rh, m);
  int minframe, maxframe;
  if (frame>=0) {
    minframe=frame;
    maxframe=minframe+1;
  } else {
    minframe=0;
    IMP::rmf::FloatKey xk
      =rh.get_key<IMP::rmf::FloatTraits>(IMP::rmf::Physics, "cartesian x");
    std::cout << xk << std::endl;
    maxframe= rh.get_number_of_frames(xk)+1;
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
    for (unsigned int i=0; i< hs.size(); ++i) {
      IMP::rmf::load_frame(rh, cur_frame, hs[i]);
      IMP_NEW(IMP::display::HierarchyGeometry, g, (hs[i]));
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
        IMP_NEW(IMP::display::XYZRGeometry, g, (ps[i]));
        if (vm.count("recolor")) {
          g->set_color(IMP::display::get_display_color(i));
        }
        w->add_geometry(g);
      }
    }
    IMP::display::Geometries gs=
      IMP::rmf::create_geometries(rh, cur_frame);
    for (unsigned int i=0; i< gs.size(); ++i) {
      w->add_geometry(gs[i]);
    }
    add_restraints(rh, cur_frame, w);
  }
  if (exec) {
    if (file_type=="pymol") {
      return system((std::string("pymol")+" "+output).c_str());
    } else {
      return system((std::string("chimera")+" "+output).c_str());
    }
  } else {
    return 0;
  }
}
