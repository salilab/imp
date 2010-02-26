/**
 *  \file Gnuplot.h   \brief A class for printing gnuplot scripts
 *   for profile viewing
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include "Gnuplot.h"

#include <fstream>

void Gnuplot::print_profile_script(const std::string pdb, bool interactive) {
  static int color = 2;
  // file names
  std::string profile_file_name = pdb + ".dat";
  std::string plt_file_name = trim_extension(pdb) + ".plt";
  std::string png_file_name = trim_extension(pdb) + ".png";
  // output script
  std::ofstream plt_file(plt_file_name.c_str());
  if(!interactive) plt_file << "set terminal png;set output \""
                            << png_file_name << "\";" << std::endl;
  plt_file << "set title \"" << trim_extension(pdb)
           <<"\";set xlabel \"q\";set ylabel \"I(q) log-scale\"\n";
  plt_file << "plot '" << profile_file_name
           << "' u 1:2 thru log(y) t \"theoretical\" w lines lw 2 lt "
           << color << std::endl;
  plt_file.close();
  color++;
}

void Gnuplot::print_profile_script(const std::vector<std::string>& pdbs,
                                   bool interactive) {
  std::ofstream plt_file("profiles.plt");
  if(!interactive)
    plt_file << "set terminal png;set output \"profiles.png\";" << std::endl;
  plt_file
   << "set title \"Profiles\";set xlabel \"q\";set ylabel \"I(q) log-scale\"\n";
  plt_file << "plot ";

  for(unsigned int i=0; i<pdbs.size(); i++) {
    std::string profile_file_name = pdbs[i] + ".dat";
    plt_file << "'" << profile_file_name << "' u 1:2 thru log(y) t \""
             << trim_extension(pdbs[i]) << "\" w lines lw 2 lt " << i+2;
    if(i==pdbs.size()-1) plt_file << std::endl;
    else plt_file << ",";
  }
  plt_file.close();
}

void Gnuplot::print_fit_script(const std::string pdb,
                               const std::string profile_file,
                               bool interactive) {
  static int color = 2;
  // file names
  std::string pdb_name = trim_extension(pdb);
  std::string profile_name =
    trim_extension(basename(const_cast<char *>(profile_file.c_str())));
  std::string plt_file_name = pdb_name + "_" + profile_name + ".plt";
  std::string png_file_name = pdb_name + "_" + profile_name + ".png";
  std::string fit_file_name = pdb_name + "_" + profile_name + ".dat";
  // output script
  std::ofstream plt_file(plt_file_name.c_str());
  plt_file.precision(3);
  if(!interactive) plt_file << "set terminal png;set output \""
                            << png_file_name << "\";" << std::endl;
  plt_file << "set title \"" << pdb_name
           <<"\";set xlabel \"q\";set ylabel \"I(q) log-scale\"\n";
  plt_file <<  "plot '" << fit_file_name
           << "' u 1:2 thru log(y) t \"experimental\", '" << fit_file_name
           << "' u 1:3 thru log(y) t \"theoretical\" w lines lw 2 lt " << color;
  //std::string crysol_file_name = trim_extension(pdb) + "00.fit";
  //        << ", '" << crysol_file_name << "' u 1:3 thru log(y) t \"crysol\""
  plt_file  << std::endl;
  plt_file.close();
  color++;
}

void Gnuplot::print_fit_script(const std::vector<std::string>& pdbs,
                               const std::string profile_file, bool interactive)
{
  std::string profile_name =
    trim_extension(basename(const_cast<char *>(profile_file.c_str())));
  std::ofstream plt_file("fit.plt");
  if(!interactive)
    plt_file << "set terminal png;set output \"fit.png\";" << std::endl;
  plt_file << "set title \"" << profile_name
           << " fit\";set xlabel \"q\";set ylabel \"I(q) log-scale\"\n";
  plt_file << "plot ";

  for(unsigned int i=0; i<pdbs.size(); i++) {
    std::string pdb_name = trim_extension(pdbs[i]);
    std::string fit_file_name = pdb_name + "_" + profile_name + ".dat";
    if(i==0) { // first fit, print experimental data
      plt_file <<  "'" << fit_file_name
               << "' u 1:2 thru log(y) t \"experimental\", ";
    }
    plt_file <<  "'" << fit_file_name << "' u 1:3 thru log(y) t \""
             << pdb_name << "\" w lines lw 2";
    if(i==pdbs.size()-1) plt_file << std::endl;
    else plt_file << ",";
  }
  plt_file.close();
}

std::string trim_extension(const std::string file_name) {
  if(file_name[file_name.size()-4] == '.')
    return file_name.substr(0, file_name.size() - 4);
  return file_name;
}
