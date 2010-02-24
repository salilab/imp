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
#define PROTEIN_TYPE(NameKey) {#NameKey, NameKey},
#define PROTEIN_TYPE_ALIAS(Name, Key) {#Name, Key},
  ProteinTypeData pat_data[]= {
    PROTEIN_TYPE(ALA_N)
    PROTEIN_TYPE(ALA_CA)
    PROTEIN_TYPE(ALA_C)
    PROTEIN_TYPE(ALA_O)
    PROTEIN_TYPE(ALA_CB)
    PROTEIN_TYPE(ARG_N)
    PROTEIN_TYPE(ARG_CA)
    PROTEIN_TYPE(ARG_C)
    PROTEIN_TYPE(ARG_O)
    PROTEIN_TYPE(ARG_CB)
    PROTEIN_TYPE(ARG_CG)
    PROTEIN_TYPE(ARG_CD)
    PROTEIN_TYPE(ARG_NE)
    PROTEIN_TYPE(ARG_CZ)
    PROTEIN_TYPE_ALIAS(ARG_NH1, ARG_NH)
    PROTEIN_TYPE_ALIAS(ARG_NH2, ARG_NH)
    PROTEIN_TYPE(ASN_N)
    PROTEIN_TYPE(ASN_CA)
    PROTEIN_TYPE(ASN_C)
    PROTEIN_TYPE(ASN_O)
    PROTEIN_TYPE(ASN_CB)
    PROTEIN_TYPE(ASN_CG)
    PROTEIN_TYPE(ASN_OD1)
    PROTEIN_TYPE(ASN_ND2)
    PROTEIN_TYPE(ASP_N)
    PROTEIN_TYPE(ASP_CA)
    PROTEIN_TYPE(ASP_C)
    PROTEIN_TYPE(ASP_O)
    PROTEIN_TYPE(ASP_CB)
    PROTEIN_TYPE(ASP_CG)
    PROTEIN_TYPE_ALIAS(ASP_OD1, ASP_OD)
    PROTEIN_TYPE_ALIAS(ASP_OD2, ASP_OD)
    PROTEIN_TYPE(CYS_N)
    PROTEIN_TYPE(CYS_CA)
    PROTEIN_TYPE(CYS_C)
    PROTEIN_TYPE(CYS_O)
    PROTEIN_TYPE(CYS_CB)
    PROTEIN_TYPE(CYS_SG)
    PROTEIN_TYPE(GLN_N)
    PROTEIN_TYPE(GLN_CA)
    PROTEIN_TYPE(GLN_C)
    PROTEIN_TYPE(GLN_O)
    PROTEIN_TYPE(GLN_CB)
    PROTEIN_TYPE(GLN_CG)
    PROTEIN_TYPE(GLN_CD)
    PROTEIN_TYPE(GLN_OE1)
    PROTEIN_TYPE(GLN_NE2)
    PROTEIN_TYPE(GLU_N)
    PROTEIN_TYPE(GLU_CA)
    PROTEIN_TYPE(GLU_C)
    PROTEIN_TYPE(GLU_O)
    PROTEIN_TYPE(GLU_CB)
    PROTEIN_TYPE(GLU_CG)
    PROTEIN_TYPE(GLU_CD)
    PROTEIN_TYPE_ALIAS(GLU_OE1, GLU_OE)
    PROTEIN_TYPE_ALIAS(GLU_OE2, GLU_OE)
    PROTEIN_TYPE(GLY_N)
    PROTEIN_TYPE(GLY_CA)
    PROTEIN_TYPE(GLY_C)
    PROTEIN_TYPE(GLY_O)
    PROTEIN_TYPE(HIS_N)
    PROTEIN_TYPE(HIS_CA)
    PROTEIN_TYPE(HIS_C)
    PROTEIN_TYPE(HIS_O)
    PROTEIN_TYPE(HIS_CB)
    PROTEIN_TYPE(HIS_CG)
    PROTEIN_TYPE(HIS_ND1)
    PROTEIN_TYPE(HIS_CD2)
    PROTEIN_TYPE(HIS_CE1)
    PROTEIN_TYPE(HIS_NE2)
    PROTEIN_TYPE(ILE_N)
    PROTEIN_TYPE(ILE_CA)
    PROTEIN_TYPE(ILE_C)
    PROTEIN_TYPE(ILE_O)
    PROTEIN_TYPE(ILE_CB)
    PROTEIN_TYPE(ILE_CG1)
    PROTEIN_TYPE(ILE_CG2)
    PROTEIN_TYPE(ILE_CD1)
    PROTEIN_TYPE(LEU_N)
    PROTEIN_TYPE(LEU_CA)
    PROTEIN_TYPE(LEU_C)
    PROTEIN_TYPE(LEU_O)
    PROTEIN_TYPE(LEU_CB)
    PROTEIN_TYPE(LEU_CG)
    PROTEIN_TYPE_ALIAS(LEU_CD1, LEU_CD)
    PROTEIN_TYPE_ALIAS(LEU_CD2, LEU_CD)
    PROTEIN_TYPE(LYS_N)
    PROTEIN_TYPE(LYS_CA)
    PROTEIN_TYPE(LYS_C)
    PROTEIN_TYPE(LYS_O)
    PROTEIN_TYPE(LYS_CB)
    PROTEIN_TYPE(LYS_CG)
    PROTEIN_TYPE(LYS_CD)
    PROTEIN_TYPE(LYS_CE)
    PROTEIN_TYPE(LYS_NZ)
    PROTEIN_TYPE(MET_N)
    PROTEIN_TYPE(MET_CA)
    PROTEIN_TYPE(MET_C)
    PROTEIN_TYPE(MET_O)
    PROTEIN_TYPE(MET_CB)
    PROTEIN_TYPE(MET_CG)
    PROTEIN_TYPE(MET_SD)
    PROTEIN_TYPE(MET_CE)
    PROTEIN_TYPE(PHE_N)
    PROTEIN_TYPE(PHE_CA)
    PROTEIN_TYPE(PHE_C)
    PROTEIN_TYPE(PHE_O)
    PROTEIN_TYPE(PHE_CB)
    PROTEIN_TYPE(PHE_CG)
    PROTEIN_TYPE_ALIAS(PHE_CD1, PHE_CD)
    PROTEIN_TYPE_ALIAS(PHE_CD2, PHE_CD)
    PROTEIN_TYPE_ALIAS(PHE_CE1, PHE_CE)
    PROTEIN_TYPE_ALIAS(PHE_CE2, PHE_CE)
    PROTEIN_TYPE(PHE_CZ)
    PROTEIN_TYPE(PRO_N)
    PROTEIN_TYPE(PRO_CA)
    PROTEIN_TYPE(PRO_C)
    PROTEIN_TYPE(PRO_O)
    PROTEIN_TYPE(PRO_CB)
    PROTEIN_TYPE(PRO_CG)
    PROTEIN_TYPE(PRO_CD)
    PROTEIN_TYPE(SER_N)
    PROTEIN_TYPE(SER_CA)
    PROTEIN_TYPE(SER_C)
    PROTEIN_TYPE(SER_O)
    PROTEIN_TYPE(SER_CB)
    PROTEIN_TYPE(SER_OG)
    PROTEIN_TYPE(THR_N)
    PROTEIN_TYPE(THR_CA)
    PROTEIN_TYPE(THR_C)
    PROTEIN_TYPE(THR_O)
    PROTEIN_TYPE(THR_CB)
    PROTEIN_TYPE_ALIAS(THR_OG1, THR_OG1)
    PROTEIN_TYPE_ALIAS(THR_CG2, THR_CG2)
    PROTEIN_TYPE(TRP_N)
    PROTEIN_TYPE(TRP_CA)
    PROTEIN_TYPE(TRP_C)
    PROTEIN_TYPE(TRP_O)
    PROTEIN_TYPE(TRP_CB)
    PROTEIN_TYPE(TRP_CG)
    PROTEIN_TYPE_ALIAS(TRP_CD1, TRP_CD1)
    PROTEIN_TYPE_ALIAS(TRP_CD2, TRP_CD2)
    PROTEIN_TYPE_ALIAS(TRP_NE1, TRP_NE1)
    PROTEIN_TYPE_ALIAS(TRP_CE2, TRP_CE2)
    PROTEIN_TYPE_ALIAS(TRP_CE3, TRP_CE3)
    PROTEIN_TYPE_ALIAS(TRP_CZ2, TRP_CZ2)
    PROTEIN_TYPE_ALIAS(TRP_CZ3, TRP_CZ3)
    PROTEIN_TYPE_ALIAS(TRP_CH2, TRP_CH2)
    PROTEIN_TYPE(TYR_N)
    PROTEIN_TYPE(TYR_CA)
    PROTEIN_TYPE(TYR_C)
    PROTEIN_TYPE(TYR_O)
    PROTEIN_TYPE(TYR_CB)
    PROTEIN_TYPE(TYR_CG)
    PROTEIN_TYPE_ALIAS(TYR_CD1, TYR_CD)
    PROTEIN_TYPE_ALIAS(TYR_CD2, TYR_CD)
    PROTEIN_TYPE_ALIAS(TYR_CE1, TYR_CE)
    PROTEIN_TYPE_ALIAS(TYR_CE2, TYR_CE)
    PROTEIN_TYPE(TYR_CZ)
    PROTEIN_TYPE(TYR_OH)
    PROTEIN_TYPE(VAL_N)
    PROTEIN_TYPE(VAL_CA)
    PROTEIN_TYPE(VAL_C)
    PROTEIN_TYPE(VAL_O)
    PROTEIN_TYPE(VAL_CB)
    PROTEIN_TYPE_ALIAS(VAL_CG1, VAL_CG)
    PROTEIN_TYPE_ALIAS(VAL_CG2, VAL_CG)};

  static const unsigned int number_of_protein_mappings
  =sizeof(pat_data)/sizeof(ProteinTypeData);
  BOOST_STATIC_ASSERT(number_of_protein_mappings >= PROTEIN_LAST);

  struct LigandTypeData {
    std::string name;
    LigandType type;
  };
#define LIGAND_TYPE(NameKey) {#NameKey, NameKey},

  LigandTypeData lat_data[]= {
    LIGAND_TYPE(C1)
    LIGAND_TYPE(C2)
    LIGAND_TYPE(C3)
    LIGAND_TYPE(Car)
    LIGAND_TYPE(Ccat)
    LIGAND_TYPE(N1)
    LIGAND_TYPE(N2)
    LIGAND_TYPE(N3)
    LIGAND_TYPE(N4)
    LIGAND_TYPE(Nar)
    LIGAND_TYPE(Nam)
    LIGAND_TYPE(Npl3)
    LIGAND_TYPE(O2)
    LIGAND_TYPE(O3)
    LIGAND_TYPE(Oco2)
    LIGAND_TYPE(Oar)
    LIGAND_TYPE(S2)
    LIGAND_TYPE(S3)
    LIGAND_TYPE(So)
    LIGAND_TYPE(So2)
    LIGAND_TYPE(Sar)
    LIGAND_TYPE(P3)
    LIGAND_TYPE(F)
    LIGAND_TYPE(Cl)
    LIGAND_TYPE(Br)
    LIGAND_TYPE(I)};
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
double ProteinLigandAtomPairScore
::evaluate(const algebra::VectorD<3> &protein_v,
           int ptype,
           const algebra::VectorD<3> &ligand_v,
           int ltype,
           core::XYZ pxyz, core::XYZ lxyz,
           DerivativeAccumulator *da) const {
   if (ptype== PROTEIN_INVALID || ltype == LIGAND_INVALID) return 0;
   double distance = algebra::get_distance(protein_v, ligand_v);
   if (distance >= threshold_ || distance < 0.001) {
     return 0;
   }
   if (!da) {
       /*std::cout << "Evaluating pair " << ptype << " " << ltype << std::endl;
         std::cout << "distance is " << distance << " score is "
         <<  table_.get_score(ptype, ltype, distance)
         << std::endl;*/
     return table_.get_score(ptype, ltype, distance);
   } else {
     DerivativePair dp= table_.get_score_with_derivative(ptype,
                                                         ltype, distance);
     algebra::VectorD<3> diff= protein_v-ligand_v;
     algebra::VectorD<3> norm= diff.get_unit_vector();
     pxyz.add_to_derivatives(dp.second*norm, *da);
     lxyz.add_to_derivatives(-dp.second*norm, *da);
     return dp.first;
   }
}


double ProteinLigandAtomPairScore::evaluate(const ParticlePair &pp,
                                            DerivativeAccumulator *da) const {
  ProteinType pt= ProteinType(pp[0]->get_value(get_protein_ligand_type_key()));
  LigandType lt= LigandType(pp[1]->get_value(get_protein_ligand_type_key()));
  core::XYZ pxyz(pp[0]);
  core::XYZ lxyz(pp[1]);
  algebra::VectorD<3> pv(pxyz.get_coordinates()),
    lv(lxyz.get_coordinates());
  return evaluate(pv, pt, lv,lt, pxyz, lxyz, da);
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
  IntKey k= get_protein_ligand_type_key();
  std::vector<algebra::VectorD<3> > pvs, lvs;
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
      score+= score_->evaluate(pvs[j], pts[j], lvs[i], lts[i],
                               core::XYZ(pas[j]), core::XYZ(las[i]),
                               accum);
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
  if (atom->has_attribute(get_protein_ligand_type_key())) {
    IMP_USAGE_CHECK(atom->get_value(get_protein_ligand_type_key()) == type,
                    "Atom " << atom << " already has protein-ligand score type "
                    << "but it is not correct. Got "
                    << atom->get_value(get_protein_ligand_type_key())
                    << " expected " << type);
  } else {
    atom->add_attribute(get_protein_ligand_type_key(), type);
  }
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
  bin_width_=bin;
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
      Floats data;
      while(true) {
        double potentialvalue;
        ins >> potentialvalue;
        if (ins) {
          data.push_back(potentialvalue);
          ++cur_bins_read;
        } else {
          break;
        }
      }
      data_[i][j]= core::internal::RawOpenCubicSpline(data, bin_width_,
                                                      inverse_bin_width_);
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
  max_= bin_width_*bins_read;
  IMP_LOG(TERSE, "PMF table entries have "
          << bins_read << " bins with width " << bin_width_ << std::endl);
}
IMPATOM_END_INTERNAL_NAMESPACE
