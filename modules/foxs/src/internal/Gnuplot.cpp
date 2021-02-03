/**
 *  \file IMP/foxs/Gnuplot.h   \brief A class for printing gnuplot scripts
 *   for profile viewing
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/foxs/internal/Gnuplot.h>
#include <IMP/foxs/internal/ColorCoder.h>
#include <IMP/saxs/utility.h>

#include <fstream>

IMPFOXS_BEGIN_INTERNAL_NAMESPACE

void Gnuplot::print_profile_script(const std::string pdb) {
  // file names
  std::string profile_file_name = pdb + ".dat";
  std::string plt_file_name = saxs::trim_extension(pdb) + ".plt";
  std::string png_file_name = saxs::trim_extension(pdb) + ".png";
  // output script
  std::ofstream plt_file(plt_file_name.c_str());
  plt_file << "set terminal png enhanced;set output \"" << png_file_name
           << "\";" << std::endl;
  plt_file << "set ylabel '';set format y '';set xtics nomirror;"
           << "set ytics nomirror; set border 3\n"
           << "set style line 11 lc rgb '#808080' lt 1;"
           << "set border 3 back ls 11;" << std::endl;
  plt_file << "plot '" << profile_file_name
           << "' u 1:(log($2)) t 'FoXS' w lines lw 2.5 lc rgb '#e26261'\n";
  plt_file << "reset\n";
  plt_file.close();
}

void Gnuplot::print_profile_script(const std::vector<std::string>& pdbs) {
  ColorCoder::set_number(pdbs.size());
  char hex_color[10] = "#ZZZZZZ";
  std::ofstream plt_file("profiles.plt");
  plt_file << "set terminal png enhanced;set output \"profiles.png\"\n";
  plt_file << "set ylabel '';set format y '';set xtics nomirror;"
           << "set ytics nomirror; set border 3\n"
           << "set style line 11 lc rgb '#808080' lt 1;"
           << "set border 3 back ls 11;" << std::endl;
  plt_file << "plot ";

  for (unsigned int i = 0; i < pdbs.size(); i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string profile_file_name = pdbs[i] + ".dat";
    plt_file << "'" << profile_file_name << "' u 1:(log($2)) t \""
             << saxs::trim_extension(pdbs[i]) << "\" w lines lw 2 lt " << i + 2;
    if (i == pdbs.size() - 1)
      plt_file << std::endl;
    else
      plt_file << ",";
  }
  plt_file << "reset\n";
  plt_file.close();
}

void Gnuplot::print_canvas_script(const std::vector<std::string>& pdbs,
                                  int max_num) {

  ColorCoder::set_number(pdbs.size());
  char hex_color[10] = "#ZZZZZZ";
  std::ofstream plt_file("canvas.plt");

  plt_file << "set terminal canvas solid butt size 400,350 fsize 10 lw 1.5 "
           << "fontscale 1 name \"jsoutput_1\" jsdir \".\"" << std::endl;
  plt_file << "set output 'jsoutput.1.js'" << std::endl;
  plt_file << "set xlabel 'q [Å^{-1}]';set ylabel 'log intensity'; "
	   << "set format y '10^{%L}'; set logscale y\n"
           << "set xtics nomirror;set ytics nomirror; set border 3\n"
           << "set style line 11 lc rgb '#808080' lt 1;unset key;"
           << "set border 3 back ls 11;" << std::endl;
  plt_file << "plot ";

  for (int i = 0; i < (int)pdbs.size() && i < max_num; i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string profile_file_name = pdbs[i] + ".dat";
    plt_file << "'" << profile_file_name << "' u 1:2 "
             << "w lines lw 2.5 lc rgb '#" << hex_color << "'";
    if (i == static_cast<int>(pdbs.size()) - 1 || i == max_num - 1)
      plt_file << std::endl;
    else
      plt_file << ",";
  }
  plt_file << "reset\n";
  plt_file.close();
}

void Gnuplot::print_fit_script(const IMP::saxs::FitParameters& fp) {
  // file names
  std::string pdb_name = saxs::trim_extension(fp.get_pdb_file_name());
  std::string profile_name = saxs::trim_extension(
      basename(const_cast<char*>(fp.get_profile_file_name().c_str())));
  std::string plt_file_name = pdb_name + "_" + profile_name + ".plt";
  std::string png_file_name = pdb_name + "_" + profile_name + ".png";
  std::string eps_file_name = pdb_name + "_" + profile_name + ".eps";
  std::string fit_file_name = pdb_name + "_" + profile_name + ".fit";
  // output script
  std::ofstream plt_file(plt_file_name.c_str());
  plt_file.precision(3);
  plt_file << "set terminal png enhanced; set output \"" << png_file_name
           << "\";" << std::endl;

  plt_file << "set lmargin 7; set rmargin 2;set multiplot\n";

  // lower residuals plot
  plt_file << "set origin 0,0;set size 1,0.3; set tmargin 0; set bmargin 3;"
           << "set ylabel 'Residual';set format y '';set xtics nomirror;"
           << "set xlabel 'q [Å^{-1}]';"
           << "set ytics nomirror; set border 3\n"
           << "set style line 11 lc rgb '#808080' lt 1;"
           << "set border 3 back ls 11;f(x)=1" << std::endl;
  plt_file << "plot f(x) notitle lc rgb '#333333'"
           << ", '" << fit_file_name
           << "' u 1:(($2-$4)/$3) notitle w lines lw 2.5 lc rgb '#e26261'\n";

  // upper fit plot
  plt_file << "set origin 0,0.3;set size 1,0.69; set bmargin 0; set tmargin 1;"
           << "set xlabel ''; set format x ''; "
           << "set ylabel 'I(q) log-scale';"
           << "set format y \"10^{%L}\"; set logscale y\n";
  plt_file << "plot '" << fit_file_name
           << "' u 1:2 t 'Experimental' lc rgb '#333333' pt 6 ps 0.8"
           << ", '" << fit_file_name
           << "' u 1:4 t 'FoXS χ^2 = " << fp.get_chi_square()
           << "' w lines lw 2.5 lc rgb '#e26261'\n";
  plt_file << "unset multiplot\n";
  plt_file << "reset\n";
  //  plt_file.close();
  // combined eps plot for paper
  plt_file << "set terminal postscript eps size 3.5,2.62 color enhanced solid ";
  plt_file << "linewidth 2.5 font 'Helvetica,22'; set output \""
           << eps_file_name << "\";" << std::endl;

  plt_file << "set lmargin 2; set rmargin 2;set multiplot\n";

  // lower residuals plot
  plt_file << "set origin 0,0;set size 1,0.3; set tmargin 0; set bmargin 3;";
  plt_file << "set ylabel '';set format y '';\n";
  plt_file << "set xtics nomirror font 'Helvetica,18';\n";
  plt_file << "set ytics nomirror; set border 3\n";
  plt_file << "set style line 11 lc rgb '#808080' lt 1;\n";
  plt_file << "set border 3 back ls 11;f(x)=1" << std::endl;
  plt_file << "plot f(x) notitle lc rgb '#333333'" << ", '" << fit_file_name
           << "' u 1:(($2-$4)/$3) notitle w lines lw 2.5 lc rgb '#e26261'\n";

  // upper fit plot
  plt_file << "set origin 0,0.3;set size 1,0.69; set bmargin 0; set tmargin 1;"
           << "set xlabel ''; set format x ''; set ylabel 'I(q) log-scale';\n"
           << "set format y \"10^{%L}\"; set logscale y\n";
  plt_file <<  "plot '" << fit_file_name
            << "' u 1:2 t 'Experimental' lc rgb '#333333' pt 6 ps 0.8";
  plt_file << ", '" << fit_file_name
           << "' u 1:4 t 'FoXS {/Symbol c}^2 = " << fp.get_chi_square()
           << "' w lines lw 2.5 lc rgb '#e26261'\n";
  plt_file << "unset multiplot\n";
  plt_file << "reset\n";
  plt_file.close();
}

void Gnuplot::print_fit_script(
    const std::vector<IMP::saxs::FitParameters>& fps) {
  ColorCoder::set_number(fps.size());
  char hex_color[10] = "#ZZZZZZ";
  std::ofstream plt_file("fit.plt");

  // terminal type
  plt_file << "set terminal png enhanced;set output \"fit.png\";" << std::endl;

  // formatting
  plt_file << "set lmargin 7; set rmargin 2;set multiplot\n"
           << "set origin 0,0;set size 1,0.3; set tmargin 0; set bmargin 3;"
           << "set ylabel 'Residual';set format y '';set xtics nomirror;"
           << "set xlabel 'q [Å^{-1}]';"
           << "set ytics nomirror; set border 3\n"
           << "set style line 11 lc rgb '#808080' lt 1;"
           << "set border 3 back ls 11" << std::endl;
  // residuals
  plt_file << "f(x)=1\n"
           << "plot f(x) notitle lc rgb '#333333'";
  for (unsigned int i = 0; i < fps.size(); i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string pdb_name = saxs::trim_extension(fps[i].get_pdb_file_name());
    std::string profile_name = saxs::trim_extension(
        basename(const_cast<char*>(fps[i].get_profile_file_name().c_str())));
    std::string fit_file_name = pdb_name + "_" + profile_name + ".fit";
    plt_file << ", '" << fit_file_name
             << "' u 1:(($2-$4)/$3) notitle w lines lw 2.5 lc rgb '#" << hex_color
             << "'";
  }
  plt_file << std::endl;
  // actual plots
  plt_file << "set origin 0,0.3;set size 1,0.69; set bmargin 0; set tmargin 1;"
           << "set xlabel ''; set format x '';"
           << "set ylabel 'I(q) log-scale';"
           << "set format y \"10^{%L}\"; set logscale y\n";
  for (unsigned int i = 0; i < fps.size(); i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string pdb_name = saxs::trim_extension(fps[i].get_pdb_file_name());
    std::string profile_name = saxs::trim_extension(
        basename(const_cast<char*>(fps[i].get_profile_file_name().c_str())));
    std::string fit_file_name = pdb_name + "_" + profile_name + ".fit";
    if (i == 0) {
      plt_file << "plot '" << fit_file_name
               << "' u 1:2 t 'Experimental' lc rgb '#333333' pt 6 ps 0.8 ";
    }
    plt_file << ", '" << fit_file_name << "' u 1:4 t '" << pdb_name
             << " χ^2 = " << fps[i].get_chi_square() << "' w lines lw 2.5 lc rgb '#"
             << hex_color << "'";
  }
  plt_file << std::endl;
  plt_file << "unset multiplot;reset" << std::endl;
  plt_file.close();
}

void Gnuplot::print_canvas_script(
    const std::vector<IMP::saxs::FitParameters>& fps, int max_num) {
  ColorCoder::set_number(fps.size());
  char hex_color[10] = "#ZZZZZZ";
  std::ofstream plt_file("canvas.plt");

  plt_file << "set terminal canvas solid butt size 400,350 fsize 10 lw 1.5 "
           << "fontscale 1 name \"jsoutput_1\" jsdir \".\"" << std::endl;
  plt_file << "set output 'jsoutput.1.js'" << std::endl;

  plt_file << "set multiplot\n";
  plt_file << "set lmargin 7\n";
  plt_file << "set origin 0,0;set size 1,0.3; set tmargin 0;"
           << "set xlabel 'q [Å^{-1}]';set ylabel 'Residual';set format y '';"
           << "set xtics nomirror;set ytics nomirror;unset key;"
           << "set border 3; set style line 11 lc rgb '#808080' lt 1;"
           << "set border 3 back ls 11" << std::endl;
  // residuals
  plt_file << "f(x)=1\n"
           << "plot f(x) lc rgb '#333333'";
  for (int i = 0; i < (int)fps.size() && i < max_num; i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string pdb_name = saxs::trim_extension(fps[i].get_pdb_file_name());
    std::string profile_name = saxs::trim_extension(
        basename(const_cast<char*>(fps[i].get_profile_file_name().c_str())));
    std::string fit_file_name = pdb_name + "_" + profile_name + ".fit";
    plt_file << ", '" << fit_file_name
             << "' u 1:(($2-$4)/$3) w lines lw 2.5 lc rgb '#" << hex_color << "'";
  }
  plt_file << std::endl;
  // actual plots
  plt_file << "set origin 0,0.3;set size 1,0.69; set bmargin 0; set tmargin 1; "
           << "set xlabel ''; set format x ''; set ylabel 'log intensity'; "
	   << "set format y '10^{%L}'; set logscale y\n";
  for (int i = 0; i < (int)fps.size() && i < max_num; i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string pdb_name = saxs::trim_extension(fps[i].get_pdb_file_name());
    std::string profile_name = saxs::trim_extension(
        basename(const_cast<char*>(fps[i].get_profile_file_name().c_str())));
    std::string fit_file_name = pdb_name + "_" + profile_name + ".fit";
    if (i == 0) {
      plt_file << "plot '" << fit_file_name
               << "' u 1:2 lc rgb '#333333' pt 6 ps 0.8 ";
    }
    plt_file << ", '" << fit_file_name << "' u 1:4 "
             << "w lines lw 2.5 lc rgb '#" << hex_color << "'";
  }
  plt_file << std::endl;
  plt_file << "unset multiplot;reset" << std::endl;
  plt_file.close();
}

IMPFOXS_END_INTERNAL_NAMESPACE
