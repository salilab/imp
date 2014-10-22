/**
 *  \file IMP/atom/Atom.h     \brief Simple atom decorator.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_ATOM_H
#define IMPATOM_ATOM_H

#include <IMP/atom/atom_config.h>
#include "atom_macros.h"
#include "Residue.h"
#include "Hierarchy.h"
#include "element.h"
#include <IMP/core/utility.h>
#include <IMP/core/XYZ.h>

#include <IMP/base_types.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>

#include <vector>
#include <deque>

IMPATOM_BEGIN_NAMESPACE

typedef Key<IMP_ATOM_TYPE_INDEX, false> AtomType;
IMP_VALUES(AtomType, AtomTypes);

/** \class IMP::atom::AtomType
    \brief The type of an atom.

    The standard Atom names in %IMP are derived from the PDB names as follows:
    - the AtomType of a protein, DNA or RNA atom is the AtomType
    created from the PDB atom name string with spaces removed. For
    example, a protein C-alpha has the name AtomType("CA").
    - the AtomType for a hetero (HETATM) atom is the AtomType created by
    prefixing "HET:" to the PDB atom name string (this time without
    spaces removed). For example, a calcium atom is AtomType("HET:CA  ").

    We provide an AtomType instance for each of the standard PDB %atom types.
    These have names such as IMP::atom::AT_N. The full list is elided for
    readability.

    An AtomType implies an element (and hence a mass). While we have the
    associations set up for protein, DNA and RNA atoms, it may be necessary
    to add them for hetero atoms. You can use the add_atom_type() function
    to do this.

    All atoms have the mass stored internally using a Mass decorator.

    \see IMP::atom::Atom
*/

/* each static must be on a separate line because of MSVC bug C2487:
   see http://support.microsoft.com/kb/127900/
*/
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_UNKNOWN;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_N;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_CA;
#ifndef IMP_DOXYGEN
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_C;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_O;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_H;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_H1;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_H2;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_H3;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_HA;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_HA1;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_HA2;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_HA3;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_CB;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_HB;
/** \see AtomType */
IMPATOMEXPORT extern const AtomType AT_HB1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HB2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HB3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OXT;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CH3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CH;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CG;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CG1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CG2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG11;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG21;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG31;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG12;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG13;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG22;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG23;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HG32;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OG;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OG1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_SG;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CD;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CD1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CD2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD11;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD21;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD31;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD12;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD13;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD22;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD23;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HD32;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_SD;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OD1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OD2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_ND1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_ND2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CE;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CE1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CE2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CE3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HE;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HE1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HE2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HE3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HE21;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HE22;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OE1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OE2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_NE;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_NE1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_NE2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_NZ;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_CH2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_NH1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_NH2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OH;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH11;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH21;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH12;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH22;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH13;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH23;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HH33;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_P;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OP1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OP2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_OP3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O5p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C5p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H5p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H5pp;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C4p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H4p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O4p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C1p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H1p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C3p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H3p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O3p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C2p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H2p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H2pp;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O2p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_HO2p;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_N9;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C8;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H8;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_N7;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C5;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C4;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_N3;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_N1;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C6;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_N6;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H61;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H62;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O6;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_N2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_NT;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H21;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H22;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H6;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H5;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O2;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_N4;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H41;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H42;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O4;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_C7;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H71;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H72;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_H73;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O1A;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O2A;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O3A;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O1B;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O2B;
/** \see AtomType */ IMPATOMEXPORT extern const AtomType AT_O3B;
#endif

//! A decorator for a particle representing an atom.
/** Atoms always are Mass particles.
   \ingroup hierarchy
   \see Hierarchy
 */
class IMPATOMEXPORT Atom : public Hierarchy {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                Atom o);
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                AtomType t);

 public:
  IMP_DECORATOR_METHODS(Atom, Hierarchy);
  IMP_DECORATOR_SETUP_1(Atom, Atom, other);
  /** Add the required attributes using the passed AtomType. */
  IMP_DECORATOR_SETUP_1(Atom, AtomType, atom_type);

  /** return true if the particle has the needed attributes */
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_atom_type_key(), pi) &&
           Hierarchy::get_is_setup(m, pi);
  }

  AtomType get_atom_type() const {
    return AtomType(
        get_model()->get_attribute(get_atom_type_key(), get_particle_index()));
  }

  //! Set the name and corresponding element and mass
  void set_atom_type(AtomType t);

  //! get element
  Element get_element() const {
    return Element(
        get_model()->get_attribute(get_element_key(), get_particle_index()));
  }
#ifndef IMP_DOXYGEN
  void set_element(Element e);
#endif

  double get_occupancy() const {
    if (!get_model()->get_has_attribute(get_occupancy_key(),
                                        get_particle_index())) {
      return 1;
    } else {
      return get_model()->get_attribute(get_occupancy_key(),
                                        get_particle_index());
    }
  }

  void set_occupancy(double occupancy) {
    if (!get_model()->get_has_attribute(get_occupancy_key(),
                                        get_particle_index())) {
      get_model()->add_attribute(get_occupancy_key(), get_particle_index(),
                                 occupancy);
    } else {
      get_model()->set_attribute(get_occupancy_key(), get_particle_index(),
                                 occupancy);
    }
  }

  double get_temperature_factor() const {
    if (!get_model()->get_has_attribute(get_temperature_factor_key(),
                                        get_particle_index())) {
      return 0;
    } else {
      return get_model()->get_attribute(get_temperature_factor_key(),
                                        get_particle_index());
    }
  }

  void set_temperature_factor(double tempFactor) {
    if (!get_model()->get_has_attribute(get_temperature_factor_key(),
                                        get_particle_index())) {
      get_model()->add_attribute(get_temperature_factor_key(),
                                 get_particle_index(), tempFactor);
    } else {
      get_model()->set_attribute(get_temperature_factor_key(),
                                 get_particle_index(), tempFactor);
    }
  }

  /** @name The atom index in the input file
      This index is not necessarily unique over any particular
      set of atoms and so should never be used as an atom identifier
      except during I/O.
      @{
  */
  IMP_DECORATOR_GET_SET_OPT(input_index, get_input_index_key(), Int, Int, -1);
  /* @}*/

  /** @name Keys
      These methods provide access to the various keys used to store
      things in the Atom. These can be used if you want to
      use an attribute to search a set of particles.
      @{
   */
  static IntKey get_atom_type_key();

  static IntKey get_element_key();

  static IntKey get_input_index_key();

  static FloatKey get_occupancy_key();

  static FloatKey get_temperature_factor_key();
  //! @}
};

IMP_DECORATORS(Atom, Atoms, Hierarchies);

#ifdef SWIG
class Residue;
#endif

//! Return the Residue containing this atom
/** The atom must be part of a molecular hierarchy.

    \throw ValueException if no residue is found, unless
    nothrow is true.

    \see Atom
    \see Residue
    \see Hierarchy
 */
IMPATOMEXPORT Residue get_residue(Atom d, bool nothrow = false);

//! Return a particle atom from the residue
/** The residue must be part of a molecular hierarchy.
    \see Atom
    \see Residue
    \see Hierarchy
 */
IMPATOMEXPORT Atom get_atom(Residue rd, AtomType at);

//! Create a new AtomType
/** This creates a new AtomType (returned) and sets up the mapping
    between the AtomType and the proper element.
    \note This method has not been tested. If you use it, please
    write a test and remove this comment.
    \see AtomType
    \see atom_type_exists()
*/
IMPATOMEXPORT AtomType add_atom_type(std::string name, Element e);

IMPATOMEXPORT Element get_element_for_atom_type(AtomType at);

//! Return true if that atom type already exists.
IMPATOMEXPORT bool get_atom_type_exists(std::string name);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ATOM_H */
