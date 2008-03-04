/**
 *  \file DihedralRestraint.h  \brief Dihedral restraint between four particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_DIHEDRAL_RESTRAINT_H
#define __IMP_DIHEDRAL_RESTRAINT_H

#include "../IMP_config.h"
#include "../UnaryFunction.h"
#include "../Restraint.h"
#include "../internal/kernel_version_info.h"


namespace IMP
{

//! Dihedral restraint between four particles
class IMPDLLEXPORT DihedralRestraint : public Restraint
{
public:
  //! Create the dihedral restraint.
  /** \param[in] score_func Scoring function for the restraint.
      \param[in] p1 Pointer to first particle in dihedral restraint.
      \param[in] p2 Pointer to second particle in dihedral restraint.
      \param[in] p3 Pointer to third particle in dihedral restraint.
      \param[in] p4 Pointer to fourth particle in dihedral restraint.
   */
  DihedralRestraint(UnaryFunction* score_func,
                    Particle* p1, Particle* p2, Particle* p3, Particle *p4);
  virtual ~DihedralRestraint();

  IMP_RESTRAINT(internal::kernel_version_info)

protected:
  //! scoring function for this restraint
  UnaryFunction* score_func_;
};

} // namespace IMP

#endif  /* __IMP_DIHEDRAL_RESTRAINT_H */
