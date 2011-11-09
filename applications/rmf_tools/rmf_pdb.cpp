/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/RootHandle.h>
#include <IMP/rmf/geometry_io.h>
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
    IMP::atom::Hierarchies inhs;
    RMF::RootHandle rh;
    int nframes=-1;
    if (get_suffix(input) == ".pdb") {
      IMP_CATCH_AND_TERMINATE(inhs= IMP::atom::read_multimodel_pdb(input, m));
      nframes=inhs.size();
      for (unsigned int i=0; i< inhs.size(); ++i) {
        IMP::atom::add_bonds(inhs[i]);
      }
    } else {
      IMP_CATCH_AND_TERMINATE(rh= RMF::open_rmf_file(input));
      inhs= IMP::rmf::create_hierarchies(rh, m);
      RMF::FloatKey xk
          =rh.get_key<RMF::FloatTraits, 1>(RMF::Physics, "cartesian x");
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
            IMP::rmf::add_hierarchy(rho, cur[i]);
          }
        } else {
          IMP::rmf::set_hierarchies(rh, cur);
          for (unsigned int i=0; i< cur.size(); ++i) {
            IMP::rmf::save_frame(rho, outframe, cur[i]);
          }
        }
      }
      ++outframe;
    }


    IMP::atom::Bonds bds;
    for (unsigned int i=0; i< inhs.size(); ++i) {
      IMP::atom::Bonds cur=IMP::atom::get_internal_bonds(inhs[i]);
      bds.insert(bds.end(), cur.begin(), cur.end());
    }
    std::cout << bds.size() << " bonds" << std::endl;
    RMF::PairIndexKey bk;
    if (rho.get_has_key<RMF::IndexTraits, 2>(RMF::bond, "type")) {
      bk= rho.get_key<RMF::IndexTraits, 2>(RMF::bond, "type");
    } else {
      bk= rho.add_key<RMF::IndexTraits, 2>(RMF::bond, "type", false);
    }
    for (unsigned int i=0; i< bds.size(); ++i) {
      IMP::Particle *p0= bds[i].get_bonded(0).get_particle();
      IMP::Particle *p1= bds[i].get_bonded(1).get_particle();
      RMF::NodeHandle n0= rho.get_node_handle_from_association(p0);
      RMF::NodeHandle n1= rho.get_node_handle_from_association(p1);
      RMF::NodeHandles nhs(2); nhs[0]=n0; nhs[1]=n1;
      RMF::NodePairHandle obd= rho.add_node_tuple<2>(nhs, RMF::BOND);
      obd.set_value(bk, 0);
    }
    return 0;
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
