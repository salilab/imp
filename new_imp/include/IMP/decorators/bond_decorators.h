/**
 *  \file bond_decorators.h     \brief Contains decorators for a bond
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BOND_DECORATORS_H
#define __IMP_BOND_DECORATORS_H


#include "../Particle.h"
#include "../Model.h"
#include "graph_base.h"

namespace IMP
{

class BondDecorator;
class BondedDecorator;

//! Connect the two wrapped particles by a bond.
/** \param[in] a The first Particle as a BondedDecorator
    \param[in] b The second Particle as a BondedDecorator
    \return BondDecorator of the bond Particle.
 */
IMPDLLEXPORT BondDecorator bond(BondedDecorator a, BondedDecorator b);


namespace internal {

extern IMPDLLEXPORT  GraphData bond_graph_data_;
extern IMPDLLEXPORT  bool bond_keys_initialized_;
extern IMPDLLEXPORT  FloatKey bond_length_key_;
extern IMPDLLEXPORT  IntKey bond_type_key_;
extern IMPDLLEXPORT  IntKey bond_order_key_;

} // namespace internal

//! A decorator for wrapping a particle representing a molecular bond
class IMPDLLEXPORT BondDecorator
{

  IMP_DECORATOR(BondDecorator,
                return internal::graph_is_edge(p, internal::bond_graph_data_),
               );

public:
  //! The types a bond can have right now
  enum Type {UNKNOWN=-1,
             COVALENT, HYDROGEN, DISULPHINE, SALT, PEPTIDE
            };

  //! Get the atom i of the bond
  /** \param[in] i 0 or 1
      \return BondedDecorator for the atom in question
  */
  BondedDecorator get_atom(unsigned int i) const ;


  IMP_DECORATOR_GET_SET_OPT(length, internal::bond_length_key_, Float,
                            Float, -1);

  IMP_DECORATOR_GET_SET_OPT(type, internal::bond_type_key_, Int, Type, UNKNOWN);

  IMP_DECORATOR_GET_SET_OPT(order, internal::bond_order_key_, Int, Int, 1);


};

IMP_OUTPUT_OPERATOR(BondDecorator);



//! A decorator for a particle which has bonds.
class IMPDLLEXPORT BondedDecorator
{
  IMP_DECORATOR(BondedDecorator, return true, );


public:

  int get_number_of_bonds() const {
    return graph_get_number_of_edges(get_particle(),
                                     internal::bond_graph_data_);
  }


  //! Get a BondDecorator of the ith child
  /** \return decorator of the ith child, or throw and exception if there
              is no such bond
  */
  BondDecorator get_bond(unsigned int i) const {
    ParticleIndex pi= graph_get_edge(get_particle(), i,
                                     internal::bond_graph_data_);
    Particle *p= get_model()->get_particle(pi);
    return BondDecorator::cast(p);
  }
};

IMP_OUTPUT_OPERATOR(BondedDecorator);



BondedDecorator BondDecorator::get_atom(unsigned int i) const
{
  ParticleIndex pi= graph_get_node(get_particle(), i,
                                   internal::bond_graph_data_);
  Particle *p= get_model()->get_particle(pi);
  return BondedDecorator::cast(p);
}

} // namespace IMP

#endif  /* __IMP_BOND_DECORATORS_H */
