/**
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */
#include "Data.h"
#include <RMF/log.h>
#include <RMF/exceptions.h>
#include <boost/version.hpp>
#if BOOST_VERSION >= 107300
#include <boost/bind/bind.hpp>
#else
#include <boost/bind.hpp>
#endif
#include <boost/bind/placeholders.hpp>
#include <boost/lambda/construct.hpp>

#if BOOST_VERSION >= 107300
using namespace boost::placeholders;
#endif

namespace {
molfile_plugin_t plugin;
molfile_plugin_t plugin3;
molfile_plugin_t pluginz;

template <class M>
int catch_exceptions(std::string name, void *mydata, M m) {
  RMF_UNUSED(name);
  RMF_TRACE(name);
  try {
    RMF_vmd::Data *data = reinterpret_cast<RMF_vmd::Data *>(mydata);
    return m(data);
  }
  catch (std::exception &e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return VMDPLUGIN_ERROR;
  }
}

void close_rmf_read(void *mydata) {
  RMF_vmd::Data *data = reinterpret_cast<RMF_vmd::Data *>(mydata);
  delete data;
}

void *open_rmf_read(const char *filename, const char *, int *natoms) {
  RMF_TRACE("open rmf file");
  try {
    return new RMF_vmd::Data(filename, natoms);
  }
  catch (std::exception &e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    *natoms = MOLFILE_NUMATOMS_NONE;
    return NULL;
  }
}

int read_rmf_structure(void *mydata, int *optflags, molfile_atom_t *atoms) {
  return catch_exceptions(
      "read rmf structure", mydata,
      boost::bind(&RMF_vmd::Data::read_structure, _1, optflags, atoms));
}

int read_rmf_timestep(void *mydata, int, molfile_timestep_t *frame) {
  return catch_exceptions(
      "read rmf timestep", mydata,
      boost::bind(&RMF_vmd::Data::read_timestep, _1, frame));
}

int read_rmf_bonds(void *mydata, int *nbonds, int **fromptr, int **toptr,
                   float **bondorderptr, int **bondtype, int *nbondtypes,
                   char ***bondtypename) {
  return catch_exceptions(
      "read rmf bonds", mydata,
      boost::bind(&RMF_vmd::Data::read_bonds, _1, nbonds, fromptr, toptr,
                  bondorderptr, bondtype, nbondtypes, bondtypename));
}

int read_rmf_graphics(void *mydata, int *nelem,
                      const molfile_graphics_t **gdata) {
  return catch_exceptions(
      "read rmf graphics", mydata,
      boost::bind(&RMF_vmd::Data::read_graphics, _1, nelem, gdata));
}

int read_rmf_timestep_metadata(void *mydata,
                               molfile_timestep_metadata_t *tdata) {
  return catch_exceptions(
      "read rmf timestep metadata", mydata,
      boost::bind(&RMF_vmd::Data::read_timestep_metadata, _1, tdata));
}

void init_plugin(molfile_plugin_t &plugin) {
  memset(&plugin, 0, sizeof(molfile_plugin_t));
  plugin.abiversion = vmdplugin_ABIVERSION;
  plugin.type = MOLFILE_PLUGIN_TYPE;
  plugin.name = "rmf";
  plugin.prettyname = "RMF";
  plugin.author = "Daniel Russel";
  plugin.majorv = 0;
  plugin.minorv = 9;
  plugin.is_reentrant = VMDPLUGIN_THREADSAFE;
  plugin.open_file_read = open_rmf_read;
  plugin.read_structure = read_rmf_structure;
  plugin.read_bonds = read_rmf_bonds;
  plugin.read_rawgraphics = read_rmf_graphics;
  plugin.close_file_read = close_rmf_read;
  plugin.read_timestep_metadata = read_rmf_timestep_metadata;
  plugin.read_next_timestep = read_rmf_timestep;
}
}

VMDPLUGIN_API int VMDPLUGIN_init() {
  init_plugin(plugin);
  plugin.name = "rmf";
  plugin.prettyname = "RMF";
  plugin.filename_extension = "rmf";

  init_plugin(plugin3);
  plugin3.name = "rmf3";
  plugin3.prettyname = "RMF3";
  plugin3.filename_extension = "rmf3";

  init_plugin(pluginz);
  pluginz.name = "rmfz";
  pluginz.prettyname = "RMFz";
  pluginz.filename_extension = "rmfz";

  RMF::set_log_level("off");

  return VMDPLUGIN_SUCCESS;
}

VMDPLUGIN_API int VMDPLUGIN_register(void *v, vmdplugin_register_cb cb) {
  std::cout << "Register" << std::endl;
  (*cb)(v, (vmdplugin_t *)&plugin);
  (*cb)(v, (vmdplugin_t *)&plugin3);
  (*cb)(v, (vmdplugin_t *)&pluginz);
  return 0;
}

VMDPLUGIN_API int VMDPLUGIN_fini() { return VMDPLUGIN_SUCCESS; }
