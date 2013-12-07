/**
 * \file soap_bond_separation.h
 * \brief SOAP bond separation filter
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SOAP_BOND_SEPARATION_H
#define IMPATOM_SOAP_BOND_SEPARATION_H

#include <IMP/atom/atom_config.h>
#include <IMP/score_functor/score_functor_config.h>

#ifdef IMP_SCORE_FUNCTOR_USE_HDF5

#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/score_functor/internal/soap_hdf5.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

// Calculate bond separation in the same manner that was used to generate
// the SOAP potential in the first place (i.e. using information in the
// HDF5 file, not the IMP bond graph)
class SoapBondSeparation {
  static const int ATOM_TYPE_N = 0;
  static const int ATOM_TYPE_C = 1;

  // Number of bonds separating each pair of atoms for a residue type
  class SoapResidueBond {
    typedef std::map<AtomType, int> AtomMap;
    int natom_;
    // Mapping from atom types to indices into the distance matrix
    AtomMap atom_map_;
    // Atom types in this residue type
    std::vector<AtomType> atom_types_;
    // Number of bonds separating each pair of atoms.
    // Note that this is a square matrix, although only the upper
    // triangle is used.
    std::vector<int> distance_;

    // Follow bonds recursively to find distances to all atoms from the given
    // start atom
    void follow_bond(int start_atom, int endpoint1, int endpoint2,
                     int distance) {
      // Protect against infinite loops if cycles are present in the structure
      if (distance > natom_) {
        return;
      }
      for (int other_atom = 0; other_atom < natom_; ++other_atom) {
        /* Don't double back on ourselves */
        if (other_atom != start_atom && other_atom != endpoint1 &&
            other_atom != endpoint2) {
          int dist = get_distance(endpoint2, other_atom);
          if (dist == 1) {
            int old_distance = get_distance(start_atom, other_atom);
            /* In case of cycles, take the shortest route */
            if (old_distance == -1 || distance + 1 <= old_distance) {
              set_distance(start_atom, other_atom, distance + 1);
              follow_bond(start_atom, endpoint2, other_atom, distance + 1);
            }
          }
        }
      }
    }

   public:
    SoapResidueBond() {}

    int add_atom_type(AtomType t) {
      int sz = atom_map_.size();
      if (sz == 0) {
        // Add backbone N and C atoms at start of distance matrix for speed
        atom_map_[AT_N] = ATOM_TYPE_N;
        atom_map_[AT_C] = ATOM_TYPE_C;
        sz = atom_map_.size();
      }
      AtomMap::const_iterator it = atom_map_.find(t);
      if (it == atom_map_.end()) {
        atom_map_[t] = sz;
        return sz;
      } else {
        return it->second;
      }
    }

    // Initialize the atom-atom bond distance matrix
    void init_distances() {
      natom_ = atom_map_.size();
      distance_.resize(0);
      for (int i = 0; i < natom_; ++i) {
        for (int j = 0; j < natom_; ++j) {
          distance_.push_back(j == i ? 0 : -1);
        }
      }
    }

    // Map an AtomType to an int
    int lookup_atom_type(AtomType at) const {
      AtomMap::const_iterator it = atom_map_.find(at);
      if (it == atom_map_.end()) {
        return -1;
      } else {
        return it->second;
      }
    }

    // Set the bond distance between the two atom types in the residue
    void set_distance(int at1, int at2, int distance) {
      int index = std::min(at1, at2) * natom_ + std::max(at1, at2);
      distance_[index] = distance;
    }

    // Get the bond distance between the two atom types in the residue
    int get_distance(int at1, int at2) const {
      int index = std::min(at1, at2) * natom_ + std::max(at1, at2);
      return distance_[index];
    }

    int get_distance(AtomType at1, int at2) const {
      int a = lookup_atom_type(at1);
      if (a >= 0) {
        return get_distance(a, at2);
      } else {
        return -1;
      }
    }

    int get_distance(AtomType at1, AtomType at2) const {
      int a = lookup_atom_type(at2);
      if (a >= 0) {
        return get_distance(at1, a);
      } else {
        return -1;
      }
    }

    // Get bond distances between all pairs of atoms
    void fill_distances() {
      for (int start_atom = 0; start_atom < natom_; ++start_atom) {
        for (int other_atom = 0; other_atom < natom_; ++other_atom) {
          if (other_atom != start_atom) {
            if (get_distance(start_atom, other_atom) == 1) {
              follow_bond(start_atom, -1, other_atom, 1);
            }
          }
        }
      }
    }
  };

  // A bond between two atom types in a given residue, as read from the
  // SOAP HDF5 library file.
  struct SoapLibraryBond {
    ResidueType r;
    AtomType a1, a2;

    SoapLibraryBond(ResidueType ir, AtomType ia1, AtomType ia2)
        : r(ir), a1(ia1), a2(ia2) {}
  };
  typedef std::vector<SoapLibraryBond> SoapLibraryBonds;

  typedef std::map<ResidueType, SoapResidueBond> ResidueTypeMap;

  // A single disulfide bridge
  struct Bridge {
    Residue r1, r2;
    Atom a1, a2;

    Bridge(Residue ir1, Residue ir2, Atom ia1, Atom ia2)
        : r1(ir1), r2(ir2), a1(ia1), a2(ia2) {}
  };
  typedef std::vector<Bridge> Bridges;

  std::vector<int> bond_span_range_;
  bool disulfide_;
  ResidueTypeMap residue_type_map_;

  typedef std::vector<std::pair<Residue, int> > NearbyCys;
  // Get all CYS residues within ssseq residues of the given residue
  NearbyCys get_nearby_cys(Chain chain, int ind, int num_in_chain,
                           int ssseq) const {
    NearbyCys nearby;
    for (int i = std::max(1, ind - ssseq);
         i <= std::min(num_in_chain, ind + ssseq); ++i) {
      Residue r = get_residue(chain, i).get_as_residue();
      if (r.get_residue_type() == CYS) {
        nearby.push_back(std::make_pair(r, i));
      }
    }
    return nearby;
  }

  // Get a list of all disulfide bridges near the given residue pair
  Bridges get_disulfide_bridges(Residue r1, Residue r2) const {
    static const int ssseq = 3;
    static const float ssdistsqr = 2.5 * 2.5;
    Bridges bridges;
    Hierarchy p = r1.get_parent();
    Chain chain = p.get_as_chain();
    int num_in_chain = p.get_number_of_children();
    int ind_r1 = r1.get_index();
    int ind_r2 = r2.get_index();
    NearbyCys cys1 = get_nearby_cys(chain, ind_r1, num_in_chain, ssseq);
    NearbyCys cys2 = get_nearby_cys(chain, ind_r2, num_in_chain, ssseq);

    for (NearbyCys::const_iterator i1 = cys1.begin(); i1 != cys1.end(); ++i1) {
      for (NearbyCys::const_iterator i2 = cys2.begin(); i2 != cys2.end();
           ++i2) {
        if (i1->second != i2->second &&
            std::abs(i1->second - ind_r1) + std::abs(i2->second - ind_r2) <=
                ssseq) {
          Atom ss_a1 = get_atom(i1->first, AT_SG);
          if (ss_a1 != Atom()) {
            Atom ss_a2 = get_atom(i2->first, AT_SG);
            if (ss_a2 != Atom()) {
              if (algebra::get_squared_distance(
                      core::XYZ(ss_a1).get_coordinates(),
                      core::XYZ(ss_a2).get_coordinates()) < ssdistsqr) {
                Bridge b(i1->first, i2->first, ss_a1, ss_a2);
                bridges.push_back(b);
              }
            }
          }
        }
      }
    }
    return bridges;
  }

  // Is the bond between the two named atoms confined to a single residue,
  // as opposed to spanning residues?
  bool internal_bond(std::string at1, std::string at2) const {
    return (at1[0] != '+' && at1[0] != '-' && at2[0] != '+' && at2[0] != '-');
  }

  // Used to cache lookups for disulfides
  const SoapResidueBond *cys_residue_bond_;
  int sg_atom_type_;

  // Given the bonds from the SOAP library, determine the number of bonds
  // between every atom in each residue type in the library
  void fill_residue_type_map(const SoapLibraryBonds &bonds) {
    for (SoapLibraryBonds::const_iterator it = bonds.begin(); it != bonds.end();
         ++it) {
      SoapResidueBond &res = residue_type_map_[it->r];
      res.add_atom_type(it->a1);
      res.add_atom_type(it->a2);
    }
    for (ResidueTypeMap::iterator it = residue_type_map_.begin();
         it != residue_type_map_.end(); ++it) {
      it->second.init_distances();
    }
    for (SoapLibraryBonds::const_iterator it = bonds.begin(); it != bonds.end();
         ++it) {
      SoapResidueBond &res = residue_type_map_[it->r];
      res.set_distance(res.lookup_atom_type(it->a1),
                       res.lookup_atom_type(it->a2), 1);
    }
    for (ResidueTypeMap::iterator it = residue_type_map_.begin();
         it != residue_type_map_.end(); ++it) {
      it->second.fill_distances();
    }
    // Cache lookups for disulfide bridges
    if (disulfide_) {
      cys_residue_bond_ = NULL;
      ResidueTypeMap::const_iterator it = residue_type_map_.find(CYS);
      if (it != residue_type_map_.end()) {
        cys_residue_bond_ = &(it->second);
        sg_atom_type_ = it->second.lookup_atom_type(AT_SG);
      }
    }
  }

  void read_bonds(score_functor::internal::Hdf5File &file_id) {
    score_functor::internal::Hdf5Group group(file_id.get(),
                                             "/library/bond_classes");
    score_functor::internal::Hdf5Dataset type_names_ds(group.get(),
                                                       "type_names");
    std::vector<std::string> type_names = type_names_ds.read_string_vector();
    SoapLibraryBonds bonds;
    for (unsigned i = 0; i < type_names.size(); i += 3) {
      if (internal_bond(type_names[i + 1], type_names[i + 2])) {
        SoapLibraryBond b = SoapLibraryBond(ResidueType(type_names[i]),
                                            AtomType(type_names[i + 1]),
                                            AtomType(type_names[i + 2]));
        bonds.push_back(b);
      }
    }
    fill_residue_type_map(bonds);
  }

  // Get bond separation between two atoms in different residues. Note that
  // atom1/res1 must be come before atom2/res2 in the amino acid sequence.
  int get_bond_separation_external(const SoapResidueBond &rb1,
                                   const SoapResidueBond &rb2, int at1, int at2,
                                   int ind_r1, int ind_r2) const {
    // Get distance to backbone C in first residue, distance to backbone N
    // in second residue, and add backbone bonds for all intervening residues.
    int dist_to_c = rb1.get_distance(at1, ATOM_TYPE_C);
    if (dist_to_c == -1) {
      return -1;
    }
    int dist_to_n = rb2.get_distance(at2, ATOM_TYPE_N);
    if (dist_to_n == -1) {
      return -1;
    }
    int backbone_bonds = (ind_r2 - ind_r1 - 1) * 3 + 1;
    return dist_to_c + backbone_bonds + dist_to_n;
  }

  // Get the number of bonds separating two atoms in the same chain.
  // -1 is returned if the atoms are not connected. Without s-s path.
  int get_bond_separation_same_chain_noss(Residue r1, Residue r2,
                                          const SoapResidueBond &rb1,
                                          const SoapResidueBond &rb2, int at1,
                                          int at2, bool &sameres) const {
    sameres = false;
    int ind_r1 = r1.get_index();
    int ind_r2 = r2.get_index();
    if (ind_r1 == ind_r2) {
      sameres = true;
      return rb1.get_distance(at1, at2);
    } else if (ind_r1 < ind_r2) {
      // Order by sequence
      return get_bond_separation_external(rb1, rb2, at1, at2, ind_r1, ind_r2);
    } else {
      return get_bond_separation_external(rb2, rb1, at2, at1, ind_r2, ind_r1);
    }
  }

  // Get the number of bonds separating two atoms in the same chain.
  // -1 is returned if the atoms are not connected.
  int get_bond_separation_same_chain(Residue r1, Residue r2,
                                     const SoapResidueBond &rb1,
                                     const SoapResidueBond &rb2, int at1,
                                     int at2) const {
    bool sameres;
    int bonddist = get_bond_separation_same_chain_noss(r1, r2, rb1, rb2, at1,
                                                       at2, sameres);
    // Recalculate the bond separation using s-s bonds
    if (disulfide_ && cys_residue_bond_ && !sameres) {
      Bridges bridges = get_disulfide_bridges(r1, r2);
      for (Bridges::const_iterator it = bridges.begin(); it != bridges.end();
           ++it) {
        int ss_dist = get_bond_separation_same_chain_noss(
                          r1, it->r1, rb1, *cys_residue_bond_, at1,
                          sg_atom_type_, sameres) +
                      get_bond_separation_same_chain_noss(
                          it->r2, r2, *cys_residue_bond_, rb2, sg_atom_type_,
                          at2, sameres) +
                      1;
        bonddist = std::min(bonddist, ss_dist);
      }
    }
    return bonddist;
  }

  bool residues_in_same_chain(Residue r1, Residue r2) const {
    Hierarchy p1 = r1.get_parent();
    Hierarchy p2 = r2.get_parent();
    return p1 == p2;
  }

  // Determine the number of bonds between the two atoms (or -1 if unconnected)
  int get_bond_separation(kernel::Model *m,
                          const kernel::ParticleIndexPair &p) const {
    Atom a1(m, p[0]);
    Atom a2(m, p[1]);
    Residue r1 = get_residue(a1);
    Residue r2 = get_residue(a2);
    if (a1 == a2) {
      return 0;
    } else if (!residues_in_same_chain(r1, r2)) {
      // Atoms in different chains can't be connected.
      return -1;
    } else {
      ResidueTypeMap::const_iterator it1 =
          residue_type_map_.find(r1.get_residue_type());
      ResidueTypeMap::const_iterator it2 =
          residue_type_map_.find(r2.get_residue_type());
      if (it1 != residue_type_map_.end() && it2 != residue_type_map_.end()) {
        int at1 = it1->second.lookup_atom_type(a1.get_atom_type());
        int at2 = it2->second.lookup_atom_type(a2.get_atom_type());
        if (at1 >= 0 && at2 >= 0) {
          return get_bond_separation_same_chain(r1, r2, it1->second,
                                                it2->second, at1, at2);
        }
      }
      // Unknown atom or residue types are not connected
      return -1;
    }
  }

 public:
  SoapBondSeparation() {}

  void read(score_functor::internal::Hdf5File &file_id) {
    score_functor::internal::Hdf5Group group(file_id.get(), "scan");
    bond_span_range_ = group.read_int_attribute_vector("bond_span_range", 2);
    disulfide_ = (group.read_int_attribute("disulfide") != 0);
    read_bonds(file_id);
  }

  bool enabled() const {
    return bond_span_range_[0] != -1 || bond_span_range_[1] != -1;
  }

  bool get_separation_ok(kernel::Model *m,
                         const kernel::ParticleIndexPair &p) const {
    if (!enabled()) {
      return true;
    } else {
      int sep = get_bond_separation(m, p);
      if (bond_span_range_[1] < 0) {
        return sep == -1 || sep >= bond_span_range_[0];
      } else {
        return sep >= bond_span_range_[0] && sep <= bond_span_range_[1];
      }
    }
  }
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif  // IMP_SCORE_FUNCTOR_USE_HDF5

#endif /* IMPATOM_SOAP_BOND_SEPARATION_H */
