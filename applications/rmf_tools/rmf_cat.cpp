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

std::vector<std::string> inputs;
std::string output;
po::options_description desc("Usage: input0.rmf input1.rmf ... output.rmf");
double restraint_max=-1;
std::string file_type="auto";

int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}


int main(int argc, char **argv) {
  try {
    desc.add_options()
      ("help,h", "Concatenate rmf files.")
      ("input-files,i", po::value< std::vector<std::string> >(&inputs),
       "input rmf file")
      ("output-file,o", po::value< std::string >(&output),
       "output rmf file");
    po::positional_options_description p;
    p.add("input-files", -1);
    //p.add("output-file", 1);
    po::variables_map vm;
    po::store(
              po::command_line_parser(argc,
                                      argv).options(desc).positional(p).run(),
              vm);
    po::notify(vm);
    if (vm.count("help") || inputs.size() < 3) {
      print_help();
      return 1;
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
