/**
 *  \file protein_ligand_score.cpp   \brief A class for reading mol2 files
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/protein_ligand_score.h>
#include <IMP/atom/internal/mol2.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

namespace {

  enum ProteinType { PROTEIN_INVALID=-1,
                     ALA_N=0, ALA_CA, ALA_C, ALA_O, ALA_CB,

                     ARG_N, ARG_CA, ARG_C, ARG_O, ARG_CB, ARG_CG, ARG_CD,
                     ARG_NE, ARG_CZ, ARG_NH,

                     ASN_N, ASN_CA, ASN_C, ASN_O, ASN_CB, ASN_CG, ASN_OD1,
                     ASN_ND2,

                     ASP_N, ASP_CA, ASP_C, ASP_O, ASP_CB, ASP_CG, ASP_OD,

                     CYS_N, CYS_CA, CYS_C, CYS_O, CYS_CB, CYS_SG,

                     GLN_N, GLN_CA, GLN_C, GLN_O, GLN_CB, GLN_CG, GLN_CD,
                     GLN_OE1, GLN_NE2,

                     GLU_N, GLU_CA, GLU_C, GLU_O, GLU_CB, GLU_CG, GLU_CD,
                     GLU_OE,

                     GLY_N, GLY_CA, GLY_C, GLY_O,

                     HIS_N, HIS_CA, HIS_C, HIS_O, HIS_CB, HIS_CG, HIS_ND1,
                     HIS_CD2, HIS_CE1, HIS_NE2,

                     ILE_N, ILE_CA, ILE_C, ILE_O, ILE_CB, ILE_CG1, ILE_CG2,
                     ILE_CD1,

                     LEU_N, LEU_CA, LEU_C, LEU_O, LEU_CB, LEU_CG, LEU_CD,

                     LYS_N, LYS_CA, LYS_C, LYS_O, LYS_CB, LYS_CG, LYS_CD,
                     LYS_CE, LYS_NZ,

                     MET_N, MET_CA, MET_C, MET_O, MET_CB, MET_CG, MET_SD,
                     MET_CE,

                     PHE_N, PHE_CA, PHE_C, PHE_O, PHE_CB, PHE_CG, PHE_CD,
                     PHE_CE,  PHE_CZ,

                     PRO_N, PRO_CA, PRO_C, PRO_O, PRO_CB, PRO_CG, PRO_CD,

                     SER_N, SER_CA, SER_C, SER_O, SER_CB, SER_OG,

                     THR_N, THR_CA, THR_C, THR_O, THR_CB, THR_OG1, THR_CG2,

                     TRP_N, TRP_CA, TRP_C, TRP_O, TRP_CB, TRP_CG, TRP_CD1,
                     TRP_CD2, TRP_NE1, TRP_CE2, TRP_CE3, TRP_CZ2, TRP_CZ3,
                     TRP_CH2,

                     TYR_N, TYR_CA, TYR_C, TYR_O, TYR_CB, TYR_CG, TYR_CD,
                     TYR_CE,  TYR_CZ,  TYR_OH,

                     VAL_N, VAL_CA, VAL_C, VAL_O, VAL_CB, VAL_CG,

                     PROTEIN_LAST};

  enum LigandType{LIGAND_INVALID=-1,
                  C1=0, C2, C3, Car, Ccat, N1, N2, N3, N4, Nar, Nam, Npl3,
                  O2, O3, Oco2, Oar, S2, S3, So, So2, Sar, P3, F, Cl, Br, I,
                  LIGAND_LAST};

  struct ProteinTypeData {
    std::string name;
    ProteinType type;
  };

  ProteinTypeData pat_data[]= {
    {"ALA_N",   ALA_N},
    {"ALA_CA",  ALA_CA},
    {"ALA_C",   ALA_C},
    {"ALA_O",   ALA_O},
    {"ALA_CB",  ALA_CB},
    {"ARG_N",   ARG_N},
    {"ARG_CA",  ARG_CA},
    {"ARG_C",   ARG_C},
    {"ARG_O",   ARG_O},
    {"ARG_CB",  ARG_CB},
    {"ARG_CG",  ARG_CG},
    {"ARG_CD",  ARG_CD},
    {"ARG_NE",  ARG_NE},
    {"ARG_CZ",  ARG_CZ},
    {"ARG_NH1", ARG_NH},
    {"ARG_NH2", ARG_NH},
    {"ASN_N",   ASN_N},
    {"ASN_CA",  ASN_CA},
    {"ASN_C",   ASN_C},
    {"ASN_O",   ASN_O},
    {"ASN_CB",  ASN_CB},
    {"ASN_CG",  ASN_CG},
    {"ASN_OD1", ASN_OD1},
    {"ASN_ND2", ASN_ND2},
    {"ASP_N",   ASP_N},
    {"ASP_CA",  ASP_CA},
    {"ASP_C",   ASP_C},
    {"ASP_O",   ASP_O},
    {"ASP_CB",  ASP_CB},
    {"ASP_CG",  ASP_CG},
    {"ASP_OD1", ASP_OD},
    {"ASP_OD2", ASP_OD},
    {"CYS_N",   CYS_N},
    {"CYS_CA",  CYS_CA},
    {"CYS_C",   CYS_C},
    {"CYS_O",   CYS_O},
    {"CYS_CB",  CYS_CB},
    {"CYS_SG",  CYS_SG},
    {"GLN_N",   GLN_N},
    {"GLN_CA",  GLN_CA},
    {"GLN_C",   GLN_C},
    {"GLN_O",   GLN_O},
    {"GLN_CB",  GLN_CB},
    {"GLN_CG",  GLN_CG},
    {"GLN_CD",  GLN_CD},
    {"GLN_OE1", GLN_OE1},
    {"GLN_NE2", GLN_NE2},
    {"GLU_N",   GLU_N},
    {"GLU_CA",  GLU_CA},
    {"GLU_C",   GLU_C},
    {"GLU_O",   GLU_O},
    {"GLU_CB",  GLU_CB},
    {"GLU_CG",  GLU_CG},
    {"GLU_CD",  GLU_CD},
    {"GLU_OE1", GLU_OE},
    {"GLU_OE2", GLU_OE},
    {"GLY_N",   GLY_N},
    {"GLY_CA",  GLY_CA},
    {"GLY_C",   GLY_C},
    {"GLY_O",   GLY_O},
    {"HIS_N",   HIS_N},
    {"HIS_CA",  HIS_CA},
    {"HIS_C",   HIS_C},
    {"HIS_O",   HIS_O},
    {"HIS_CB",  HIS_CB},
    {"HIS_CG",  HIS_CG},
    {"HIS_ND1", HIS_ND1},
    {"HIS_CD2", HIS_CD2},
    {"HIS_CE1", HIS_CE1},
    {"HIS_NE2", HIS_NE2},
    {"ILE_N",   ILE_N},
    {"ILE_CA",  ILE_CA},
    {"ILE_C",   ILE_C},
    {"ILE_O",   ILE_O},
    {"ILE_CB",  ILE_CB},
    {"ILE_CG1", ILE_CG1},
    {"ILE_CG2", ILE_CG2},
    {"ILE_CD1", ILE_CD1},
    {"LEU_N",   LEU_N},
    {"LEU_CA",  LEU_CA},
    {"LEU_C",   LEU_C},
    {"LEU_O",   LEU_O},
    {"LEU_CB",  LEU_CB},
    {"LEU_CG",  LEU_CG},
    {"LEU_CD1", LEU_CD},
    {"LEU_CD2", LEU_CD},
    {"LYS_N",   LYS_N},
    {"LYS_CA",  LYS_CA},
    {"LYS_C",   LYS_C},
    {"LYS_O",   LYS_O},
    {"LYS_CB",  LYS_CB},
    {"LYS_CG",  LYS_CG},
    {"LYS_CD",  LYS_CD},
    {"LYS_CE",  LYS_CE},
    {"LYS_NZ",  LYS_NZ},
    {"MET_N",   MET_N},
    {"MET_CA",  MET_CA},
    {"MET_C",   MET_C},
    {"MET_O",   MET_O},
    {"MET_CB",  MET_CB},
    {"MET_CG",  MET_CG},
    {"MET_SD",  MET_SD},
    {"MET_CE",  MET_CE},
    {"PHE_N",   PHE_N},
    {"PHE_CA",  PHE_CA},
    {"PHE_C",   PHE_C},
    {"PHE_O",   PHE_O},
    {"PHE_CB",  PHE_CB},
    {"PHE_CG",  PHE_CG},
    {"PHE_CD1", PHE_CD},
    {"PHE_CD2", PHE_CD},
    {"PHE_CE1", PHE_CE},
    {"PHE_CE2", PHE_CE},
    {"PHE_CZ",  PHE_CZ},
    {"PRO_N",   PRO_N},
    {"PRO_CA",  PRO_CA},
    {"PRO_C",   PRO_C},
    {"PRO_O",   PRO_O},
    {"PRO_CB",  PRO_CB},
    {"PRO_CG",  PRO_CG},
    {"PRO_CD",  PRO_CD},
    {"SER_N",   SER_N},
    {"SER_CA",  SER_CA},
    {"SER_C",   SER_C},
    {"SER_O",   SER_O},
    {"SER_CB",  SER_CB},
    {"SER_OG",  SER_OG},
    {"THR_N",   THR_N},
    {"THR_CA",  THR_CA},
    {"THR_C",   THR_C},
    {"THR_O",   THR_O},
    {"THR_CB",  THR_CB},
    {"THR_OG1", THR_OG1},
    {"THR_CG2", THR_CG2},
    {"TRP_N",   TRP_N},
    {"TRP_CA",  TRP_CA},
    {"TRP_C",   TRP_C},
    {"TRP_O",   TRP_O},
    {"TRP_CB",  TRP_CB},
    {"TRP_CG",  TRP_CG},
    {"TRP_CD1", TRP_CD1},
    {"TRP_CD2", TRP_CD2},
    {"TRP_NE1", TRP_NE1},
    {"TRP_CE2", TRP_CE2},
    {"TRP_CE3", TRP_CE3},
    {"TRP_CZ2", TRP_CZ2},
    {"TRP_CZ3", TRP_CZ3},
    {"TRP_CH2", TRP_CH2},
    {"TYR_N",   TYR_N},
    {"TYR_CA",  TYR_CA},
    {"TYR_C",   TYR_C},
    {"TYR_O",   TYR_O},
    {"TYR_CB",  TYR_CB},
    {"TYR_CG",  TYR_CG},
    {"TYR_CD1", TYR_CD},
    {"TYR_CD2", TYR_CD},
    {"TYR_CE1", TYR_CE},
    {"TYR_CE2", TYR_CE},
    {"TYR_CZ",  TYR_CZ},
    {"TYR_OH",  TYR_OH},
    {"VAL_N",   VAL_N},
    {"VAL_CA",  VAL_CA},
    {"VAL_C",   VAL_C},
    {"VAL_O",   VAL_O},
    {"VAL_CB",  VAL_CB},
    {"VAL_CG1", VAL_CG},
    {"VAL_CG2", VAL_CG}};

  static const unsigned int number_of_protein_mappings
  =sizeof(pat_data)/sizeof(ProteinTypeData);
  BOOST_STATIC_ASSERT(number_of_protein_mappings >= PROTEIN_LAST);

  struct LigandTypeData {
    std::string name;
    LigandType type;
  };

  LigandTypeData lat_data[]= {
    {"C1",   C1},
    {"C2",   C2},
    {"C3",   C3},
    {"Car",  Car},
    {"Ccat", Ccat},
    {"N1",   N1},
    {"N2",   N2},
    {"N3",   N3},
    {"N4",   N4},
    {"Nar",  Nar},
    {"Nam",  Nam},
    {"Npl3", Npl3},
    {"O2",   O2},
    {"O3",   O3},
    {"Oco2", Oco2},
    {"Oar",  Oar},
    {"S2",   S2},
    {"S3",   S3},
    {"So",   So},
    {"So2",  So2},
    {"Sar",  Sar},
    {"P3",   P3},
    {"F",    F},
    {"Cl",   Cl},
    {"Br",   Br},
    {"I",    I}};
  static const unsigned int number_of_ligand_mappings
  =sizeof(lat_data)/sizeof(LigandTypeData);
  BOOST_STATIC_ASSERT(number_of_ligand_mappings >= LIGAND_LAST);


  struct TypeMap {
    std::map<std::string, ProteinType> pmap;
    std::map<std::string, LigandType> lmap;
    TypeMap() {
      for (unsigned int i=0; i<number_of_protein_mappings; i++) {
        pmap[pat_data[i].name]= pat_data[i].type;
      }
      for (unsigned int j=0; j<number_of_ligand_mappings; j++) {
        lmap[lat_data[j].name]= lat_data[j].type;
      }
    }
    bool get_phas_element(String at) const {
      return pmap.find(at) != pmap.end();
    }
    bool get_lhas_element(String at) const {
      return lmap.find(at) != lmap.end();
    }
    ProteinType get_pmap_element(String at) const {
      //      std::cout << at << std::endl;
      /*      IMP_check(Pmap.find(at) != Pmap.end(),
              "Unknown AtomType in get_protein_element.",
              ValueException);
      */
      if(pmap.find(at) != pmap.end()){
        return pmap.find(at)->second;
      }
      else{
        return PROTEIN_INVALID;
      }
    }
    LigandType get_lmap_element(String at) const {
      //      std::cout << at << std::endl;
      /*      IMP_check(Lmap.find(at) != Lmap.end(),
              "Unknown AtomType in get_ligand_element.",
              ValueException);
      */
      if(lmap.find(at) != lmap.end()){
        return lmap.find(at)->second;
      }
      else{
        return LIGAND_INVALID;
      }
    }
  };

  const TypeMap &get_type_map() {
    static TypeMap t;
    return t;
  }

  IntKey get_protein_ligand_type_key() {
    static const IntKey ik("protein-ligand atom type");
    return ik;
  }
}




ProteinLigandAtomPairScore::ProteinLigandAtomPairScore(double threshold):
  table_(get_data_path("protein_ligand_score.lib")),
  threshold_(threshold){
  }
double ProteinLigandAtomPairScore::evaluate(const algebra::Vector3D &protein_v,
                                            int ptype,
                                            const algebra::Vector3D &ligand_v,
                                            int ltype) const {
   if (ptype== PROTEIN_INVALID || ltype == LIGAND_INVALID) return 0;
  double distance = algebra::distance(protein_v, ligand_v);
  if (distance <= threshold_){
    /*std::cout << "Evaluating pair " << ptype << " " << ltype << std::endl;
    std::cout << "distance is " << distance << " score is "
              <<  table_.get_score(ptype, ltype, distance)
              << std::endl;*/
    return table_.get_score(ptype, ltype, distance);
  } else {
    return 0;
  }
}


double ProteinLigandAtomPairScore::evaluate(const ParticlePair &pp,
                                            DerivativeAccumulator *da) const {
  IMP_USAGE_CHECK(!da, "The ProteinLigandAtomScore does not support "
                  << "derivatives");
  ProteinType pt= ProteinType(pp[0]->get_value(get_protein_ligand_type_key()));
  LigandType lt= LigandType(pp[1]->get_value(get_protein_ligand_type_key()));
  algebra::Vector3D pv(core::XYZ(pp[0]).get_coordinates()),
    lv(core::XYZ(pp[1]).get_coordinates());
  return evaluate(pv, pt, lv,lt);
}

void ProteinLigandAtomPairScore::do_show(std::ostream &out) const {
  out << "threshold: " << threshold_ << std::endl;
}




ProteinLigandRestraint::ProteinLigandRestraint(Hierarchy protein,
                                               Hierarchy ligand,
                                               double threshold):
  score_(new ProteinLigandAtomPairScore(threshold)),
  protein_(protein), ligand_(ligand){
  add_protein_ligand_score_data(protein_);
  add_protein_ligand_score_data(ligand_);
  IMP_IF_CHECK(USAGE) {
    HierarchiesTemp pr= get_by_type(protein_, RESIDUE_TYPE);
    for (unsigned int i=0; i< pr.size(); ++i) {
      IMP_USAGE_CHECK(!get_is_heterogen(pr[i]),
                      "Some of protein is actually a heterogen "
                      <<  pr[i]);
    }
    HierarchiesTemp lr= get_by_type(ligand_, RESIDUE_TYPE);
    for (unsigned int i=0; i< lr.size(); ++i) {
      IMP_USAGE_CHECK(get_is_heterogen(lr[i]),
                      "Some of ligand is actually protein "
                      <<  lr[i]);
    }
  }
}


double ProteinLigandRestraint
::unprotected_evaluate(DerivativeAccumulator *accum) const {
  IMP_USAGE_CHECK(!accum,
                  "ProteinLigandRestraint does not support derivatives");
  IntKey k= get_protein_ligand_type_key();
  algebra::Vector3Ds pvs, lvs;
  std::vector<ProteinType> pts;
  std::vector<LigandType> lts;
  HierarchiesTemp pas(get_by_type(protein_.get_decorator(), ATOM_TYPE));
  HierarchiesTemp las(get_by_type(ligand_.get_decorator(), ATOM_TYPE));
  pvs.resize(pas.size());
  pts.resize(pas.size());
  for (unsigned int i=0; i< pas.size(); ++i) {
    pvs[i]= core::XYZ(pas[i]).get_coordinates();
    pts[i]= ProteinType(pas[i]->get_value(k));
  }
  lvs.resize(las.size());
  lts.resize(las.size());
  for (unsigned int i=0; i< las.size(); ++i) {
    lvs[i]= core::XYZ(las[i]).get_coordinates();
    lts[i]= LigandType(las[i]->get_value(k));
  }
  double score=0;
  for (unsigned int i=0; i< las.size(); ++i) {
    for (unsigned int j=0; j< pas.size(); ++j) {
      score+= score_->evaluate(pvs[j], pts[j], lvs[i], lts[i]);
    }
  }
  return score;
}

ContainersTemp ProteinLigandRestraint::get_input_containers() const {
  return ContainersTemp();
}
ParticlesList ProteinLigandRestraint::get_interacting_particles() const {
  ParticlesTemp ret(get_by_type(protein_.get_decorator(), ATOM_TYPE));
  ParticlesTemp retb(get_by_type(ligand_.get_decorator(), ATOM_TYPE));
  ret.insert(ret.end(), retb.begin(), retb.end());
  return ParticlesList(1, ret);
}
ParticlesTemp ProteinLigandRestraint::get_input_particles() const{
  ParticlesTemp ret(core::get_all_descendants(protein_.get_decorator()));
  ParticlesTemp retb(core::get_all_descendants(ligand_.get_decorator()));
  ret.insert(ret.end(), retb.begin(), retb.end());
  ret.push_back(protein_);
  ret.push_back(ligand_);
  return ret;
}

void ProteinLigandRestraint::do_show(std::ostream &out) const {
  out << *score_;
}


void add_protein_ligand_score_data(Atom atom) {
  int type;
  Residue rd= get_residue(atom);
  std::string atom_string=atom.get_atom_type().get_string();
  std::string residue_string= rd.get_residue_type().get_string();
  if (!get_is_heterogen(rd)) {
    std::string score_type = residue_string + '_' + atom_string;
    type= get_type_map().get_pmap_element(score_type);
  } else {
    if(atom_string[0] == 'O' || atom_string[0] == 'S') {
      if(internal::check_arbond(atom)) {
        std::string stratype (1, atom_string[0]);
        atom_string = stratype + ".ar";
      }
    }
    if(atom_string.find('.') != String::npos) {
      atom_string.erase(atom_string.find('.'), 1);
    }
    type= get_type_map().get_lmap_element(atom_string);
  }
  if (type==-1 && atom.get_element() != H) {
    IMP_LOG(VERBOSE, "Failed to find type for "
            << atom << " " << rd << std::endl);
  }
  atom->add_attribute(get_protein_ligand_type_key(), type);
}
void add_protein_ligand_score_data(Hierarchy h) {
  HierarchiesTemp atoms= get_by_type(h, ATOM_TYPE);
  for (unsigned int i= 0; i< atoms.size(); ++i) {
    add_protein_ligand_score_data(Atom(atoms[i]));
  }
}


IMPATOM_END_NAMESPACE

IMPATOM_BEGIN_INTERNAL_NAMESPACE
PMFTable::PMFTable(TextInput tin) {
  std::istream &in =tin;
  double bin;
  std::string line;
  std::getline(in, line);
  std::istringstream iss(line);
  iss >> bin;
  unsigned int np, nl;
  iss >> np >> nl;
  if (np != PROTEIN_LAST
      || nl != LIGAND_LAST) {
    IMP_THROW("Expected number of protein and ligand types not found. "
              << "Expected " << PROTEIN_LAST
              << " " << LIGAND_LAST
              << " but got " << np << " " << nl,
              IOException);
  }
  if (!iss) {
    IMP_THROW("Error reading bin size from line " << line,
              IOException);
  }
  {
    double test;
    iss >> test;
    if (iss) {
      IMP_THROW("Extra data found on bin size line " << line
                << " got " << test,
                IOException);
    }
  }
  inverse_bin_width_=1.0/bin;

  data_.resize(PROTEIN_LAST);
  int bins_read=-1;
  for(unsigned int i=0;i<data_.size();i++){
    data_[i].resize(LIGAND_LAST);
    for(unsigned int j=0;j<data_[i].size();j++){
      std::string line;
      std::getline(in, line);
      if (!in) {
        IMP_THROW("Error getting next line at " << i << " " << j,
                  IOException);
      }
      std::istringstream ins(line);
      //      std::cout << line << std::endl;
      //      if(j>1) exit(EXIT_FAILURE);
      int cur_bins_read=0;
      while(true) {
        double potentialvalue;
        ins >> potentialvalue;
        if (ins) {
          data_[i][j].push_back(potentialvalue);
          ++cur_bins_read;
        } else {
          break;
        }
      }
      if (bins_read != -1 && cur_bins_read != bins_read) {
        IMP_THROW("Read wrong number of bins from line: "
                  << line << "\nExpected " << bins_read
                  << " got " << cur_bins_read,
                  IOException);
      }
      bins_read= cur_bins_read;
      ins.clear();
    }
  }
  IMP_LOG(TERSE, "PMF table entries have "
          << bins_read << " bins" << std::endl);
}
IMPATOM_END_INTERNAL_NAMESPACE
