/**
 *  \file bond_decorators.h     \brief Contains decorators for a bond
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BOND_DECORATORS_H
#define __IMP_BOND_DECORATORS_H


#include "../Particle.h"
#include "../Model.h"
#include "../DecoratorBase.h"
#include "../internal/graph_base.h"
#include "utility.h"

namespace IMP
{

class BondDecorator;
class BondedDecorator;

/** \defgroup bond Creating and restraining bonds
    A set of classes and functions for manipulating bonds.
 */

namespace internal
{
class GraphData;
extern IMPDLLEXPORT  GraphData bond_graph_data_;
extern IMPDLLEXPORT  bool bond_keys_initialized_;
extern IMPDLLEXPORT  IntKey bond_type_key_;
extern IMPDLLEXPORT  IntKey bond_order_key_;
extern IMPDLLEXPORT  FloatKey bond_length_key_;
extern IMPDLLEXPORT  FloatKey bond_stiffness_key_;

} // namespace internal

//! A decorator for wrapping a particle representing a molecular bond
/**
   As with AtomDecorator, the types of bonds will eventually be run-time
   expandible.

   \ingroup bond Bonds
 */
class IMPDLLEXPORT BondDecorator: public DecoratorBase
{

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
 */
class IMPDLLEXPORT BondedDecorator: public DecoratorBase
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
 */
IMPDLLEXPORT
BondDecorator bond(BondedDecorator a, BondedDecorator b, Int t);


//! Connect the two wrapped particles by a custom bond.
/** \param[in] a The first Particle as a BondedDecorator
    \param[in] b The second Particle as a BondedDecorator
    \param[in] length The length of the bond.
    \param[in] stiffness The stiffness of the bond.
    \return BondDecorator of the bond Particle.

    \ingroup bond
 */
IMPDLLEXPORT
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
 */
IMPDLLEXPORT
void unbond(BondDecorator b);

//! Get the bond between two particles.
/**
   BondDecorator() is returned if the particles are not bonded.
   \ingroup bond
 */
IMPDLLEXPORT
BondDecorator get_bond(BondedDecorator a, BondedDecorator b);

} // namespace IMP

#endif  /* __IMP_BOND_DECORATORS_H */
