/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/display/Color.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

int get_count(RMF::NodeHandle nh, int level) {
  int ret=0;
  if (level> 0) {
    RMF::NodeHandles ch= nh.get_children();
    for (unsigned int i=0; i< ch.size(); ++i) {
      ret+= get_count(ch[i], level-1);
    }
  } else {
    ret+= 1;
  }
  return ret;
}

int recolor(RMF::NodeHandle nh, int level, int total,
            int so_far, std::string method,
            RMF::FloatKey red,
            RMF::FloatKey green,
            RMF::FloatKey blue) {
  if (level> 0) {
    RMF::NodeHandles ch= nh.get_children();
    for (unsigned int i=0; i< ch.size(); ++i) {
      so_far= recolor(ch[i], level-1, total, so_far, method,
                      red, green, blue);
    }
  } else {
    double f= static_cast<double>(so_far)/total;
    IMP::display::Color c;
    if (method =="jet") {
      c= IMP::display::get_jet_color(f);
    } else {
      c= IMP::display::get_display_color(so_far);
    }
    nh.set_value(red, c.get_red());
    nh.set_value(green, c.get_green());
    nh.set_value(blue, c.get_blue());
    ++so_far;
  }
  return so_far;
}

std::string input, method;
po::options_description desc("Usage: input.rmf");
bool help=false;
bool verbose=false;
int level=1;
void print_help() {
  std::cerr << desc << std::endl;
}

int main(int argc, char **argv) {
  try {
    desc.add_options()
      ("help,h", "Print the contents of an rmf file to the terminal.")
      ("verbose,v", "Print lots of information about each node.")
      ("level,l", po::value< int >(&level),
       "What level to apply the color to in the RMF (from the top.")
      ("method,m", po::value< std::string >(&method),
       "how to choose the colors: display, jet")
      ("input-file,i", po::value< std::string >(&input),
       "input hdf5 file");
    po::positional_options_description p;
    p.add("input-file", 1);
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
    RMF::FileHandle rh= RMF::open_rmf_file(input);
    int count= get_count(rh.get_root_node(), level);
    RMF::Category cat= RMF::get_category_always<1>(rh, "shape");
    RMF::FloatKey red
        = RMF::get_key_always<RMF::FloatTraits>(rh, cat, "rgb color red");
    RMF::FloatKey green
        = RMF::get_key_always<RMF::FloatTraits>(rh, cat, "rgb color green");
    RMF::FloatKey blue
        = RMF::get_key_always<RMF::FloatTraits>(rh, cat, "rgb color blue");
    recolor(rh.get_root_node(), level, count, 0, method, red, green, blue);
  } catch (const IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
