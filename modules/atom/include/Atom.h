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
#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <deque>

IMPATOM_BEGIN_NAMESPACE

IMP_DECLARE_CONTROLLED_KEY_TYPE(AtomType, IMP_ATOM_TYPE_INDEX);
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
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_UNKNOWN;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_N;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_CA;
#ifndef IMP_DOXYGEN
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_C;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_O;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_H;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_H1;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_H2;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_H3;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_HA;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_HA1;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_HA2;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_HA3;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_CB;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_HB;
/** \relatesalso AtomType */
IMPATOMEXPORT extern const AtomType AT_HB1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HB2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HB3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OXT;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CH3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CH;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CG;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CG1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CG2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG11;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG21;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG31;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG12;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG13;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG22;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG23;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HG32;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OG;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OG1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_SG;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CD;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CD1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CD2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD11;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD21;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD31;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD12;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD13;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD22;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HD32;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_SD;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OD1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OD2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_ND1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_ND2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CE;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CE1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CE2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CE3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HE;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HE1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HE2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HE3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HE21;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HE22;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OE1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OE2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_NE;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_NE1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_NE2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CZ3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_NZ;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HZ3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_CH2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_NH1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_NH2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OH;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH11;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH21;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH12;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH22;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH13;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH23;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HH33;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_P;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OP1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OP2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_OP3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O5p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C5p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H5p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H5pp;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C4p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H4p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O4p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C1p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H1p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C3p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H3p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O3p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C2p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H2p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H2pp;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O2p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_HO2p;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_N9;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C8;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H8;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_N7;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C5;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C4;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_N3;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_N1;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C6;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_N6;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H61;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H62;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O6;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_N2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_NT;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H21;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H22;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H6;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H5;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O2;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_N4;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H41;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H42;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O4;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_C7;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H71;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H72;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_H73;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O1A;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O2A;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O3A;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O1B;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O2B;
/** \relatesalso AtomType */ IMPATOMEXPORT extern const AtomType AT_O3B;
#endif


//! A decorator for a particle representing an atom.
/** Atoms always are Mass particles.
   \ingroup hierarchy
   \see Hierarchy
 */
class IMPATOMEXPORT Atom:
  public Hierarchy
{
public:

  IMP_DECORATOR(Atom, Hierarchy);

  Particle* get_particle() const {
    return Hierarchy::get_particle();
  }

  /** Create a decorator with the passed type.*/
  static Atom setup_particle(Model *m,
                             ParticleIndex pi,
                             AtomType t);

  /** Create a decorator with the passed type.*/
  static Atom setup_particle(Particle *p, AtomType t) {
    return setup_particle(p->get_model(),
                          p->get_index(), t);
  }

  /** Create a decorator by copying from o.*/
  static Atom setup_particle(Particle *p, Atom o);

  //! return true if the particle has the needed attributes
  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_atom_type_key())
      && Hierarchy::particle_is_instance(p);
  }

  AtomType get_atom_type() const {
    return AtomType(get_particle()->get_value(get_atom_type_key()));
  }

  //! Set the name and corresponding element and mass
  void set_atom_type(AtomType t);

  //! get element
  Element get_element() const {
    return Element(get_particle()->get_value(get_element_key()));
  }
#ifndef IMP_DOXYGEN
  void set_element(Element e);
#endif

  double get_occupancy() const {
    if (!get_particle()->has_attribute(get_occupancy_key())) {
      return 1;
    } else {
      return  get_particle()->get_value(get_occupancy_key());
    }
  }

  void set_occupancy(double occupancy) {
    if (!get_particle()->has_attribute(get_occupancy_key())) {
      get_particle()->add_attribute(get_occupancy_key(),occupancy);
    } else {
      get_particle()->set_value(get_occupancy_key(),occupancy);
    }
  }

  double get_temperature_factor() const {
    if (!get_particle()->has_attribute(get_temperature_factor_key())) {
      return 0;
    } else {
      return  get_particle()->get_value(get_temperature_factor_key());
    }
  }

  void set_temperature_factor(double tempFactor) {
    if (!get_particle()->has_attribute(get_temperature_factor_key())) {
      get_particle()->add_attribute(get_temperature_factor_key(),tempFactor);
    } else {
      get_particle()->set_value(get_temperature_factor_key(),tempFactor);
    }
  }

  /** @name The atom index in the input file
      This index is not necessarily unique over any particular
      set of atoms and so should never be used as an atom identifier
      except during I/O.
      @{
  */
  IMP_DECORATOR_GET_SET_OPT(input_index, get_input_index_key(),
                            Int, Int, -1);
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

IMP_DECORATORS(Atom,Atoms, Hierarchies);


#ifdef SWIG
class Residue;
#endif

//! Return the Residue containing this atom
/** The atom must be part of a molecular hierarchy.

    \throw ValueException if no residue is found, unless
    nothrow is true.

    \relatesalso Atom
    \relatesalso Residue
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Residue get_residue(Atom d, bool nothrow=false);

//! Return a particle atom from the residue
/** The residue must be part of a molecular hierarchy.
    \relatesalso Atom
    \relatesalso Residue
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Atom get_atom(Residue rd, AtomType at);


//! Create a new AtomType
/** This creates a new AtomType (returned) and sets up the mapping
    between the AtomType and the proper element.
    \note This method has not been tested. If you use it, please
    write a test and remove this comment.
    \relatesalso AtomType
    \see atom_type_exists()
*/
IMPATOMEXPORT AtomType add_atom_type(std::string name, Element e);

IMPATOMEXPORT Element get_element_for_atom_type(AtomType at);

//! Return true if that atom type already exists.
IMPATOMEXPORT bool get_atom_type_exists(std::string name);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ATOM_H */
