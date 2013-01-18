/**
 * \file JmolWriter.h \brief outputs javascript for jmol display
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_JMOL_WRITER_H
#define IMP_JMOL_WRITER_H

#include <IMP/saxs/FitParameters.h>

#include <IMP/Particle.h>

#include <string>
#include <vector>

class JmolWriter {
public:
  static void prepare_jmol_script(
                             const std::vector<IMP::saxs::FitParameters>& fps,
                             const std::vector<IMP::Particles>& particles_vec,
                             const std::string filename);
  static void prepare_jmol_script(const std::vector<std::string>& pdbs,
                               const std::vector<IMP::Particles>& particles_vec,
                               const std::string filename);
private:
  static void prepare_PDB_file(const std::vector<IMP::saxs::FitParameters>& fps,
                               const std::vector<IMP::Particles>& particles_vec,
                               const std::string filename);

  static void prepare_PDB_file(const std::vector<IMP::Particles>& particles_vec,
                               const std::string filename);

  static std::string jmol_script(std::string jmol_path);
  static std::string prepare_coloring_string(unsigned int model_num);
  static std::string prepare_gnuplot_init_selection_string(
                                             unsigned int model_num, bool exp);
  static std::string model_checkbox(unsigned int model_num, bool is_checked,
                                    bool exp);

  static std::string show_all_checkbox(unsigned int model_num, bool exp);
  static std::string group_checkbox(unsigned int model_num);

private:
  static std::string display_selection_;

public:
  static unsigned int MAX_DISPLAY_NUM_;
};

#endif /* IMP_JMOL_WRITER_H */
