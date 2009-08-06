/**
 *  \file atom/Atom.h     \brief Simple atom decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_ATOM_H
#define IMPATOM_ATOM_H

#include "config.h"
#include "macros.h"
#include "Residue.h"
#include "Hierarchy.h"
#include <IMP/core/utility.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/macros.h>

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <deque>

IMPATOM_BEGIN_NAMESPACE

IMP_DECLARE_KEY_TYPE(AtomType, IMP_ATOM_TYPE_INDEX);


/** @name PDB Atom Types

    We provide an AtomType instance for each of the standard PDB %atom types.
    These have names such as IMP::atom::AT_N. The full list is elided for
    readability. New types can be added using the
    add_atom_type() function.

    \see IMP::atom::Atom
*/
/*@{*/
/* each static must be on a separate line because of MSVC bug C2487:
   see http://support.microsoft.com/kb/127900/
*/
/** */
IMPATOMEXPORT extern const AtomType AT_UNKNOWN;
/** */
IMPATOMEXPORT extern const AtomType AT_N;
/** */
IMPATOMEXPORT extern const AtomType AT_CA;
#ifndef IMP_DOXYGEN
/** */
IMPATOMEXPORT extern const AtomType AT_C;
/** */
IMPATOMEXPORT extern const AtomType AT_O;
/** */
IMPATOMEXPORT extern const AtomType AT_H;
/** */
IMPATOMEXPORT extern const AtomType AT_1H;
/** */
IMPATOMEXPORT extern const AtomType AT_2H;
/** */
IMPATOMEXPORT extern const AtomType AT_3H;
/** */
IMPATOMEXPORT extern const AtomType AT_HA;
/** */
IMPATOMEXPORT extern const AtomType AT_1HA;
/** */
IMPATOMEXPORT extern const AtomType AT_2HA;
/** */
IMPATOMEXPORT extern const AtomType AT_CB;
/** */
IMPATOMEXPORT extern const AtomType AT_HB;
/** */
IMPATOMEXPORT extern const AtomType AT_1HB;
/** */
IMPATOMEXPORT extern const AtomType AT_2HB;
/** */
IMPATOMEXPORT extern const AtomType AT_3HB;
/** */
IMPATOMEXPORT extern const AtomType AT_OXT;
/** */
IMPATOMEXPORT extern const AtomType AT_CH3;
/** */
IMPATOMEXPORT extern const AtomType AT_CG;
/** */
IMPATOMEXPORT extern const AtomType AT_CG1;
/** */
/** */ IMPATOMEXPORT extern const AtomType AT_CG2;
/** */ IMPATOMEXPORT extern const AtomType AT_HG;
/** */ IMPATOMEXPORT extern const AtomType AT_1HG;
/** */ IMPATOMEXPORT extern const AtomType AT_2HG;
// IMPATOMEXPORT extern const AtomType AT_HG1;
/** */ IMPATOMEXPORT extern const AtomType AT_1HG1;
/** */ IMPATOMEXPORT extern const AtomType AT_2HG1;
/** */ IMPATOMEXPORT extern const AtomType AT_3HG1;
/** */ IMPATOMEXPORT extern const AtomType AT_1HG2;
/** */ IMPATOMEXPORT extern const AtomType AT_2HG2;
/** */ IMPATOMEXPORT extern const AtomType AT_3HG2;
/** */ IMPATOMEXPORT extern const AtomType AT_OG;
/** */ IMPATOMEXPORT extern const AtomType AT_OG1;
/** */ IMPATOMEXPORT extern const AtomType AT_SG;
// IMPATOMEXPORT extern const AtomType AT_HD1;
// IMPATOMEXPORT extern const AtomType AT_HD2;
/** */ IMPATOMEXPORT extern const AtomType AT_CD;
/** */ IMPATOMEXPORT extern const AtomType AT_CD1;
/** */ IMPATOMEXPORT extern const AtomType AT_CD2;
/** */ IMPATOMEXPORT extern const AtomType AT_HD;
/** */ IMPATOMEXPORT extern const AtomType AT_1HD;
/** */ IMPATOMEXPORT extern const AtomType AT_2HD;
/** */ IMPATOMEXPORT extern const AtomType AT_3HD;
/** */ IMPATOMEXPORT extern const AtomType AT_1HD1;
/** */ IMPATOMEXPORT extern const AtomType AT_2HD1;
/** */ IMPATOMEXPORT extern const AtomType AT_3HD1;
/** */ IMPATOMEXPORT extern const AtomType AT_1HD2;
/** */ IMPATOMEXPORT extern const AtomType AT_2HD2;
/** */ IMPATOMEXPORT extern const AtomType AT_3HD2;
/** */ IMPATOMEXPORT extern const AtomType AT_SD;
/** */ IMPATOMEXPORT extern const AtomType AT_OD1;
/** */ IMPATOMEXPORT extern const AtomType AT_OD2;
/** */ IMPATOMEXPORT extern const AtomType AT_ND1;
/** */ IMPATOMEXPORT extern const AtomType AT_ND2;
/** */ IMPATOMEXPORT extern const AtomType AT_CE;
/** */ IMPATOMEXPORT extern const AtomType AT_CE1;
/** */ IMPATOMEXPORT extern const AtomType AT_CE2;
/** */ IMPATOMEXPORT extern const AtomType AT_CE3;
/** */ IMPATOMEXPORT extern const AtomType AT_HE;
/** */ IMPATOMEXPORT extern const AtomType AT_1HE;
/** */ IMPATOMEXPORT extern const AtomType AT_2HE;
/** */ IMPATOMEXPORT extern const AtomType AT_3HE;
// IMPATOMEXPORT extern const AtomType AT_HE1;
// IMPATOMEXPORT extern const AtomType AT_HE2;
// IMPATOMEXPORT extern const AtomType AT_HE3;
/** */ IMPATOMEXPORT extern const AtomType AT_1HE2;
/** */ IMPATOMEXPORT extern const AtomType AT_2HE2;
/** */ IMPATOMEXPORT extern const AtomType AT_OE1;
/** */ IMPATOMEXPORT extern const AtomType AT_OE2;
/** */ IMPATOMEXPORT extern const AtomType AT_NE;
/** */ IMPATOMEXPORT extern const AtomType AT_NE1;
/** */ IMPATOMEXPORT extern const AtomType AT_NE2;
/** */ IMPATOMEXPORT extern const AtomType AT_CZ;
/** */ IMPATOMEXPORT extern const AtomType AT_CZ2;
/** */ IMPATOMEXPORT extern const AtomType AT_CZ3;
/** */ IMPATOMEXPORT extern const AtomType AT_NZ;
/** */ IMPATOMEXPORT extern const AtomType AT_HZ;
/** */ IMPATOMEXPORT extern const AtomType AT_1HZ;
/** */ IMPATOMEXPORT extern const AtomType AT_2HZ;
/** */ IMPATOMEXPORT extern const AtomType AT_3HZ;
// IMPATOMEXPORT extern const AtomType AT_HZ2;
// IMPATOMEXPORT extern const AtomType AT_HZ3;
/** */ IMPATOMEXPORT extern const AtomType AT_CH2;
/** */ IMPATOMEXPORT extern const AtomType AT_NH1;
/** */ IMPATOMEXPORT extern const AtomType AT_NH2;
/** */ IMPATOMEXPORT extern const AtomType AT_OH;
/** */ IMPATOMEXPORT extern const AtomType AT_HH;
/** */ IMPATOMEXPORT extern const AtomType AT_1HH1;
/** */ IMPATOMEXPORT extern const AtomType AT_2HH1;
/** */ IMPATOMEXPORT extern const AtomType AT_HH2;
/** */ IMPATOMEXPORT extern const AtomType AT_1HH2;
/** */ IMPATOMEXPORT extern const AtomType AT_2HH2;
/** */ IMPATOMEXPORT extern const AtomType AT_1HH3;
/** */ IMPATOMEXPORT extern const AtomType AT_2HH3;
/** */ IMPATOMEXPORT extern const AtomType AT_3HH3;
/** */ IMPATOMEXPORT extern const AtomType AT_P;
/** */ IMPATOMEXPORT extern const AtomType AT_OP1;
/** */ IMPATOMEXPORT extern const AtomType AT_OP2;
/** */ IMPATOMEXPORT extern const AtomType AT_O5p;
/** */ IMPATOMEXPORT extern const AtomType AT_C5p;
/** */ IMPATOMEXPORT extern const AtomType AT_H5p;
/** */ IMPATOMEXPORT extern const AtomType AT_H5pp;
/** */ IMPATOMEXPORT extern const AtomType AT_C4p;
/** */ IMPATOMEXPORT extern const AtomType AT_H4p;
/** */ IMPATOMEXPORT extern const AtomType AT_O4p;
/** */ IMPATOMEXPORT extern const AtomType AT_C1p;
/** */ IMPATOMEXPORT extern const AtomType AT_H1p;
/** */ IMPATOMEXPORT extern const AtomType AT_C3p;
/** */ IMPATOMEXPORT extern const AtomType AT_H3p;
/** */ IMPATOMEXPORT extern const AtomType AT_O3p;
/** */ IMPATOMEXPORT extern const AtomType AT_C2p;
/** */ IMPATOMEXPORT extern const AtomType AT_H2p;
/** */ IMPATOMEXPORT extern const AtomType AT_H2pp;
/** */ IMPATOMEXPORT extern const AtomType AT_O2p;
/** */ IMPATOMEXPORT extern const AtomType AT_HO2p;
/** */ IMPATOMEXPORT extern const AtomType AT_N9;
/** */ IMPATOMEXPORT extern const AtomType AT_C8;
/** */ IMPATOMEXPORT extern const AtomType AT_H8;
/** */ IMPATOMEXPORT extern const AtomType AT_N7;
/** */ IMPATOMEXPORT extern const AtomType AT_C5;
/** */ IMPATOMEXPORT extern const AtomType AT_C4;
/** */ IMPATOMEXPORT extern const AtomType AT_N3;
/** */ IMPATOMEXPORT extern const AtomType AT_C2;
/** */ IMPATOMEXPORT extern const AtomType AT_H2;
/** */ IMPATOMEXPORT extern const AtomType AT_N1;
/** */ IMPATOMEXPORT extern const AtomType AT_C6;
/** */ IMPATOMEXPORT extern const AtomType AT_N6;
/** */ IMPATOMEXPORT extern const AtomType AT_H61;
/** */ IMPATOMEXPORT extern const AtomType AT_H62;
/** */ IMPATOMEXPORT extern const AtomType AT_O6;
/** */ IMPATOMEXPORT extern const AtomType AT_H1;
/** */ IMPATOMEXPORT extern const AtomType AT_N2;
/** */ IMPATOMEXPORT extern const AtomType AT_H21;
/** */ IMPATOMEXPORT extern const AtomType AT_H22;
/** */ IMPATOMEXPORT extern const AtomType AT_H6;
/** */ IMPATOMEXPORT extern const AtomType AT_H5;
/** */ IMPATOMEXPORT extern const AtomType AT_O2;
/** */ IMPATOMEXPORT extern const AtomType AT_N4;
/** */ IMPATOMEXPORT extern const AtomType AT_H41;
/** */ IMPATOMEXPORT extern const AtomType AT_H42;
/** */ IMPATOMEXPORT extern const AtomType AT_H3;
/** */ IMPATOMEXPORT extern const AtomType AT_O4;
/** */ IMPATOMEXPORT extern const AtomType AT_C7;
/** */ IMPATOMEXPORT extern const AtomType AT_H71;
/** */ IMPATOMEXPORT extern const AtomType AT_H72;
/** */ IMPATOMEXPORT extern const AtomType AT_H73;
#endif
/*@}*/


//! A decorator for a particle representing an atom.
/**
   \ingroup hierarchy
   \see Hierarchy
 */
class IMPATOMEXPORT Atom:
  public Hierarchy
{
public:

  IMP_DECORATOR(Atom, Hierarchy)

  Particle* get_particle() const {
    return Hierarchy::get_particle();
  }

  /** Create a decorator with the passed type and coordinates.*/
  static Atom create(Particle *p, AtomType t= AT_UNKNOWN);

  /** Create a decorator by copying from o.*/
  static Atom create(Particle *p, Atom o);

  //! return true if the particle has the needed attributes
  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_type_key())
      && Hierarchy::is_instance_of(p);
  }

  AtomType get_atom_type() const {
    return AtomType(get_particle()->get_value(get_type_key()));
  }

  void set_atom_type(AtomType t);

  IMP_DECORATOR_GET_SET_OPT(charge, get_charge_key(),
                            Float, Float, 0);
  IMP_DECORATOR_GET_SET_OPT(element, get_element_key(),
                            Int, Int, 0);

  /** @name The atom index in the input file
      This index is not necessarily unique over any particular
      set of atoms and so should never be used as an atom identifier
      except during I/O.
      @{
  */
  IMP_DECORATOR_GET_SET_OPT(input_index, get_input_index_key(),
                            Int, Int, -1);
  /* @}*/

  //! shows the Atom record in a PDB format
  /** \param index the atom index. If index eq -1 than the original
             atom index (as read from a PDB file) is used.
      \note  The following fileds are currently not displayed:
             chain, residue insertion code, occupancy,  temp. factor
  */
  std::string get_pdb_string(int index=-1);

  /** @name Keys
      These methods provide access to the various keys used to store
      things in the Atom. These can be used if you want to
      use an attribute to search a set of particles.
      @{
   */
  static IntKey get_type_key();

  static IntKey get_element_key();

  static FloatKey get_charge_key();

  static IntKey get_input_index_key();
  //! @}
};

IMP_OUTPUT_OPERATOR(Atom);

typedef IMP::Decorators<Atom, Hierarchies> Atoms;

//! Return the AtomType from the four letter code in the PDB
/** \throw ValueException if nm is invalid.
    \relatesalso Atom
    \relatesalso AtomType
 */
IMPATOMEXPORT AtomType atom_type_from_pdb_string(std::string nm);


//! Return the index of the residue containing this atom
/** The atom must be part of a molecular hierarchy.
    \relatesalso Atom
 */
IMPATOMEXPORT int get_residue_index(Atom d);

#ifdef SWIG
// ResidueType is a typedef so this is invalid C++ code, but swig needs it
class ResidueType;
class Residue;
#endif

//! Return the type of the residue containing this atom
/** The atom must be part of a molecular hierarchy.
    \relatesalso Atom
    \relatesalso Residue
    \relatesalso Hierarchy
 */
IMPATOMEXPORT ResidueType get_residue_type(Atom d);

//! Return the Residue containing this atom
/** The atom must be part of a molecular hierarchy.
    \relatesalso Atom
    \relatesalso Residue
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Residue get_residue(Atom d);

//! Return a particle atom from the residue
/** The residue must be part of a molecular hierarchy.
    \relatesalso Atom
    \relatesalso Residue
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Atom get_atom(Residue rd, AtomType at);

//! Return the chain id of this atom
/** The atom must be part of a molecular hierarchy.
    \relatesalso Atom
    \relatesalso Residue
    \relatesalso Hierarchy
 */
IMPATOMEXPORT char get_chain(Atom d);


//! Create a new AtomType
/** This creates a new AtomType (returned) and sets up the mapping
    between the AtomType and the proper element.
    \note This method has not been tested. If you use it, please
    write a test and remove this comment.
    \relatesalso AtomType
*/
IMPATOMEXPORT AtomType add_atom_type(std::string name);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ATOM_H */
