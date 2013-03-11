/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <RMF/FileHandle.h>
#include <IMP/rmf/geometry_io.h>
#include <IMP/display/Color.h>
#include <IMP/base/flags.h>
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
}

int main(int argc, char **argv) {
  try {
    boost::int64_t level=0;
    std::string colormap="jet";
    IMP::base::AddIntFlag lf("level",
                 "What level to apply the color to in the RMF (0 is root).",
                             &level);
    IMP::base::AddStringFlag cf("colormap",
                                "The colormap to use: display, jet",
                                &colormap);
    IMP::Strings files
      = IMP::base::setup_from_argv(argc, argv, "Add color to an RMF file",
                                   "file0.rmf ...", -1);
    for (unsigned int i = 0; i < files.size(); ++i) {
      RMF::FileHandle rh= RMF::open_rmf_file(files[i]);
      int count= get_count(rh.get_root_node(), level);
      RMF::Category cat= rh.get_category("shape");
      RMF::FloatKey red
        = rh.get_key<RMF::FloatTraits>(cat, "rgb color red");
      RMF::FloatKey green
        = rh.get_key<RMF::FloatTraits>(cat, "rgb color green");
      RMF::FloatKey blue
        = rh.get_key<RMF::FloatTraits>(cat, "rgb color blue");
      recolor(rh.get_root_node(), level, count, 0, colormap, red, green, blue);
    }
  } catch (const IMP::base::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
