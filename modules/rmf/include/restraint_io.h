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
#include "link_macros.h"
#include <IMP/base/object_macros.h>
#include <IMP/Restraint.h>
#include <IMP/restraint_macros.h>

IMPRMF_BEGIN_NAMESPACE


class RMFRestraint;
IMP_OBJECTS(RMFRestraint, RMFRestraints);
/** A dummy restraint object to represent restraints loaded from
    an RMF file.*/
class IMPRMFEXPORT RMFRestraint: public Restraint {
  ParticlesTemp ps_;
  RMFRestraints decomp_;
public:
#ifndef IMP_DOXYGEN
  RMFRestraint(Model *m, std::string name);
  void set_score(double s) {
    set_last_score(s);
  }
  void set_particles(const ParticlesTemp &ps) {ps_=ps;}
  void set_decomposition(const RMFRestraints &d);
#endif
  IMP_RESTRAINT(RMFRestraint);
  Restraints do_create_current_decomposition() const;
};



/** \name Restraint I/O
    Restraints are written as am RMF::FEATURE node with subnodes for
    the decomposed restraints (Restraint::create_current_decompositon()).
    The Restraints::get_last_score() value is what is saved to the file,
    so make sure that the restraints have been evaluated before saving
    a frame.
    @{
*/
IMP_DECLARE_LINKERS(Restraint, restraint, restraints,
                   RMFRestraint*, RMFRestraints,
                   Restraint*, RestraintsTemp,
                   (RMF::FileHandle fh),
                   (RMF::FileConstHandle fh,
                    Model *m));

/** @} */


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_RESTRAINT_IO_H */
