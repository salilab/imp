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
#include <boost/format.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input.rmf output.rmf");
double restraint_max=-1;
std::string file_type="auto";

int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}


int main(int argc, char **argv) {
  try {
    desc.add_options()
      ("help,h", "Extract frames from a rmf file.")
      ("frame,f", po::value< int >(&frame),
       "Frame to use. Do '-#' for every #th frame (eg -1 is every frame).")
      ("input-file,i", po::value< std::string >(&input),
       "input rmf file")
      ("output-file,o", po::value< std::string >(&output),
       "output rmf file");
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
    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    RMF::FileHandle orh= RMF::create_rmf_file(output);
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchies hs= IMP::rmf::create_hierarchies(rh, m);
    IMP::ParticlesTemp ps= IMP::rmf::create_particles(rh, m);
    IMP::rmf::RMFRestraints rs= IMP::rmf::create_restraints(rh, m);
    IMP::display::Geometries gs= IMP::rmf::create_geometries(rh);

    IMP::rmf::add_hierarchies(orh, hs);
    IMP::rmf::add_particles(orh, ps);
    IMP::rmf::add_restraints(orh, rs);
    IMP::rmf::add_geometries(orh, gs);

    int minframe, maxframe, step;
    if (frame>=0) {
      minframe=frame;
      maxframe=minframe+1;
      step=1;
    } else {
      minframe=0;
      maxframe= rh.get_number_of_frames()+1;
      step=-frame;
    }
    int out_frame=0;
    for (int cur_frame=minframe; cur_frame < maxframe; cur_frame+=step) {
      if (cur_frame%10==0) {
        std::cout << cur_frame << " ";
      }
      IMP::rmf::load_frame(rh, cur_frame);
      IMP::rmf::save_frame(orh, out_frame);
      ++out_frame;
    }
    return 0;
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
