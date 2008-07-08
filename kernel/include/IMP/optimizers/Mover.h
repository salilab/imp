/**
 *  \file Mover.h    \brief The base class for movers for MC optimization.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MOVER_H
#define __IMP_MOVER_H

#include "../IMP_config.h"
#include "../base_types.h"
#include "../Object.h"
#include "../Pointer.h"
#include "../Optimizer.h"

#include <vector>

namespace IMP
{

class MonteCarlo;
class Mover;
typedef Index<Mover> MoverIndex;

/** \ingroup optimizer
    \addtogroup mover Monte Carlo movers
    Classes which move particles around. Currently used
    for MonteCarlo optimization.
 */

//! A class to make a monte carlo move.
/** You probably want to use MoverBase if you are implementing a Mover.
 */
class IMPDLLEXPORT Mover: public Object
{
  friend class MonteCarlo;
  void set_optimizer(Optimizer *c, MoverIndex i) {
    opt_=c;
    index_=i;
  }

  Pointer<Optimizer> opt_;
  MoverIndex index_;
public:
  Mover();
  virtual ~Mover(){};

  //! propose a modification
  /** \param[in] size A number between 0 and 1 used to scale the proposed 
      moves. This number can be either used to scale a continuous move
      or affect the probability of a discrete move.
   */
  virtual void propose_move(float size)=0;
  //! set whether the proposed modification is accepted
  /** \note Accepting should not change the Particles at all.
   */
  virtual void accept_move()=0;

  //! Roll back any changes made to the Particles
  virtual void reject_move()=0;

  //! Get a pointer to the optimizer which has this mover.
  Optimizer *get_optimizer() const {
    IMP_CHECK_OBJECT(this);
    return opt_.get();
  }
  MoverIndex get_index() const {
    IMP_assert(index_!= MoverIndex(), "Call set_optimizer first");
    return index_;
  }
  virtual void show(std::ostream&out= std::cout) const {
    out << "Mover doesn't implement show " << std::endl;
  }
};

IMP_OUTPUT_OPERATOR(Mover);

} // namespace IMP

#endif  /* __IMP_MOVER_H */
