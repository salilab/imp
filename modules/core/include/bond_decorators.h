/**
 *  \file bond_decorators.h     \brief Contains decorators for a bond
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_BOND_DECORATORS_H
#define IMPCORE_BOND_DECORATORS_H

#include "core_exports.h"
#include "utility.h"
#include "internal/graph_base.h"

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/DecoratorBase.h>

IMPCORE_BEGIN_NAMESPACE

class BondDecorator;
class BondedDecorator;

/** \defgroup bond Creating and restraining bonds
    A set of classes and functions for manipulating bonds.
 */

namespace internal
{
extern IMPCOREEXPORT internal::GraphData bond_graph_data_;
extern IMPCOREEXPORT  bool bond_keys_initialized_;
extern IMPCOREEXPORT  IntKey bond_type_key_;
extern IMPCOREEXPORT  IntKey bond_order_key_;
extern IMPCOREEXPORT  FloatKey bond_length_key_;
extern IMPCOREEXPORT  FloatKey bond_stiffness_key_;

} // namespace internal


class BondedDecorator;

//! A decorator for wrapping a particle representing a molecular bond
/**
   As with AtomDecorator, the types of bonds will eventually be run-time
   expandible.

   \ingroup bond
   \ingroup decorators
 */
class IMPCOREEXPORT BondDecorator: public DecoratorBase
{
  friend class BondedDecorator;

  /* Do not initialize the attributes as we don't want people
     creating bonds directly.
  */
  IMP_DECORATOR(BondDecorator, DecoratorBase,
                return internal::graph_is_edge(p, internal::bond_graph_data_),
               );

public:
  //! The types a bond can have right now
  enum Type {UNKNOWN=-1,
             COVALENT, HYDROGEN, DISULPHIDE, SALT, PEPTIDE,
             CUSTOM
            };

  //! Get the atom i of the bond
  /** \param[in] i 0 or 1
      \return BondedDecorator for the atom in question
  */
  BondedDecorator get_bonded(unsigned int i) const ;

  IMP_DECORATOR_GET_SET_OPT(type, internal::bond_type_key_, Int, Int,
                            UNKNOWN);

  IMP_DECORATOR_GET_SET_OPT(order, internal::bond_order_key_, Int, Int, 1);

  IMP_DECORATOR_GET_SET_OPT(length, internal::bond_length_key_, Float,
                            Float, -1);
  IMP_DECORATOR_GET_SET_OPT(stiffness, internal::bond_stiffness_key_, Float,
                            Float, -1);
};

IMP_OUTPUT_OPERATOR(BondDecorator);



//! A decorator for a particle which has bonds.
/** \ingroup bond
    \ingroup decorators
 */
class IMPCOREEXPORT BondedDecorator: public DecoratorBase
{
  IMP_DECORATOR(BondedDecorator, DecoratorBase,
                return internal::graph_is_node(p, internal::bond_graph_data_),
                graph_initialize_node(p, internal::bond_graph_data_));


public:

  unsigned int get_number_of_bonds() const {
    return graph_get_number_of_edges(get_particle(),
                                     internal::bond_graph_data_);
  }


  //! Get a BondDecorator of the ith bond
  /** \return decorator of the ith child, or throw an exception if there
              is no such bond
  */
  BondDecorator get_bond(unsigned int i) const {
    Particle *p= graph_get_edge(get_particle(), i,
                                     internal::bond_graph_data_);
    return BondDecorator(p);
  }

  //! Get a BondedDecorator of the ith bonded particle
  /** \return decorator of the ith child, or throw an exception if there
              is no such bond

      \note I don't entirely like having this here as it duplicates
      functionality available otherwise, however it is such a fundamental
      operation and kind of a pain to write. It also means that we
      could later pull the edge endpoints into the vertex if
      desired.
  */
  BondedDecorator get_bonded(unsigned int i) const {
    Particle *p= graph_get_edge(get_particle(), i, internal::bond_graph_data_);
    BondDecorator bd(p);
    if (bd.get_bonded(0) == *this) return bd.get_bonded(1);
    else return bd.get_bonded(0);
  }
};

IMP_OUTPUT_OPERATOR(BondedDecorator);


typedef std::vector<BondDecorator> BondDecorators;

inline BondedDecorator BondDecorator::get_bonded(unsigned int i) const
{
  Particle *p= graph_get_node(get_particle(), i,
                              internal::bond_graph_data_);
  return BondedDecorator(p);
}

//! Connect the two wrapped particles by a bond.
/** \param[in] a The first Particle as a BondedDecorator
    \param[in] b The second Particle as a BondedDecorator
    \param[in] t The type to use for the bond
    \return BondDecorator of the bond Particle.

    \ingroup bond
    \relates BondDecorator
    \relates BondedDecorator
 */
IMPCOREEXPORT
BondDecorator bond(BondedDecorator a, BondedDecorator b, Int t);


//! Connect the two wrapped particles by a custom bond.
/** \param[in] a The first Particle as a BondedDecorator
    \param[in] b The second Particle as a BondedDecorator
    \param[in] length The length of the bond.
    \param[in] stiffness The stiffness of the bond.
    \return BondDecorator of the bond Particle.

    \ingroup bond
    \relates BondDecorator
    \relates BondedDecorator
 */
IMPCOREEXPORT
inline BondDecorator custom_bond(BondedDecorator a, BondedDecorator b,
                          Float length, Float stiffness=-1) {
  IMP_assert(length>=0, "Length must be positive");
  BondDecorator bd=bond(a,b, BondDecorator::CUSTOM);
  bd.set_length(length);
  if (stiffness >=0) bd.set_stiffness(stiffness);
  return bd;
}


//! Destroy the bond connecting to particles.
/** \param[in] b The bond.
    \ingroup bond
    \relates BondDecorator
    \relates BondedDecorator
 */
IMPCOREEXPORT
void unbond(BondDecorator b);

//! Get the bond between two particles.
/** BondDecorator() is returned if the particles are not bonded.
    \ingroup bond
    \relates BondDecorator
    \relates BondedDecorator
 */
IMPCOREEXPORT
BondDecorator get_bond(BondedDecorator a, BondedDecorator b);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_DECORATORS_H */
