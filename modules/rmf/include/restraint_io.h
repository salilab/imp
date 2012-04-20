/**
 *  \file IMP/rmf/restraint_io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_RESTRAINT_IO_H
#define IMPRMF_RESTRAINT_IO_H

#include "rmf_config.h"
#include <IMP/base/Object.h>
#include <IMP/base/object_macros.h>
#include <RMF/NodeHandle.h>
#include <IMP/Restraint.h>
#include <IMP/restraint_macros.h>

IMPRMF_BEGIN_NAMESPACE


class RMFRestraint;
IMP_OBJECTS(RMFRestraint, RMFRestraints);
/** A dummy restraint object to represent restraints loaded from
    an RMF file.*/
class IMPRMFEXPORT RMFRestraint: public Restraint {
  double score_;
  ParticlesTemp ps_;
  RMFRestraints decomp_;
public:
#ifndef IMP_DOXYGEN
  RMFRestraint(Model *m, std::string name);
  void set_score(double s) {score_=s;}
  void set_particles(const ParticlesTemp &ps) {ps_=ps;}
  void set_decomposition(const RMFRestraints &d) {
    decomp_=d;
  }
#endif
  IMP_RESTRAINT(RMFRestraint);
  Restraints do_create_current_decomposition() const;
};



/** \name Restraint I/O
    Restraint I/O is quite limited as it is not practical to write enough
    information to the file to recreate actual IMP::Restraint objects
    at the moment. Instead, all that is written is the name, the particles
    used and the score. Restraints that can be decomposed are then
    decomposed and their the decomposed restraints are written as children.
    When the decomposition changes from frame to frame (eg restraints on
    IMP::container::ClosePairContainer containers), the list of particles
    will be empty for frames where that bit of the decomposed restraint
    is not found.
    @{
*/
/** Add a restraint to the file. This does not save the score.*/
IMPRMFEXPORT void add_restraints(RMF::FileHandle parent,
                                 const RestraintsTemp&r);

#ifndef IMP_DOXYGEN
IMPRMFEXPORT void add_restraint(RMF::FileHandle parent, Restraint *r);
#endif

IMPRMFEXPORT
RMFRestraints create_restraints(RMF::FileConstHandle fh, Model *m);

/** @} */


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_RESTRAINT_IO_H */
