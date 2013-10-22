/**
 *  \file IMP/atom/Atom.h     \brief Simple atom decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
    prefixing "HET:" to the PBD atom name string (this time without
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
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_UNKNOWN;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_N;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_CA;
#ifndef IMP_DOXYGEN
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_C;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_O;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_H;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_H1;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_H2;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_H3;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_HA;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_HA1;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_HA2;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_HA3;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_CB;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_HB;
/** See AtomType */
IMPATOMEXPORT extern const AtomType AT_HB1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HB2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HB3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OXT;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CH3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CH;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CG;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CG1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CG2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG11;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG21;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG31;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG12;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG13;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG22;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG23;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HG32;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OG;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OG1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_SG;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CD;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CD1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CD2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD11;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD21;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD31;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD12;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD13;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD22;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD23;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HD32;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_SD;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OD1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OD2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_ND1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_ND2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CE;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CE1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CE2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CE3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HE;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HE1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HE2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HE3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HE21;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HE22;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OE1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OE2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_NE;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_NE1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_NE2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_NZ;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_CH2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_NH1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_NH2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OH;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH11;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH21;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH12;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH22;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH13;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH23;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HH33;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_P;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OP1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OP2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_OP3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O5p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C5p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H5p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H5pp;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C4p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H4p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O4p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C1p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H1p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C3p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H3p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O3p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C2p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H2p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H2pp;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O2p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_HO2p;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_N9;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C8;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H8;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_N7;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C5;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C4;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_N3;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_N1;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C6;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_N6;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H61;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H62;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O6;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_N2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_NT;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H21;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H22;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H6;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H5;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O2;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_N4;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H41;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H42;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O4;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_C7;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H71;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H72;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_H73;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O1A;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O2A;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O3A;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O1B;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O2B;
/** See AtomType */ IMPATOMEXPORT extern const AtomType AT_O3B;
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

    See Atom
    See Residue
    See Hierarchy
 */
IMPATOMEXPORT Residue get_residue(Atom d, bool nothrow = false);

//! Return a particle atom from the residue
/** The residue must be part of a molecular hierarchy.
    See Atom
    See Residue
    See Hierarchy
 */
IMPATOMEXPORT Atom get_atom(Residue rd, AtomType at);

//! Create a new AtomType
/** This creates a new AtomType (returned) and sets up the mapping
    between the AtomType and the proper element.
    \note This method has not been tested. If you use it, please
    write a test and remove this comment.
    See AtomType
    \see atom_type_exists()
*/
IMPATOMEXPORT AtomType add_atom_type(std::string name, Element e);

IMPATOMEXPORT Element get_element_for_atom_type(AtomType at);

//! Return true if that atom type already exists.
IMPATOMEXPORT bool get_atom_type_exists(std::string name);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ATOM_H */
