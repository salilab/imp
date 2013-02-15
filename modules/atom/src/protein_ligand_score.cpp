/**
 *  \file protein_ligand_score.cpp   \brief A class for reading mol2 files
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/protein_ligand_score.h>
#include <IMP/atom/internal/mol2.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/core/XYZ.h>
#include <IMP/container/CloseBipartitePairContainer.h>
#include <IMP/container/ListSingletonContainer.h>

IMPATOM_BEGIN_NAMESPACE

namespace {

  namespace data {

#define PROTEIN_TYPE(NameKey) const ProteinLigandType \
    NameKey(ProteinLigandType::add_key(#NameKey));
#define PROTEIN_TYPE_ALIAS(Name, Key) const ProteinLigandType Name\
  (ProteinLigandType::add_alias(ProteinLigandType(#Key), #Name));

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
    PROTEIN_TYPE(ARG_NH)
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
    PROTEIN_TYPE(ASP_OD)
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
    PROTEIN_TYPE(GLU_OE)
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
    PROTEIN_TYPE(ILE_CD)
    PROTEIN_TYPE_ALIAS(ILE_CD1, ILE_CD)
    PROTEIN_TYPE(LEU_N)
    PROTEIN_TYPE(LEU_CA)
    PROTEIN_TYPE(LEU_C)
    PROTEIN_TYPE(LEU_O)
    PROTEIN_TYPE(LEU_CB)
    PROTEIN_TYPE(LEU_CG)
    PROTEIN_TYPE(LEU_CD)
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
    PROTEIN_TYPE(PHE_CD)
    PROTEIN_TYPE_ALIAS(PHE_CD1, PHE_CD)
    PROTEIN_TYPE_ALIAS(PHE_CD2, PHE_CD)
    PROTEIN_TYPE(PHE_CE)
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
    PROTEIN_TYPE(THR_OG1)
    PROTEIN_TYPE(THR_CG2)
    PROTEIN_TYPE(TRP_N)
    PROTEIN_TYPE(TRP_CA)
    PROTEIN_TYPE(TRP_C)
    PROTEIN_TYPE(TRP_O)
    PROTEIN_TYPE(TRP_CB)
    PROTEIN_TYPE(TRP_CG)
    PROTEIN_TYPE(TRP_CD1)
    PROTEIN_TYPE(TRP_CD2)
    PROTEIN_TYPE(TRP_NE1)
    PROTEIN_TYPE(TRP_CE2)
    PROTEIN_TYPE(TRP_CE3)
    PROTEIN_TYPE(TRP_CZ2)
    PROTEIN_TYPE(TRP_CZ3)
    PROTEIN_TYPE(TRP_CH2)
    PROTEIN_TYPE(TYR_N)
    PROTEIN_TYPE(TYR_CA)
    PROTEIN_TYPE(TYR_C)
    PROTEIN_TYPE(TYR_O)
    PROTEIN_TYPE(TYR_CB)
    PROTEIN_TYPE(TYR_CG)
    PROTEIN_TYPE(TYR_CD)
    PROTEIN_TYPE_ALIAS(TYR_CD1, TYR_CD)
    PROTEIN_TYPE_ALIAS(TYR_CD2, TYR_CD)
    PROTEIN_TYPE(TYR_CE)
    PROTEIN_TYPE_ALIAS(TYR_CE1, TYR_CE)
    PROTEIN_TYPE_ALIAS(TYR_CE2, TYR_CE)
    PROTEIN_TYPE(TYR_CZ)
    PROTEIN_TYPE(TYR_OH)
    PROTEIN_TYPE(VAL_N)
    PROTEIN_TYPE(VAL_CA)
    PROTEIN_TYPE(VAL_C)
    PROTEIN_TYPE(VAL_O)
    PROTEIN_TYPE(VAL_CB)
    PROTEIN_TYPE(VAL_CG)
    PROTEIN_TYPE_ALIAS(VAL_CG1, VAL_CG)
    PROTEIN_TYPE_ALIAS(VAL_CG2, VAL_CG)
    PROTEIN_TYPE(LAST_PROTEIN_TYPE)

#define LIGAND_TYPE(NameKey) \
    const ProteinLigandType NameKey(ProteinLigandType::add_key(#NameKey));

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
    LIGAND_TYPE(I)
    LIGAND_TYPE(LAST_LIGAND_TYPE)
}

  IntKey get_protein_ligand_type_key() {
    static const IntKey ik("protein-ligand atom type");
    return ik;
  }

  const int ni= data::LAST_PROTEIN_TYPE.get_index();
  /*const int nj= data::LAST_LIGAND_TYPE.get_index()
    - data::LAST_PROTEIN_TYPE.get_index()-1;*/
}




ProteinLigandAtomPairScore::ProteinLigandAtomPairScore(double threshold):
  P(get_protein_ligand_type_key(), threshold,
    get_data_path("protein_ligand_rank_score.lib"),
    ni+1), threshold_(threshold){
  }

ProteinLigandAtomPairScore::ProteinLigandAtomPairScore(double threshold,
                                                       base::TextInput file):
  P(get_protein_ligand_type_key(), threshold, file, ni+1),
  threshold_(threshold){
  }

void ProteinLigandRestraint::initialize(Hierarchy protein,
                                        Hierarchy ligand) {
  add_protein_ligand_score_data(protein);
  add_protein_ligand_score_data(ligand);
  IMP_IF_CHECK(USAGE) {
    Hierarchies pr= get_by_type(protein, RESIDUE_TYPE);
    for (unsigned int i=0; i< pr.size(); ++i) {
      IMP_USAGE_CHECK(!get_is_heterogen(pr[i]),
                      "Some of protein is actually a heterogen "
                      <<  pr[i]);
    }
    Hierarchies lr= get_by_type(ligand, RESIDUE_TYPE);
    for (unsigned int i=0; i< lr.size(); ++i) {
      IMP_USAGE_CHECK(get_is_heterogen(lr[i]),
                      "Some of ligand is actually protein "
                      <<  lr[i]);
    }
  }
}


namespace {
  PairScore* create_pair_score(double threshold, base::TextInput data) {
    return new ProteinLigandAtomPairScore(threshold, data);
  }
  PairScore* create_pair_score(double threshold) {
    return new ProteinLigandAtomPairScore(threshold);
  }
  PairContainer *create_pair_container(Hierarchy a,
                                       Hierarchy b,
                                       double threshold) {
    ParticlesTemp aa= get_by_type(a, ATOM_TYPE);
    ParticlesTemp ba= get_by_type(b, ATOM_TYPE);
    IMP_NEW(container::ListSingletonContainer, lsca, (aa));
    IMP_NEW(container::ListSingletonContainer, lscb, (ba));
    IMP_NEW(container::CloseBipartitePairContainer,
            ret, (lsca, lscb, threshold));
    return ret.release();
  }
}

ProteinLigandRestraint::ProteinLigandRestraint(Hierarchy protein,
                                               Hierarchy ligand,
                                               double threshold):
  container::PairsRestraint(create_pair_score(threshold),
                            create_pair_container(protein, ligand, threshold)){
  initialize(protein, ligand);
}

ProteinLigandRestraint::ProteinLigandRestraint(Hierarchy protein,
                                               Hierarchy ligand,
                                               double threshold,
                                               base::TextInput data):
  container::PairsRestraint(create_pair_score(threshold, data),
                            create_pair_container(protein, ligand, threshold)) {
  initialize(protein, ligand);
}


namespace {
void add_protein_ligand_score_data(Atom atom) {
  int type;
  Residue rd= get_residue(atom);
  std::string atom_string=atom.get_atom_type().get_string();
  std::string residue_string= rd.get_residue_type().get_string();
  if (!get_is_heterogen(rd)) {
    std::string score_type = residue_string + '_' + atom_string;
    if (!ProteinLigandType::get_key_exists(score_type)) {
      type=-1;
    } else {
      /*std::cout << "Type for " << atom << " is "
        << ProteinType(score_type) << std::endl;*/
      type= ProteinLigandType(score_type).get_index();
    }
  } else {
    std::string nm= internal::get_mol2_name(atom);
    if(nm.find('.') != String::npos) {
      nm.erase(nm.find('.'), 1);
    }
    if (!ProteinLigandType::get_key_exists(nm)) {
      type=-1;
    } else {
      //std::cout << "Type for " << atom << " is "
      //<< LigandType(nm) << std::endl;
      type= ProteinLigandType(nm).get_index();
    }
  }
  if (type==-1 && atom.get_element() != H) {
    IMP_LOG_TERSE( "Failed to find type for "
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
}

void add_protein_ligand_score_data(Hierarchy h) {
  Hierarchies atoms= get_by_type(h, ATOM_TYPE);
  for (unsigned int i= 0; i< atoms.size(); ++i) {
    add_protein_ligand_score_data(Atom(atoms[i]));
  }
}


IMPATOM_END_NAMESPACE
