/**
 *  \file IMP/atom/pdb.h
 *  \brief Functions to read PDBs
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_PDB_H
#define IMPATOM_PDB_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include "Atom.h"
#include "element.h"
#include "internal/pdb.h"
#include "atom_macros.h"
#include <IMP/file.h>
#include "Selection.h"
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/OptimizerState.h>
#include <IMP/internal/utility.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

IMPATOM_BEGIN_NAMESPACE

//! Select which atoms to read from a PDB file
/** Selector is a general purpose class used to select records from a PDB
    file. Using descendants of this class one may implement arbitrary
    selection functions with operator() and pass them to PDB reading functions
    for object selection. Simple selectors can be used to build more complicated
    ones. Inheritance means "AND" unless otherwise noted (that is, the
    CAlphaPDBSelector takes all non-alternate C-alphas since it inherits from
    NonAlternativePDBSelector).

    \see read_pdb
*/
class IMPATOMEXPORT PDBSelector : public IMP::Object {
 public:
  PDBSelector(std::string name) : Object(name) {}
  //! Return true if the line should be processed
  virtual bool get_is_selected(const std::string &pdb_line) const = 0;
  virtual ~PDBSelector();
};

IMP_OBJECTS(PDBSelector, PDBSelectors);

//! Select all ATOM and HETATM records which are not alternatives
class NonAlternativePDBSelector : public PDBSelector {
 public:
  NonAlternativePDBSelector(std::string name = "NonAlternativePDBSelector%1%")
      : PDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    return (internal::atom_alt_loc_indicator(pdb_line) == ' ' ||
            internal::atom_alt_loc_indicator(pdb_line) == 'A');
  }
  IMP_OBJECT_METHODS(NonAlternativePDBSelector);
};

//! Select all non-alternative ATOM records
class ATOMPDBSelector : public NonAlternativePDBSelector {
 public:
  ATOMPDBSelector(std::string name = "ATOMPDBSelector%1%")
      : NonAlternativePDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    return (NonAlternativePDBSelector::get_is_selected(pdb_line) &&
            internal::is_ATOM_rec(pdb_line));
  }
  IMP_OBJECT_METHODS(ATOMPDBSelector)
};

//! Select all CA ATOM records
class CAlphaPDBSelector : public NonAlternativePDBSelector {
 public:
  CAlphaPDBSelector(std::string name = "CAlphaPDBSelector%1%")
      : NonAlternativePDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == 'A' && type[3] == ' ');
  }
  IMP_OBJECT_METHODS(CAlphaPDBSelector)
};

//! Select all CB ATOM records
class CBetaPDBSelector : public NonAlternativePDBSelector {
 public:
  CBetaPDBSelector(std::string name = "CBetaPDBSelector%1%")
      : NonAlternativePDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == 'B' && type[3] == ' ');
  }
  IMP_OBJECT_METHODS(CBetaPDBSelector)
};

//! Select all atoms of the given types
/** Note that unlike CAlphaPDBSelector and similar classes, this selects all
    atoms, even those in alternative locations (combine with
    a NonAlternativePDBSelector if necessary).
 */
class AtomTypePDBSelector : public PDBSelector {
  Strings atom_types_;
 public:
  AtomTypePDBSelector(Strings atom_types,
                      std::string name = "AtomTypePDBSelector%1%")
      : PDBSelector(name), atom_types_(atom_types) {
    std::sort(atom_types_.begin(), atom_types_.end());
  }

  bool get_is_selected(const std::string &pdb_line) const {
    std::string type = internal::atom_type(pdb_line);
    boost::trim(type);
    return std::binary_search(atom_types_.begin(), atom_types_.end(), type);
  }
  IMP_OBJECT_METHODS(AtomTypePDBSelector)
};

//! Select all atoms in residues of the given types
/** Note that unlike CAlphaPDBSelector and similar classes, this selects all
    atoms, even those in alternative locations (combine with
    a NonAlternativePDBSelector if necessary).
 */
class ResidueTypePDBSelector : public PDBSelector {
  Strings residue_types_;
 public:
  ResidueTypePDBSelector(Strings residue_types,
                         std::string name = "ResidueTypePDBSelector%1%")
      : PDBSelector(name), residue_types_(residue_types) {
    std::sort(residue_types_.begin(), residue_types_.end());
  }

  bool get_is_selected(const std::string &pdb_line) const {
    std::string type = internal::atom_residue_name(pdb_line);
    boost::trim(type);
    return std::binary_search(residue_types_.begin(), residue_types_.end(),
                              type);
  }
  IMP_OBJECT_METHODS(ResidueTypePDBSelector)
};

//! Select all C (not CA or CB) ATOM records
class CPDBSelector : public NonAlternativePDBSelector {
 public:
  CPDBSelector(std::string name = "CPDBSelector%1%")
      : NonAlternativePDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == ' ' && type[3] == ' ');
  }
  IMP_OBJECT_METHODS(CPDBSelector)
};

//! Select all N ATOM records
class NPDBSelector : public NonAlternativePDBSelector {
 public:
  NPDBSelector(std::string name = "NPDBSelector%1%")
      : NonAlternativePDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'N' && type[2] == ' ' && type[3] == ' ');
  }
  IMP_OBJECT_METHODS(NPDBSelector)
};

//! Defines a selector that will pick every ATOM and HETATM record
class AllPDBSelector : public PDBSelector {
 public:
  AllPDBSelector(std::string name = "AllPDBSelector%1%") : PDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    return (true || pdb_line.empty());
  }
  IMP_OBJECT_METHODS(AllPDBSelector);
};

//! Select all ATOM and HETATM records with the given chain ids
class ChainPDBSelector : public NonAlternativePDBSelector {
 public:
  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
      return false;
    }
    for (int i = 0; i < (int)chains_.length(); i++) {
      if (internal::atom_chain_id(pdb_line) == chains_[i]) return true;
    }
    return false;
  }
  IMP_OBJECT_METHODS(ChainPDBSelector);
  //! The chain id can be any character in chains
  ChainPDBSelector(const std::string &chains,
                   std::string name = "ChainPDBSelector%1%")
      : NonAlternativePDBSelector(name), chains_(chains) {}

 private:
  std::string chains_;
};

//! Select all non-water ATOM and HETATM records
class WaterPDBSelector : public NonAlternativePDBSelector {
 public:
  WaterPDBSelector(std::string name = "WaterPDBSelector%1%")
      : NonAlternativePDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
      return false;
    }
    const std::string res_name = internal::atom_residue_name(pdb_line);
    return ((res_name[0] == 'H' && res_name[1] == 'O' && res_name[2] == 'H') ||
            (res_name[0] == 'D' && res_name[1] == 'O' && res_name[2] == 'D'));
  }
  IMP_OBJECT_METHODS(WaterPDBSelector)
};

//! Select all hydrogen ATOM and HETATM records
class IMPATOMEXPORT HydrogenPDBSelector : public NonAlternativePDBSelector {
  bool is_hydrogen(std::string pdb_line) const;

 public:
  HydrogenPDBSelector(std::string name = "HydrogenPDBSelector%1%")
      : NonAlternativePDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) return false;
    return is_hydrogen(pdb_line);
  }
  IMP_OBJECT_METHODS(HydrogenPDBSelector)
};

//! Select non water and non hydrogen atoms
class NonWaterNonHydrogenPDBSelector : public NonAlternativePDBSelector {
  IMP::PointerMember<PDBSelector> ws_, hs_;

 public:
  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
      return false;
    }
    return (!ws_->get_is_selected(pdb_line) && !hs_->get_is_selected(pdb_line));
  }
  IMP_OBJECT_METHODS(NonWaterNonHydrogenPDBSelector);
  NonWaterNonHydrogenPDBSelector(std::string name)
      : NonAlternativePDBSelector(name),
        ws_(new WaterPDBSelector()),
        hs_(new HydrogenPDBSelector()) {}
  NonWaterNonHydrogenPDBSelector()
      : NonAlternativePDBSelector("NonWaterPDBSelector%1%"),
        ws_(new WaterPDBSelector()),
        hs_(new HydrogenPDBSelector()) {}
};

//! Select non hydrogen atoms
class NonHydrogenPDBSelector : public NonAlternativePDBSelector {
  IMP::PointerMember<PDBSelector> hs_;

 public:
  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
      return false;
    }
    return (!hs_->get_is_selected(pdb_line));
  }
  IMP_OBJECT_METHODS(NonHydrogenPDBSelector);
  NonHydrogenPDBSelector(std::string name)
    : NonAlternativePDBSelector(name),
      hs_(new HydrogenPDBSelector()) {}
  NonHydrogenPDBSelector()
    : NonAlternativePDBSelector("NonHydrogenPDBSelector%1%"),
      hs_(new HydrogenPDBSelector()) {}
};

//! Select all non-water non-alternative ATOM and HETATM records
class NonWaterPDBSelector : public NonAlternativePDBSelector {
  IMP::PointerMember<PDBSelector> ws_;

 public:
  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
      return false;
    }
    return (!ws_->get_is_selected(pdb_line));
  }
  IMP_OBJECT_METHODS(NonWaterPDBSelector);
  NonWaterPDBSelector(std::string name)
      : NonAlternativePDBSelector(name), ws_(new WaterPDBSelector()) {}
  NonWaterPDBSelector()
      : NonAlternativePDBSelector("NonWaterPDBSelector%1%"),
        ws_(new WaterPDBSelector()) {}
};

//! Select all backbone (N,CA,C,O) ATOM records
class BackbonePDBSelector : public NonWaterNonHydrogenPDBSelector {
 public:
  BackbonePDBSelector(std::string name = "BackbonePDBSelector%1%")
      : NonWaterNonHydrogenPDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonWaterNonHydrogenPDBSelector::get_is_selected(pdb_line))
      return false;
    const std::string type = internal::atom_type(pdb_line);
    return ((type[1] == 'N' && type[2] == ' ' && type[3] == ' ') ||
            (type[1] == 'C' && type[2] == 'A' && type[3] == ' ') ||
            (type[1] == 'C' && type[2] == ' ' && type[3] == ' ') ||
            (type[1] == 'O' && type[2] == ' ' && type[3] == ' '));
  }
  IMP_OBJECT_METHODS(BackbonePDBSelector)
};

//! Select all P (= phosphate) ATOM records
class PPDBSelector : public NonAlternativePDBSelector {
 public:
  PPDBSelector(std::string name = "PPDBSelector%1%")
      : NonAlternativePDBSelector(name) {}

  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'P' && type[2] == ' ' && type[3] == ' ');
  }
  IMP_OBJECT_METHODS(PPDBSelector)
};

//! Select atoms which are selected by both selectors
/** To use do something like
    \code
    read_pdb(name, m, AndPDBSelector(PPDBSelector(), WaterPDBSelector()));
    \endcode

    In Python, the and operator (&) can be used to the same effect:
    \code
    read_pdb(name, m, PPDBSelector() & WaterPDBSelector());
    \endcode
 */
class AndPDBSelector : public PDBSelector {
  const IMP::PointerMember<PDBSelector> a_, b_;

 public:
  bool get_is_selected(const std::string &pdb_line) const {
    return a_->get_is_selected(pdb_line) && b_->get_is_selected(pdb_line);
  }
  IMP_OBJECT_METHODS(AndPDBSelector);
  AndPDBSelector(PDBSelector *a, PDBSelector *b)
      : PDBSelector("AndPDBSelector%1%"), a_(a), b_(b) {}
};

//! Select atoms which are selected by either or both selectors
/** To use do something like
    \code
    read_pdb(name, m, OrPDBSelector(PPDBSelector(), WaterPDBSelector()));
    \endcode

    In Python, the or operator (|) can be used to the same effect:
    \code
    read_pdb(name, m, PPDBSelector() | WaterPDBSelector());
    \endcode
 */
class OrPDBSelector : public PDBSelector {
  const IMP::PointerMember<PDBSelector> a_, b_;

 public:
  bool get_is_selected(const std::string &pdb_line) const {
    return a_->get_is_selected(pdb_line) || b_->get_is_selected(pdb_line);
  }
  IMP_OBJECT_METHODS(OrPDBSelector);
  OrPDBSelector(PDBSelector *a, PDBSelector *b)
      : PDBSelector("OrPDBSelector%1%"), a_(a), b_(b) {}
};

//! Select atoms which are selected by either selector but not both
/** To use do something like
    \code
    read_pdb(name, m, XorPDBSelector(HydrogenPDBSelector(),
                                     WaterPDBSelector()));
    \endcode

    In Python, the xor operator (^) can be used to the same effect:
    \code
    read_pdb(name, m, HydrogenPDBSelector() ^ WaterPDBSelector());
    \endcode
 */
class XorPDBSelector : public PDBSelector {
  const IMP::PointerMember<PDBSelector> a_, b_;

 public:
  bool get_is_selected(const std::string &pdb_line) const {
    return a_->get_is_selected(pdb_line) != b_->get_is_selected(pdb_line);
  }
  IMP_OBJECT_METHODS(XorPDBSelector);
  XorPDBSelector(PDBSelector *a, PDBSelector *b)
      : PDBSelector("XorPDBSelector%1%"), a_(a), b_(b) {}
};

//! Select atoms which are not selected by a given selector
/** To use do something like
    \code
    read_pdb(name, m, NotPDBSelector(PPDBSelector()));
    \endcode

    In Python, the inversion operator (~) can be used to the same effect:
    \code
    read_pdb(name, m, ~PPDBSelector());
    \endcode
 */
class NotPDBSelector : public PDBSelector {
  const IMP::PointerMember<PDBSelector> a_;

 public:
  bool get_is_selected(const std::string &pdb_line) const {
    return !a_->get_is_selected(pdb_line);
  }
  IMP_OBJECT_METHODS(NotPDBSelector);
  NotPDBSelector(PDBSelector *a) : PDBSelector("NotPDBSelector%1%"), a_(a) {}
};

/** @name PDB Reading
    \anchor pdb_in
   The read PDB methods produce a hierarchy that looks as follows:
    - One Atom per ATOM or HETATM record in the PDB.
    - All Atom particles have a parent which is a Residue.
    - All Residue particles have a parent which is a Chain.

    Waters are currently dropped if they are ATOM records. This can be fixed.

    The read_pdb() functions should successfully parse all valid PDB files. It
    can produce warnings on files which are not valid. It will attempt to read
    such files, but all bets are off.

    In order to track the provenance of IMP-generated models, the provenance
    of any PDB files read in here - for example, the PDB id, or detail about
    a comparative model - needs to also be tracked. This is done using the
    PDB headers:
    - Structures stored in the PDB database should keep the standard
      `HEADER` record stating their PDB ID.
    - Comparative models generated using MODELLER should include the
      MODELLER-generated `EXPDTA` and `REMARK` records.
    - Structures that are trivial modifications of an existing PDB structure
      or comparative model should use the `TITLE` record to describe the
      nature of the modification (e.g. rotation and translation) and one of
      the two following custom `EXPDTA` record formats to point to the original
      structure:
      - `EXPDTA    DERIVED FROM PDB:1XYZ`
      - `EXPDTA    DERIVED FROM COMPARATIVE MODEL, DOI:x.y/z`
    - Structures generated from multiple sources (e.g. two structures that
      have been docked and then concatenated into a single PDB file) are not
      allowed. Store each constituent structure in its own file and annotate
      each one with a suitable `EXPDTA` record, as above.
    Note that while provenance of PDB files is not currently enforced, it
    likely will be in future IMP releases.

    When reading PDBs, PDBSelector objects can be used to choose to only process
    certain record types. See the class documentation for more information.
    When no PDB selector is supplied for reading, the
    NonWaterPDBSelector is used.

    Set the IMP::LogLevel to VERBOSE to see details of parse errors.
*/
//!@{

inline PDBSelector *get_default_pdb_selector() {
  return new NonWaterPDBSelector();
}

//! Read all the molecules in the first model of the PDB file.
IMPATOMEXPORT Hierarchy
    read_pdb(TextInput input, Model *model,
             PDBSelector *selector = get_default_pdb_selector(),
             bool select_first_model = true
#ifndef IMP_DOXYGEN
             ,
             bool no_radii = false
#endif
             );

/** Rewrite the coordinates of the passed hierarchy based
    on the contents of the first model in the PDB file.

    The hierarchy must have been created by reading from a PDB
    file and the atom numbers must correspond between the files.
    These are not really checked.

    A ValueException is thrown if there are insufficient models
    in the file.

    core::RigidMember particles are handled by updating the
    core::RigidBody algebra::ReferenceFrame3D to align with the
    loaded particles. Bad things will happen if the loaded coordinates
    are not a rigid transform of the prior coordinates.
 */
IMPATOMEXPORT void read_pdb(TextInput input, int model, Hierarchy h);

/** Read all models from the PDB file.
 */
IMPATOMEXPORT Hierarchies
    read_multimodel_pdb(TextInput input, Model *model,
                        PDBSelector *selector = get_default_pdb_selector()
#ifndef IMP_DOXYGEN
                        ,
                        bool noradii = false
#endif
                        );

/** @name PDB Writing
    \anchor pdb_out
    The methods to write a PDB expects a Hierarchy that looks as follows:
    - all leaves are Atom particles
    - all Atom particles have Residue particles as parents

    All Residue particles that have a Chain particle as an ancestor
    are considered part of a protein, DNA or RNA, ones without are
    considered heterogens.

    The functions produce files that are not valid PDB files,
    eg only ATOM/HETATM lines are printed for all Atom particles
    in the hierarchy. Complain if your favorite program can't read them and
    we might fix it.
*/
//!@{

/** Write some atoms to a PDB.
*/
IMPATOMEXPORT void write_pdb(const Selection &mhd, TextOutput out,
                             unsigned int model = 1);

/** \brief Write a hierarchy to a PDB as C_alpha atoms.

    This method is used to write a non-atomic hierarchy into a PDB in a way
    that can be read by most programs. If the leaves are Residue particles
    then the index and residue type will be read from them. Otherwise default
    values will be used so that each leaf ends up in a separate residue.
*/
IMPATOMEXPORT void write_pdb_of_c_alphas(const Selection &mhd,
                                         TextOutput out,
                                         unsigned int model = 1);

/** Write the hierarchies one per frame.
*/
IMPATOMEXPORT void write_multimodel_pdb(const Hierarchies &mhd,
                                        TextOutput out);
/** @} */

#ifndef IMP_DOXYGEN

/**
   This function returns a string in PDB ATOM format
*/
IMPATOMEXPORT std::string get_pdb_string(
    const algebra::Vector3D &v, int index = -1, AtomType at = AT_CA,
    ResidueType rt = atom::ALA, char chain = ' ', int res_index = 1,
    char res_icode = ' ', double occupancy = 1.00, double tempFactor = 0.00,
    Element e = C);

/**
   This function returns a connectivity string in PDB format
  \note The CONECT records specify connectivity between atoms for which
      coordinates are supplied. The connectivity is described using
      the atom serial number as found in the entry.
  \note http://www.bmsc.washington.edu/CrystaLinks/man/pdb/guide2.2_frame.html
*/
IMPATOMEXPORT std::string get_pdb_conect_record_string(int, int);
#endif

/** \class WritePDBOptimizerState
    This writes a PDB file at the specified interval during optimization.
    If the file name contains %1% then a new file is written each time
    with the %1% replaced by the index. Otherwise a new model is written
    each time to the same file.
*/
class IMPATOMEXPORT WritePDBOptimizerState : public OptimizerState {
  std::string filename_;
  ParticleIndexes pis_;

 public:
  WritePDBOptimizerState(Model *m,
                         const ParticleIndexesAdaptor &pis,
                         std::string filename);
  WritePDBOptimizerState(const atom::Hierarchies mh, std::string filename);

 protected:
  virtual void do_update(unsigned int call) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(WritePDBOptimizerState);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PDB_H */
