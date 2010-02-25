/**
 *  \file Gnuplot.h   \brief A class for printing gnuplot scripts
 *   for profile viewing
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMP_GNUPLOT_H
#define IMP_GNUPLOT_H

#include <string>
#include <vector>

class Gnuplot {
public:
  static void print_profile_script(const std::string pdb,
                                   bool interactive=true);
  static void print_profile_script(const std::vector<std::string>& pdbs,
                                   bool interactive=true);
  static void print_fit_script(const std::string pdb,
                               const std::string profile_file,
                               bool interactive=true);
  static void print_fit_script(const std::vector<std::string>& pdbs,
                               const std::string profile_file,
                               bool interactive=true);
};

std::string trim_extension(const std::string file_name);

#endif /* IMP_GNUPLOT_H */
