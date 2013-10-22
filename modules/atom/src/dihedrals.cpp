/**
 *  \file dihedrals.cpp   \brief Helpers to extract dihedral information.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
      if (h) {
        Residue r = h.get_as_residue();
        if (r) {
          return IMP::atom::get_atom(r, type_);
        }
      }
    } else if (residue_ == NEXT) {
      Hierarchy h = get_next_residue(rd);
      if (h) {
        Residue r = h.get_as_residue();
        if (r) {
          return IMP::atom::get_atom(r, type_);
        }
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

IMPATOM_END_NAMESPACE
