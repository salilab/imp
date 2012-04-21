/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/rmf/frames.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/force_fields.h>
#include <IMP/exception.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input.[rmf/pdb] output.[rmf/pdb].\n The"\
         " RMF file's contents must be an atomic structure of a molecule.");
bool help=false;
int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}

std::string get_suffix(std::string name) {
  int pos= name.rfind(".");
  if (pos ==-1) {
    std::cerr << "Bad file name: " << name <<std::endl;
    print_help();
    exit(1);
  }
  std::string ret(name, pos);
  if (ret != ".pdb" && ret != ".rmf") {
    std::cerr << "The file names must end in .rmf or .pdb: "
              << name << std::endl;
    print_help();
    exit(1);
  }
  return ret;
}

int main(int argc, char **argv) {
  try {
    desc.add_options()
    ("help,h", "Translate and hdf5 file to pdb.")
    ("frame,f", po::value< int >(&frame),
     "Frame to use")
    ("input-file,i", po::value< std::string >(&input),
     "input hdf5 file")
    ("output-file,o", po::value< std::string >(&output),
     "output pdb file");;
    po::positional_options_description p;
    p.add("input-file", 1);
    p.add("output-file", 1);
    po::variables_map vm;
    po::store(
              po::command_line_parser(argc,
                                      argv).options(desc).positional(p).run(),
              vm);
    po::notify(vm);
    if (vm.count("help") || input.empty() || output.empty()) {
      print_help();
      return 1;
    }
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchy h;
    RMF::FileHandle rh;
    int nframes=-1;
    if (get_suffix(input) == ".pdb") {
      IMP_CATCH_AND_TERMINATE(h= IMP::atom::read_pdb(input, m));
      IMP::atom::add_bonds(h);
      nframes= std::numeric_limits<int>::max();
    } else {
      IMP_CATCH_AND_TERMINATE(rh= RMF::open_rmf_file(input));
      IMP::atom::Hierarchies inhs= IMP::rmf::create_hierarchies(rh, m);
      h= IMP::atom::Hierarchy::setup_particle(new IMP::Particle(m, "root"));
      for (unsigned int i=0; i< inhs.size(); ++i) {
        h.add_child(inhs[i]);
      }
      //std::cout << xk << std::endl;
      nframes= rh.get_number_of_frames();
    }
    int minframe, maxframe, step;
    if (frame>=0) {
      minframe=frame;
      maxframe=frame+1;
      step =1;
    } else {
      minframe=0;
      maxframe=nframes+1;
      step=-frame;
    }
    RMF::FileHandle rho;
    int outframe=0;
    for (int cur_frame=minframe; cur_frame < maxframe; cur_frame+=step) {
      if (outframe%10==0) {
        std::cout << outframe << std::endl;
      }
      if (get_suffix(input) == ".pdb") {
        try {
          IMP::atom::read_pdb(input, cur_frame, h);
        } catch (IMP::base::ValueException) {
          // out of frames;
          break;
        }
      } else {
        IMP::rmf::load_frame(rh, cur_frame);
      }
      if (get_suffix(output) == ".pdb") {
        IMP::base::TextOutput out(output, outframe!=0);
        IMP_CATCH_AND_TERMINATE(IMP::atom::write_pdb(h, out, outframe));
      } else {
        if (outframe==0) {
          rho= RMF::create_rmf_file(output);
          IMP::rmf::add_hierarchies(rho, h.get_children());
        } else {
          IMP::rmf::save_frame(rho, outframe);
        }
      }
      ++outframe;
    }
    return 0;
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
