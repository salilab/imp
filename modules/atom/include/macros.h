/**
 *  \file atom/macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_MACROS_H
#define IMPATOM_MACROS_H

#define IMP_ATOM_TYPE_INDEX 8974343
#define IMP_RESIDUE_TYPE_INDEX 90784334

//! Define the basic things you need for a ForceFieldParameters.
/** In addition to the methods done by all the macros, it declares
    - IMP::Restraint::evaluate()
    - IMP::Restraint::incremental_evaluate()
    and it defines
    - IMP::Restraint::get_is_incremental() to return true
*/
#define IMP_FORCE_FIELD_PARAMETERS(Name, version_info)  \
  IMP_OBJECT(Name, version_info)

#endif  /* IMPATOM_MACROS_H */
