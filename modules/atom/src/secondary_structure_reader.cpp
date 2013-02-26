/**
 *  \file secondary_structure_reader.cpp   \brief Reading PSIPRED/DSSP results
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/secondary_structure_reader.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <string>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

IMPATOM_BEGIN_NAMESPACE

namespace {

Strings parse_psipred_file(base::TextInput inf){
  std::string line;
  std::vector<std::string> line_split,frag_split,res_split;
  std::string conf="",pred="",aa="";
  while (!inf.get_stream().eof()) {
    getline(inf.get_stream(), line);
    if (inf.get_stream().eof()) break;
    if (inf.get_stream().bad() || inf.get_stream().fail()) {
      IMP_THROW("Error reading from PDB file", IOException);
    }
    line.erase(std::remove(line.begin(),line.end(),' '),line.end());
    if (boost::starts_with(line, "Conf:")){
        boost::split(line_split, line, boost::is_any_of(":"));
        conf+=line_split[1];
    }
    else if (boost::starts_with(line, "Pred:")){
        boost::split(line_split, line, boost::is_any_of(":"));
        pred+=line_split[1];
    }
    else if (boost::starts_with(line, "AA:")){
        boost::split(line_split, line, boost::is_any_of(":"));
        aa+=line_split[1];
    }
  }
  Strings ss;
  ss.push_back(conf);
  ss.push_back(pred);
  ss.push_back(aa);
  return ss;
}

  SecondaryStructureResidues create_sses_from_strings(Strings ss,
                                                      Particles ps){
  SecondaryStructureResidues ssrs;
  std::string conf=ss[0],pred=ss[1],aa=ss[2];
  for (int nres=0;nres<(int)ss[0].length();nres++){
    if (!Residue::particle_is_instance(ps[nres])){
      // TODO: actually use 1-letter-code here (aa[nc])
      Residue::setup_particle(ps[nres],ResidueType("GLY"),nres);
    }
    SecondaryStructureResidue ssr=
      SecondaryStructureResidue::setup_particle(ps[nres],0.0,0.0,0.0);
    double prob=(boost::lexical_cast<float>(conf[nres])+1.0)/10.0;
    if (pred[nres]=='H') ssr.set_prob_helix(prob);
    else if (pred[nres]=='E') ssr.set_prob_strand(prob);
    else if (pred[nres]=='C') ssr.set_prob_coil(prob);
    ssrs.push_back(ssr);
  }
  return ssrs;
}
}

SecondaryStructureResidues read_psipred(base::TextInput inf,
                                        Model * mdl){
  Strings ss=parse_psipred_file(inf);
  int nres=ss[0].size();
  Particles ps;
  for (int nr=0;nr<nres;nr++){
    IMP_NEW(Particle,p,(mdl));
    ps.push_back(p);
  }
  return create_sses_from_strings(ss,ps);
}

SecondaryStructureResidues read_psipred(base::TextInput inf,
                                        Particles ps){
  Strings ss=parse_psipred_file(inf);
  return create_sses_from_strings(ss,ps);
}

IMPATOM_END_NAMESPACE
