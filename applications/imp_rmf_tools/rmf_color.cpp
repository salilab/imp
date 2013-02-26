/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/display/Color.h>
#include "common.h"

namespace {
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

std::string method;
std::string description("Recolor the molecules in an rmf file.");
int level=1;
}

int main(int argc, char **argv) {
  try {
    options.add_options()
      ("verbose,v", "Print lots of information about each node.")
      ("level,l", boost::program_options::value< int >(&level),
       "What level to apply the color to in the RMF (from the top.")
      ("method,m", boost::program_options::value< std::string >(&method),
       "how to choose the colors: display, jet");
    IMP_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);
    RMF::FileHandle rh= RMF::open_rmf_file(input);
    int count= get_count(rh.get_root_node(), level);
    RMF::Category cat= rh.get_category("shape");
    RMF::FloatKey red
        = rh.get_key<RMF::FloatTraits>(cat, "rgb color red");
    RMF::FloatKey green
        = rh.get_key<RMF::FloatTraits>(cat, "rgb color green");
    RMF::FloatKey blue
        = rh.get_key<RMF::FloatTraits>(cat, "rgb color blue");
    recolor(rh.get_root_node(), level, count, 0, method, red, green, blue);
  } catch (const IMP::base::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
