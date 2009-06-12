/**
 * \file force_fields \brief
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/force_fields.h>
#include <IMP/atom/Charmm.h>
#include <IMP/directories.h>

IMPATOM_BEGIN_NAMESPACE

void add_bonds(Hierarchy d, std::string topology_file_name)
{
  // we want the file to be read only once
  std::string file_name = IMP::get_data_directory() +"/atom/top.lib";
  static Charmm charmm(file_name);

  if(!topology_file_name.empty()) {
    Charmm user_charmm(topology_file_name);
    user_charmm.add_bonds(d);
  } else {
    charmm.add_bonds(d);
  }
}

void add_radius(Hierarchy d, std::string par_file_name,
                std::string top_file_name)
{
  std::string def_top_file_name = IMP::get_data_directory() +"/atom/top.lib";
  std::string def_par_file_name = IMP::get_data_directory() +"/atom/par.lib";

  static Charmm charmm(def_top_file_name, def_par_file_name);

  if(!top_file_name.empty() && !par_file_name.empty()) {
    Charmm user_charmm(top_file_name, par_file_name);
    user_charmm.add_radius(d);
  } else {
    charmm.add_radius(d);
  }
}

IMPATOM_END_NAMESPACE
