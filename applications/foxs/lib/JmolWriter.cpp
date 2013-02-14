/**
 * \file JmolWriter.cpp \brief outputs javascript for jmol display
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "JmolWriter.h"

#include "ColorCoder.h"
#include "Gnuplot.h"

#include <IMP/atom/pdb.h>
#include <IMP/saxs/utility.h>

#include <fstream>
#include <boost/lexical_cast.hpp>

std::string JmolWriter::display_selection_ =
  "frame 0#;restrict selection;\
select selection and (protein, nucleic); ribbons only;\
select selection and not (protein, nucleic); spacefill only;\
if (!{*}.ribbons) { select selection and (protein, nucleic);spacefill only; };";

unsigned int JmolWriter::MAX_DISPLAY_NUM_ = 30;

void JmolWriter::prepare_jmol_script(
                              const std::vector<IMP::saxs::FitParameters>& fps,
                              const std::vector<IMP::Particles>& particles_vec,
                              const std::string filename) {

  std::string html_filename = filename + ".html";
  std::string pdb_filename = filename + ".pdb";

  unsigned int model_num = std::min((unsigned int)fps.size(), MAX_DISPLAY_NUM_);
  std::string pdb_colors = prepare_coloring_string(model_num);
  prepare_PDB_file(fps, particles_vec, pdb_filename);

  std::ofstream outstream(html_filename.c_str());
  outstream << prepare_gnuplot_init_selection_string(model_num, true);
  outstream << jmol_script("/foxs/jmol");
  std::string init = "select all;" + display_selection_;
  std::string selection_init = "define selection model =1";

  init += selection_init + "; " + pdb_colors + display_selection_
    + "; background white; hide hydrogens;";

  // load applet with molecules
  outstream <<"<td width=350 height=350><div id=\"wrapper\" align=\"center\">\n"
            << "<script type=\"text/javascript\"> jmolApplet(350, 'load "
            << pdb_filename << "; " << init;
  outstream << "');\n" << "</script> </div> </td> </tr> \n </table>\n";

  std::string show_all_checkbox_str = show_all_checkbox(model_num, true);

  outstream.precision(2);
  outstream.setf(std::ios::fixed,std::ios::floatfield);
  // output table
  bool showMolecule = true;
  char hex_color[10]="ZZZZZZ";
  outstream << "<table align='center'>";
  outstream << "<tr><th> PDB file </th> "
            << "<th> " << show_all_checkbox_str << "</th>"
            << "<th><center> &chi; </th><th><center> c<sub>1</sub> </th>"
            << "<th><center> c<sub>2</sub> </th><th><center>R<sub>g</sub></th>"
            << "<th><center> # atoms </th> <th> fit file </th></tr>\n";
  for(unsigned int i=0; i<fps.size(); i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string pdb_name = trim_extension(fps[i].get_pdb_file_name());
    std::string profile_name = trim_extension(
          basename(const_cast<char *>(fps[i].get_profile_file_name().c_str())));
    std::string fit_file_name = pdb_name + "_" + profile_name + ".dat";
    float rg =
      IMP::saxs::radius_of_gyration(particles_vec[fps[i].get_mol_index()]);
    outstream << "<tr><td>";
    if(i<MAX_DISPLAY_NUM_) outstream << "<font color=#" << hex_color << ">";
    outstream  << pdb_name;
    if(i<MAX_DISPLAY_NUM_) outstream << "</font>";
    outstream<< "</td>\n<td><center>" << std::endl;
    if(i>0) showMolecule = false;
    if(i<MAX_DISPLAY_NUM_) {
      std::string checkbox_string = model_checkbox(i, showMolecule, true);
      outstream << checkbox_string << std::endl;
    }
    outstream << "</center></td><td><center> " << fps[i].get_chi()
              << "</center></td><td><center> " << fps[i].get_c1()
              << "</center></td><td><center> " << fps[i].get_c2()
              << "</center></td><td><center> " << rg
              << "</center></td><td><center> "
              << particles_vec[fps[i].get_mol_index()].size() << "</td><td>"
              << "<a href = \"dirname/" << fit_file_name
              << "\">" << fit_file_name
              << "</a></td></tr>\n";
  }
  outstream << "</table>\n";
  outstream << group_checkbox(model_num);
  outstream.close();
}

void JmolWriter::prepare_jmol_script(const std::vector<std::string>& pdbs,
                              const std::vector<IMP::Particles>& particles_vec,
                              const std::string filename) {

  std::string html_filename = filename + ".html";
  std::string pdb_filename = filename + ".pdb";

  unsigned int model_num = std::min((unsigned int)pdbs.size(),MAX_DISPLAY_NUM_);
  std::string pdb_colors = prepare_coloring_string(model_num);

  prepare_PDB_file(particles_vec, pdb_filename);

  std::ofstream outstream(html_filename.c_str());
  outstream << prepare_gnuplot_init_selection_string(model_num, false);
  outstream << jmol_script("/foxs/jmol");
  std::string init = "select all;" + display_selection_;
  std::string selection_init = "define selection model =1";

  init += selection_init + "; " + pdb_colors + display_selection_
    + "; background white; hide hydrogens;";

  // load applet with molecules
  outstream <<"<td width=350 height=350><div id=\"wrapper\" align=\"center\">\n"
            << "<script type=\"text/javascript\"> jmolApplet(350, 'load "
            << pdb_filename << "; " << init;
  outstream << "');\n" << "</script> </div> </td> </tr> \n </table>\n";

  std::string show_all_checkbox_str = show_all_checkbox(model_num, false);

  outstream.precision(2);
  outstream.setf(std::ios::fixed,std::ios::floatfield);
  // output table
  bool showMolecule = true;
  char hex_color[10]="ZZZZZZ";
  outstream << "<table align='center'>";
  outstream << "<tr><th> PDB file </th>"
            << "<th> " << show_all_checkbox_str << " </th>"
            << "<th><center> R<sub>g</sub> </th>"
            << "<th><center> # atoms </th> <th> Profile file</th></tr>\n";
  for(unsigned int i=0; i<pdbs.size(); i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string pdb_name = trim_extension(pdbs[i]);
    std::string profile_name = pdbs[i] + ".dat";
    float rg = IMP::saxs::radius_of_gyration(particles_vec[i]);
    outstream << "<tr><td>";
    if(i<MAX_DISPLAY_NUM_) outstream << "<font color=#" << hex_color << ">";
    outstream << pdb_name;
    if(i<MAX_DISPLAY_NUM_) outstream<< "</font>";
    outstream << "</td>\n<td><center>" << std::endl;
    if(i>0) showMolecule = false;
    if(i<MAX_DISPLAY_NUM_) {
      std::string checkbox_string = model_checkbox(i, showMolecule, false);
      outstream << checkbox_string << std::endl;
    }
    outstream << "</center></td><td><center> " << rg
              << "</center></td><td><center> "
              << particles_vec[i].size() << "</td><td><a href = \"dirname/"
              << profile_name << "\">" << profile_name << "</a></td></tr>\n";
  }
  outstream << "</table>\n";
  outstream << group_checkbox(model_num);
  outstream.close();
}

void JmolWriter::prepare_PDB_file(
                            const std::vector<IMP::saxs::FitParameters>& fps,
                            const std::vector<IMP::Particles>& particles_vec,
                            const std::string filename) {
  std::ofstream out_file(filename.c_str());
  // center coordinates and join into a single PDB
  for(unsigned int i=0; i<fps.size() && i<MAX_DISPLAY_NUM_; i++) {
    int mol_index = fps[i].get_mol_index();

    // compute mean
    std::vector<IMP::algebra::Vector3D> coordinates;
    IMP::saxs::get_coordinates(particles_vec[mol_index], coordinates);
    IMP::algebra::Vector3D m =
      std::accumulate(coordinates.begin(), coordinates.end(),
                      IMP::algebra::Vector3D(0.0, 0.0, 0.0));
    m /= particles_vec[mol_index].size();

    // output file
    out_file << "MODEL    " << i+1 << std::endl;
    for(unsigned int j=0; j< particles_vec[mol_index].size(); j++) {
      // centering
      IMP::algebra::Vector3D v =
        IMP::core::XYZ(particles_vec[mol_index][j]).get_coordinates()-m;
      IMP::atom::Atom ad(particles_vec[mol_index][j]);
      IMP::atom::Residue rd = get_residue(ad);
      IMP::atom::Chain c = get_chain(rd);
      char chain = c.get_id();
      out_file <<
       IMP::atom::get_pdb_string(v, ad.get_input_index(), ad.get_atom_type(),
                                 rd.get_residue_type(), chain, rd.get_index(),
                                 rd.get_insertion_code(), ad.get_occupancy(),
                                 ad.get_temperature_factor(), ad.get_element());
    }
    out_file << "ENDMDL" << std::endl;
  }
  out_file.close();
}

void JmolWriter::prepare_PDB_file(
                            const std::vector<IMP::Particles>& particles_vec,
                            const std::string filename) {
  std::ofstream out_file(filename.c_str());
  // center coordinates and join into a single PDB
  for(unsigned int i=0; i<particles_vec.size() && i<MAX_DISPLAY_NUM_; i++) {

    // compute mean
    std::vector<IMP::algebra::Vector3D> coordinates;
    IMP::saxs::get_coordinates(particles_vec[i], coordinates);
    IMP::algebra::Vector3D m =
      std::accumulate(coordinates.begin(), coordinates.end(),
                      IMP::algebra::Vector3D(0.0, 0.0, 0.0));
    m /= particles_vec[i].size();

    // output file
    out_file << "MODEL    " << i+1 << std::endl;
    for(unsigned int j=0; j< particles_vec[i].size(); j++) {
      // centering
      IMP::algebra::Vector3D v =
        IMP::core::XYZ(particles_vec[i][j]).get_coordinates()-m;
      IMP::atom::Atom ad(particles_vec[i][j]);
      IMP::atom::Residue rd = get_residue(ad);
      IMP::atom::Chain c = get_chain(rd);
      char chain = c.get_id();
      out_file <<
       IMP::atom::get_pdb_string(v, ad.get_input_index(), ad.get_atom_type(),
                                 rd.get_residue_type(), chain, rd.get_index(),
                                 rd.get_insertion_code(), ad.get_occupancy(),
                                 ad.get_temperature_factor(), ad.get_element());
    }
    out_file << "ENDMDL" << std::endl;
  }
  out_file.close();
}

std::string JmolWriter::prepare_gnuplot_init_selection_string(
                                             unsigned int model_num, bool exp) {
  std::string gnuplot_string = "";
  for(unsigned int i=0; i<model_num-1; i++) {
    if(exp) {
      gnuplot_string += "<script> gnuplot.hide_plot(\"jsoutput_1_plot_" +
        std::string(boost::lexical_cast<std::string>(i+3)) + "\");</script>";
    } else {
      gnuplot_string += "<script>  gnuplot.hide_plot(\"jsoutput_1_plot_" +
        std::string(boost::lexical_cast<std::string>(i+2)) + "\");</script>";
    }
  }
  return gnuplot_string;
}

std::string JmolWriter::prepare_coloring_string(unsigned int model_num) {
  char dec_color[20];
  std::string coloring_string = "";
  for(unsigned int i=0; i<model_num; i++) {
    ColorCoder::jmol_dec_color(dec_color, i);
    coloring_string += "select model = "
      + std::string(boost::lexical_cast<std::string>(i+1))
      + "; color " + std::string(dec_color) + ";";
  }
  return coloring_string;
}

std::string JmolWriter::jmol_script(std::string jmol_path) {
  std::string js_string = "";
  js_string += "<script src=\"" + jmol_path + "/Jmol.js\"></script>\n";
  js_string += "<script> jmolInitialize(\"" + jmol_path
    + "\", \"JmolAppletSigned.jar\"); </script>\n";
  return js_string;
}

std::string JmolWriter::model_checkbox(unsigned int model_num,
                                       bool is_checked, bool exp) {
  std::string model_num_string =
    std::string(boost::lexical_cast<std::string>(model_num+1));
  std::string model_num_string2 = model_num_string;
  if(exp) model_num_string2 =
            std::string(boost::lexical_cast<std::string>(model_num+2));
  std::string checkbox_string = "<script>";
  checkbox_string += "\n jmolCheckbox(";
  checkbox_string += "'javascript gnuplot.show_plot(\"jsoutput_1_plot_"
    + model_num_string2 + "\");";
  checkbox_string += "define selection selection, model=" + model_num_string
    + ";" + display_selection_ + "',";
  checkbox_string += "'javascript gnuplot.hide_plot(\"jsoutput_1_plot_"
    + model_num_string2 + "\");";
  checkbox_string += "define selection selection and not model="
    + model_num_string + ";" + display_selection_ + "',\"\"";
  if(is_checked)  checkbox_string += ",\"isChecked\"";
  checkbox_string += ") </script>\n";
  return checkbox_string;
}

std::string JmolWriter::show_all_checkbox(unsigned int model_num, bool exp) {
  if(model_num == 1) return "show/hide";
  std::string show_string = "select all;define selection all;frame 0#;" +
    display_selection_;
  std::string hide_string =
    "define selection not all;frame 0#;restrict not all;";
  for(unsigned int i=0; i<model_num; i++) {
    unsigned int plotnum = i+1;
    if(exp) plotnum = i+2;
    std::string plotnum_str =
      std::string(boost::lexical_cast<std::string>(plotnum));
    show_string +=
      "javascript gnuplot.show_plot(\"jsoutput_1_plot_" + plotnum_str + "\");";
    hide_string +=
      "javascript gnuplot.hide_plot(\"jsoutput_1_plot_" + plotnum_str + "\");";
  }
  std::string ret = "<script>jmolCheckbox('" + show_string + "', '"
    + hide_string + "', \"\");</script> show all/hide all";
  return ret;
}

std::string JmolWriter::group_checkbox(unsigned int model_num) {
  if(model_num == 1) return "";
  std::string ret = "<script> jmolSetCheckboxGroup(0";
  for(unsigned int i=0; i<model_num; i++) {
    ret += ", " + std::string(boost::lexical_cast<std::string>(i+1));
  }
  ret += ");</script> \n";
  return ret;
}
