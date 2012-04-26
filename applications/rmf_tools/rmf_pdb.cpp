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

#include "common.h"

std::string description("Convert between rmf files and pdbs.");
int frame=0;

std::string get_suffix(char *argv[], std::string name) {
  int pos= name.rfind(".");
  if (pos ==-1) {
    std::cerr << "Bad file name: " << name <<std::endl;
    print_help_and_exit(argv);
  }
  std::string ret(name, pos);
  if (ret != ".pdb" && ret != ".rmf") {
    std::cerr << "The file names must end in .rmf or .pdb: "
              << name << std::endl;
    print_help_and_exit(argv);
  }
  return ret;
}

int main(int argc, char **argv) {
  try {
    IMP_ADD_INPUT_FILE("rmf or pdb");
    IMP_ADD_OUTPUT_FILE("rmf or pdb");
    IMP_ADD_FRAMES;
    process_options(argc, argv);
    IMP_NEW(IMP::Model, m, ());
    IMP::atom::Hierarchy h;
    RMF::FileHandle rh;
    int nframes=-1;
    if (get_suffix(argv, input) == ".pdb") {
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
    RMF::FileHandle rho;
    IMP_FOR_EACH_FRAME(nframes) {
      if (get_suffix(argv,input) == ".pdb") {
        try {
          IMP::atom::read_pdb(input, current_frame, h);
        } catch (IMP::base::ValueException) {
          // out of frames;
          break;
        }
      } else {
        IMP::rmf::load_frame(rh, current_frame);
      }
      if (get_suffix(argv,output) == ".pdb") {
        IMP::base::TextOutput out(output, frame_iteration!=0);
        IMP_CATCH_AND_TERMINATE(IMP::atom::write_pdb(h, out, frame_iteration));
      } else {
        if (frame_iteration==0) {
          rho= RMF::create_rmf_file(output);
          IMP::rmf::add_hierarchies(rho, h.get_children());
        } else {
          IMP::rmf::save_frame(rho, frame_iteration);
        }
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
