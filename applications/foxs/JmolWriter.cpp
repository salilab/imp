/**
 * \file JmolWriter \brief outputs javascript for jmol display
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include "JmolWriter.h"

#include "ColorCoder.h"
#include "Gnuplot.h"

#include <IMP/atom/pdb.h>
#include <IMP/saxs/utility.h>

#include <fstream>
#include <boost/lexical_cast.hpp>

std::string JmolWriter::show_selection_ =
  "frame 0#;restrict selection;select selection; ribbons";

void JmolWriter::prepare_jmol_script(
                              const std::vector<IMP::saxs::FitParameters>& fps,
                              const std::vector<IMP::Particles>& particles_vec,
                              const std::string filename) {

  std::string html_filename = filename + ".html";
  std::string pdb_filename = filename + ".pdb";

  std::string pdb_colors = prepare_coloring_string(fps.size());
  prepare_PDB_file(fps, particles_vec, pdb_filename);

  std::ofstream outstream(html_filename.c_str());
  outstream << jmol_script("/foxs/jmol");
  std::string init = "select all; spacefill off; ribbons; restrict not all;";
  std::string selection_init = "define selection model =1";

  init += selection_init + "; " + pdb_colors + show_selection_
    + "; background white; hide hydrogens;";

  // load applet with molecules
  outstream << "<td width=350 height=350> <div align=\"center\">\n"
            << "<script type=\"text/javascript\"> jmolApplet(350, 'load "
            << pdb_filename << "; " << init;
  outstream << prepare_gnuplot_init_selection_string(fps.size(), true);
  outstream << "');\n" << "</script> </div> </td> </tr> \n </table>\n";

  outstream.precision(2);
  outstream.setf(std::ios::fixed,std::ios::floatfield);
  // output table
  bool showMolecule = true;
  char hex_color[10]="ZZZZZZ";
  outstream << "<table align='center'>";
  outstream << "<tr><td> PDB file </td> <td> show/hide </td>"
            << "<td><center> &chi; </td><td><center> c<sub>1</sub> </td>"
            << "<td><center> c<sub>2</sub> </td></tr>\n";
  for(unsigned int i=0; i<fps.size(); i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string pdb_name = trim_extension(fps[i].get_pdb_file_name());
    outstream << "<tr><td> <font color=#" << hex_color << ">" << pdb_name;
    outstream<< "</font></td>\n<td><center>" << std::endl;
    if(i>0) showMolecule = false;
    std::string checkbox_string = model_checkbox(i, showMolecule, true);
    outstream << checkbox_string << std::endl;
    outstream << "</center></td><td><center> " << fps[i].get_chi()
              << "</center></td><td><center> " << fps[i].get_c1()
              << "</center></td><td><center> " << fps[i].get_c2() << "</tr>\n";
  }
  outstream << "</table>\n";
  outstream.close();
}

void JmolWriter::prepare_jmol_script(const std::vector<std::string>& pdbs,
                              const std::vector<IMP::Particles>& particles_vec,
                              const std::string filename) {

  std::string html_filename = filename + ".html";
  std::string pdb_filename = filename + ".pdb";

  std::string pdb_colors = prepare_coloring_string(pdbs.size());
  prepare_PDB_file(particles_vec, pdb_filename);

  std::ofstream outstream(html_filename.c_str());
  outstream << jmol_script("/foxs/jmol");
  std::string init = "select all; spacefill off; ribbons; restrict not all;";
  std::string selection_init = "define selection model =1";

  init += selection_init + "; " + pdb_colors + show_selection_
    + "; background white; hide hydrogens;";

  // load applet with molecules
  outstream << "<td width=350 height=350> <div align=\"center\">\n"
            << "<script type=\"text/javascript\"> jmolApplet(350, 'load "
            << pdb_filename << "; " << init;
  outstream << prepare_gnuplot_init_selection_string(pdbs.size(), false);
  outstream << "');\n" << "</script> </div> </td> </tr> \n </table>\n";

  outstream.precision(2);
  outstream.setf(std::ios::fixed,std::ios::floatfield);
  // output table
  bool showMolecule = true;
  char hex_color[10]="ZZZZZZ";
  outstream << "<table align='center'>";
  outstream << "<tr><td> PDB file </td> <td> show/hide </td></tr>\n";
  for(unsigned int i=0; i<pdbs.size(); i++) {
    ColorCoder::html_hex_color(hex_color, i);
    std::string pdb_name = trim_extension(pdbs[i]);
    outstream << "<tr><td> <font color=#" << hex_color << ">" << pdb_name;
    outstream<< "</font></td>\n<td><center>" << std::endl;
    if(i>0) showMolecule = false;
    std::string checkbox_string = model_checkbox(i, showMolecule, false);
    outstream << checkbox_string << std::endl;
    outstream << "</tr>\n";
  }
  outstream << "</table>\n";
  outstream.close();
}

void JmolWriter::prepare_PDB_file(
                            const std::vector<IMP::saxs::FitParameters>& fps,
                            const std::vector<IMP::Particles>& particles_vec,
                            const std::string filename) {
  std::ofstream out_file(filename.c_str());
  // center coordinates and join into a single PDB
  for(unsigned int i=0; i<fps.size(); i++) {
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
  for(unsigned int i=0; i<particles_vec.size(); i++) {

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
      gnuplot_string += "javascript  gnuplot.toggle_plot(\"jsoutput_1_plot_" +
        std::string(boost::lexical_cast<std::string>(i+3)) + "\");";
    } else {
      gnuplot_string += "javascript  gnuplot.toggle_plot(\"jsoutput_1_plot_" +
        std::string(boost::lexical_cast<std::string>(i+2)) + "\");";
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
  checkbox_string += "'javascript gnuplot.toggle_plot(\"jsoutput_1_plot_"
    + model_num_string2 + "\");";
  checkbox_string += "define selection selection, model=" + model_num_string
    + ";" + show_selection_ + "',";
  checkbox_string += "'javascript gnuplot.toggle_plot(\"jsoutput_1_plot_"
    + model_num_string2 + "\");";
  checkbox_string += "define selection selection and not model="
    + model_num_string + ";" + show_selection_ + "',\"\"";
  if(is_checked)  checkbox_string += ",\"isChecked\"";
  checkbox_string += ") </script>\n";
  return checkbox_string;
}
