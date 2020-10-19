/**
 *  \file dihedrals.cpp   \brief Helpers to extract dihedral information.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/dihedrals.h>

IMPATOM_BEGIN_NAMESPACE

namespace {
class DihedralAtom {
 public:
  enum WhichResidue {
    THIS,
    PREV,
    NEXT
  };

 private:
  WhichResidue residue_;
  AtomType type_;

 public:
  DihedralAtom(WhichResidue residue, AtomType type)
      : residue_(residue), type_(type) {}

  Atom get_atom(Residue rd) const {
    if (residue_ == PREV) {
      Hierarchy h = get_previous_residue(rd);
      if (h && Residue::get_is_setup(h)) {
        return IMP::atom::get_atom(Residue(h), type_);
      }
    } else if (residue_ == NEXT) {
      Hierarchy h = get_next_residue(rd);
      if (h && Residue::get_is_setup(h)) {
        return IMP::atom::get_atom(Residue(h), type_);
      }
    } else {
      return IMP::atom::get_atom(rd, type_);
    }
    return Atom();
  }
};


Atoms get_dihedral_atoms(Residue rd, const DihedralAtom *dihedral) {
  Atoms atoms;
  for (int i = 0; i < 4; ++i) {
    Atom a = dihedral[i].get_atom(rd);
    if (!a) {
      return Atoms();
    } else {
      atoms.push_back(a);
    }
  }
  return atoms;
}
}

Atoms get_phi_dihedral_atoms(Residue rd) {
  static DihedralAtom dihedral[4] = {DihedralAtom(DihedralAtom::PREV, AT_C),
                                     DihedralAtom(DihedralAtom::THIS, AT_N),
                                     DihedralAtom(DihedralAtom::THIS, AT_CA),
                                     DihedralAtom(DihedralAtom::THIS, AT_C)};
  return get_dihedral_atoms(rd, dihedral);
}

Atoms get_psi_dihedral_atoms(Residue rd) {
  static DihedralAtom dihedral[4] = {DihedralAtom(DihedralAtom::THIS, AT_N),
                                     DihedralAtom(DihedralAtom::THIS, AT_CA),
                                     DihedralAtom(DihedralAtom::THIS, AT_C),
                                     DihedralAtom(DihedralAtom::NEXT, AT_N)};
  return get_dihedral_atoms(rd, dihedral);
}

Atoms get_omega_dihedral_atoms(Residue rd) {
  static DihedralAtom dihedral[4] = {DihedralAtom(DihedralAtom::THIS, AT_CA),
                                     DihedralAtom(DihedralAtom::THIS, AT_C),
                                     DihedralAtom(DihedralAtom::NEXT, AT_N),
                                     DihedralAtom(DihedralAtom::NEXT, AT_CA)};
  return get_dihedral_atoms(rd, dihedral);
}

Vector<AtomTypes> get_chi_dihedral_atom_types(ResidueType rt) {
  /*
  if (rt = "VAL") {
    std::cout << "RT = str(VAL)" << std::endl;
  } else if (rt = atom::VAL) {
    std::cout << "RT = atom::VAL" << std::endl;
  } else if (rt = VAL) {
    std::cout << "RT = atom::VAL" << std::endl;
  } else {
    std::cout << "This is not any of those" << std::endl;
  }
  */
  static std::map<ResidueType, Vector<AtomTypes> > chi_dihedral_atom_types;

  AtomTypes chi1_cg;
  chi1_cg.push_back(AT_N);
  chi1_cg.push_back(AT_CA);
  chi1_cg.push_back(AT_CB);
  chi1_cg.push_back(AT_CG);

  AtomTypes chi1_cg1;
  chi1_cg1.push_back(AT_N);
  chi1_cg1.push_back(AT_CA);
  chi1_cg1.push_back(AT_CB);
  chi1_cg1.push_back(AT_CG1);

  AtomTypes chi1_og;
  chi1_og.push_back(AT_N);
  chi1_og.push_back(AT_CA);
  chi1_og.push_back(AT_CB);
  chi1_og.push_back(AT_OG);

  AtomTypes chi1_og1;
  chi1_og1.push_back(AT_N);
  chi1_og1.push_back(AT_CA);
  chi1_og1.push_back(AT_CB);
  chi1_og1.push_back(AT_OG1);

  AtomTypes chi1_sg;
  chi1_sg.push_back(AT_N);
  chi1_sg.push_back(AT_CA);
  chi1_sg.push_back(AT_CB);
  chi1_sg.push_back(AT_SG);

  AtomTypes chi2_cd;
  chi2_cd.push_back(AT_CA);
  chi2_cd.push_back(AT_CB);
  chi2_cd.push_back(AT_CG);
  chi2_cd.push_back(AT_CD);

  AtomTypes chi2_cg1_cd1;
  chi2_cg1_cd1.push_back(AT_CA);
  chi2_cg1_cd1.push_back(AT_CB);
  chi2_cg1_cd1.push_back(AT_CG1);
  chi2_cg1_cd1.push_back(AT_CD1);

  AtomTypes chi2_cd1;
  chi2_cd1.push_back(AT_CA);
  chi2_cd1.push_back(AT_CB);
  chi2_cd1.push_back(AT_CG);
  chi2_cd1.push_back(AT_CD1);

  AtomTypes chi2_od1;
  chi2_od1.push_back(AT_CA);
  chi2_od1.push_back(AT_CB);
  chi2_od1.push_back(AT_CG);
  chi2_od1.push_back(AT_OD1);

  AtomTypes chi2_nd1;
  chi2_nd1.push_back(AT_CA);
  chi2_nd1.push_back(AT_CB);
  chi2_nd1.push_back(AT_CG);
  chi2_nd1.push_back(AT_ND1);

  AtomTypes chi2_sd;
  chi2_sd.push_back(AT_CA);
  chi2_sd.push_back(AT_CB);
  chi2_sd.push_back(AT_CG);
  chi2_sd.push_back(AT_SD);

  AtomTypes chi3_sd_ce;
  chi3_sd_ce.push_back(AT_CB);
  chi3_sd_ce.push_back(AT_CG);
  chi3_sd_ce.push_back(AT_SD);
  chi3_sd_ce.push_back(AT_CE);

  AtomTypes chi3_ce;
  chi3_ce.push_back(AT_CB);
  chi3_ce.push_back(AT_CG);
  chi3_ce.push_back(AT_CD);
  chi3_ce.push_back(AT_CE);
  
  AtomTypes chi3_ne;
  chi3_ne.push_back(AT_CB);
  chi3_ne.push_back(AT_CG);
  chi3_ne.push_back(AT_CD);
  chi3_ne.push_back(AT_NE);
  
  AtomTypes chi3_n;
  chi3_n.push_back(AT_CB);
  chi3_n.push_back(AT_CG);
  chi3_n.push_back(AT_CD);
  chi3_n.push_back(AT_N);

  AtomTypes chi3_oe1;
  chi3_oe1.push_back(AT_CB);
  chi3_oe1.push_back(AT_CG);
  chi3_oe1.push_back(AT_CD);
  chi3_oe1.push_back(AT_OE1);

  AtomTypes chi4_cz;
  chi4_cz.push_back(AT_CG);
  chi4_cz.push_back(AT_CD);
  chi4_cz.push_back(AT_NE);
  chi4_cz.push_back(AT_CZ);

  AtomTypes chi4_nz;
  chi4_nz.push_back(AT_CG);
  chi4_nz.push_back(AT_CD);
  chi4_nz.push_back(AT_CE);
  chi4_nz.push_back(AT_NZ);

  AtomTypes chi5_nh1;
  chi5_nh1.push_back(AT_CD);
  chi5_nh1.push_back(AT_NE);
  chi5_nh1.push_back(AT_CZ);
  chi5_nh1.push_back(AT_NH1);

  Vector<AtomTypes> ala;

  Vector<AtomTypes> cys;
  cys.push_back(chi1_sg);

  Vector<AtomTypes> asp;
  asp.push_back(chi1_cg);
  asp.push_back(chi2_od1);

  Vector<AtomTypes> glu;
  glu.push_back(chi1_cg);
  glu.push_back(chi2_cd);
  glu.push_back(chi3_oe1);

  Vector<AtomTypes> phe;
  phe.push_back(chi1_cg);
  phe.push_back(chi2_cd1);

  Vector<AtomTypes> gly;

  Vector<AtomTypes> his;
  his.push_back(chi1_cg);
  his.push_back(chi2_nd1);

  Vector<AtomTypes> ile;
  ile.push_back(chi1_cg1);
  ile.push_back(chi2_cg1_cd1);

  Vector<AtomTypes> lys;
  lys.push_back(chi1_cg);
  lys.push_back(chi2_cd);
  lys.push_back(chi3_ce);
  lys.push_back(chi4_nz);

  Vector<AtomTypes> leu;
  leu.push_back(chi1_cg);
  leu.push_back(chi2_cd1);

  Vector<AtomTypes> met;
  met.push_back(chi1_cg);
  met.push_back(chi2_sd);
  met.push_back(chi3_sd_ce);

  Vector<AtomTypes> asn;
  asn.push_back(chi1_cg);
  asn.push_back(chi2_od1);

  Vector<AtomTypes> pro;
  pro.push_back(chi1_cg);
  pro.push_back(chi2_cd);
  pro.push_back(chi3_n);

  Vector<AtomTypes> gln;
  gln.push_back(chi1_cg);
  gln.push_back(chi2_cd);

  Vector<AtomTypes> arg;
  arg.push_back(chi1_cg);
  arg.push_back(chi2_cd);
  arg.push_back(chi3_ne);
  arg.push_back(chi4_cz);
  //arg.push_back(chi5_nh1);

  Vector<AtomTypes> ser;
  ser.push_back(chi1_og);

  Vector<AtomTypes> thr;
  thr.push_back(chi1_og1);

  Vector<AtomTypes> val;
  val.push_back(chi1_cg1);

  Vector<AtomTypes> trp;
  trp.push_back(chi1_cg);
  trp.push_back(chi2_cd1);

  Vector<AtomTypes> tyr;
  tyr.push_back(chi1_cg);
  tyr.push_back(chi2_cd1);

  Vector<AtomTypes> unk;
  
  if(chi_dihedral_atom_types.size() == 0) {          
          chi_dihedral_atom_types[atom::ALA] = ala;
          chi_dihedral_atom_types[atom::ARG] = arg;
          chi_dihedral_atom_types[atom::ASP] = asp;
          chi_dihedral_atom_types[atom::ASN] = asn;
          chi_dihedral_atom_types[atom::CYS] = cys;
          chi_dihedral_atom_types[atom::GLN] = gln;
          chi_dihedral_atom_types[atom::GLU] = glu;
          chi_dihedral_atom_types[atom::GLY] = gly;
          chi_dihedral_atom_types[atom::HIS] = his;
          chi_dihedral_atom_types[atom::ILE] = ile;
          chi_dihedral_atom_types[atom::LEU] = leu;
          chi_dihedral_atom_types[atom::LYS] = lys;
          chi_dihedral_atom_types[atom::MET] = met;
          chi_dihedral_atom_types[atom::PHE] = phe;
          chi_dihedral_atom_types[atom::PRO] = pro;
          chi_dihedral_atom_types[atom::SER] = ser;
          chi_dihedral_atom_types[atom::THR] = thr;
          chi_dihedral_atom_types[atom::TYR] = tyr;
          chi_dihedral_atom_types[atom::TRP] = trp;
          chi_dihedral_atom_types[atom::VAL] = val;
          chi_dihedral_atom_types[atom::UNK] = unk;
  }

if( chi_dihedral_atom_types[rt].size() > 0) {
  return chi_dihedral_atom_types[rt];
} else {
  return unk;
}

}
/*
AtomTypesSet get_chi_dihedral_atom_types(ResidueType rt) {
  std::map<ResidueType, AtomTypesSet> const chi_dihedral_atom_types;
  if(chi_dihedral_atom_types.size() == 0) {          
          chi_dihedral_atom_types[atom::ALA] = {{}};
          chi_dihedral_atom_types[atom::ARG] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_CD},
                                                {AT_CB, AT_CG, AT_CD, AT_NE},
                                                {AT_CG, AT_CD, AT_NE, AT_CZ},
                                                {AT_CD, AT_NE, AT_CZ, AT_NH1}};
          chi_dihedral_atom_types[atom::ASP] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_OD1}};
          chi_dihedral_atom_types[atom::ASN] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_OD1}};
          chi_dihedral_atom_types[atom::CYS] = {{AT_N, AT_CA, AT_CB, AT_SG}};
          chi_dihedral_atom_types[atom::GLN] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_CD},
                                                {AT_CB, AT_CG, AT_CD, AT_OE1}}; 
          chi_dihedral_atom_types[atom::GLU] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_CD},
                                                {AT_CB, AT_CG, AT_CD, AT_OE1}};
          chi_dihedral_atom_types[atom::GLY] = {{}};
          chi_dihedral_atom_types[atom::HIS] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_ND1}}; 
          chi_dihedral_atom_types[atom::ILE] = {{AT_N, AT_CA, AT_CB, AT_CG1},
                                                {AT_CA, AT_CB, AT_CG1, AT_CD}}; 
          chi_dihedral_atom_types[atom::LEU] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG1, AT_CD1}};
          chi_dihedral_atom_types[atom::LYS] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_CD},
                                                {AT_CB, AT_CG, AT_CD, AT_CE},
                                                {AT_CG, AT_CD, AT_CE, AT_NZ}};
          chi_dihedral_atom_types[atom::MET] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_SD},
                                                {AT_CB, AT_CG, AT_SD, AT_CE}}; 
          chi_dihedral_atom_types[atom::PHE] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_CD1}};
          chi_dihedral_atom_types[atom::PRO] = {{}};
          chi_dihedral_atom_types[atom::SER] = {{AT_N, AT_CA, AT_CB, AT_OG}};
          chi_dihedral_atom_types[atom::THR] = {{AT_N, AT_CA, AT_CB, AT_OG}};
          chi_dihedral_atom_types[atom::TYR] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_CD1}}; 
          chi_dihedral_atom_types[atom::TRP] = {{AT_N, AT_CA, AT_CB, AT_CG},
                                                {AT_CA, AT_CB, AT_CG, AT_CD1}}; 
          chi_dihedral_atom_types[atom::VAL] = {{AT_N, AT_CA, AT_CB, AT_CG1},
                                                {AT_CA, AT_CB, AT_CG, AT_CD}};
          chi_dihedral_atom_types[atom::UNK] = {{}};

  }  
  return chi_dihedral_atom_types[rt];
}
*/
Vector<Atoms> get_chi_dihedral_atoms(Residue rd) {

  Vector<AtomTypes> dats = get_chi_dihedral_atom_types(rd.get_residue_type());
  if (dats.size() == 0) { 
    Vector<Atoms> das;
    //std::cout << "No chi angles for this residue" << std::endl;
    return das;

  } else {

    Vector<Atoms> das;

    for(unsigned int i=0; i<dats.size(); i++) {
      Atoms da;
      AtomTypes atypes = dats[i];

      for( unsigned int j=0; j<4; j++) {
          da.push_back(IMP::atom::get_atom(rd, atypes[j]));
      }  
      das.push_back(da);
    }

    return das;
  }
}

ParticleIndexQuads get_chi_dihedral_particle_indexes(Residue rd) {

  Vector<AtomTypes> dats = get_chi_dihedral_atom_types(rd.get_residue_type());
  if (dats.size() == 0) { 
    ParticleIndexQuads piqs;
    //std::cout << "No chi angles for this residue" << std::endl;
    return piqs;

  } else {

    ParticleIndexQuads piqs;

    for(unsigned int i=0; i<dats.size(); i++) {
      //std::cout << "Chi angle " << i << " " << dats[i] << std::endl;
      ParticleIndexes pis;
      AtomTypes atypes = dats[i];
      //std::cout << atypes;
      for( unsigned int j=0; j<4; j++) {
          pis.push_back(IMP::atom::get_atom(rd, atypes[j]).get_particle_index());
      }  
      piqs.push_back(ParticleIndexQuad(pis[0], pis[1], pis[2], pis[3]));
    }
    return piqs;
  }
}


IMPATOM_END_NAMESPACE
