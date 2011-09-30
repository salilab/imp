/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/RootHandle.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/atom/pdb.h>
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
    IMP::atom::Hierarchies inhs;
    RMF::RootHandle rh;
    int nframes=-1;
    if (get_suffix(input) == ".pdb") {
      IMP_CATCH_AND_TERMINATE(inhs= IMP::atom::read_multimodel_pdb(input, m));
      nframes=inhs.size();
    } else {
      IMP_CATCH_AND_TERMINATE(rh= RMF::open_rmf_file(input));
      inhs= IMP::rmf::create_hierarchies(rh, m);
      RMF::FloatKey xk
        =rh.get_key<RMF::FloatTraits>(RMF::Physics, "cartesian x");
      std::cout << xk << std::endl;
      nframes= rh.get_number_of_frames(xk)+1;
    }
    int minframe, maxframe, step;
    if (frame>=0) {
      minframe=frame;
      maxframe=frame+1;
      step =1;
    } else {
      minframe=0;
      maxframe=nframes;
      step=-frame;
    }
    RMF::RootHandle rho;
    int outframe=0;
    for (int cur_frame=minframe; cur_frame < maxframe; cur_frame+=step) {
      if (outframe%10==0) {
        std::cout << outframe << " ";
      }
      IMP::atom::Hierarchies cur;
      if (get_suffix(input) == ".pdb") {
        cur= IMP::atom::Hierarchies(1, inhs[cur_frame]);
      } else {
        for (unsigned int i=0; i< inhs.size(); ++i) {
          IMP::rmf::load_frame(rh, cur_frame, inhs[i]);
        }
        cur= inhs;
      }
      if (get_suffix(output) == ".pdb") {
        IMP::base::TextOutput out(output, outframe!=0);
        IMP_CATCH_AND_TERMINATE(IMP::atom::write_pdb(inhs, out, outframe));
      } else {
        if (outframe==0) {
          rho= RMF::create_rmf_file(output);
          for (unsigned int i=0; i< cur.size(); ++i) {
            IMP::rmf::add_hierarchy(rh, cur[i]);
          }
        } else {
          IMP::rmf::set_hierarchies(rh, cur);
          for (unsigned int i=0; i< cur.size(); ++i) {
            IMP::rmf::save_frame(rh, outframe, cur[i]);
          }
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
