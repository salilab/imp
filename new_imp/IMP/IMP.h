/*
 *  IMP.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#if !defined(__imp_h)
#define __imp_h 1

#include "IMP_config.h"

namespace imp
{

class Model;

extern IMPDLLEXPORT Model& get_model(void);

} // namespace imp

#include "emfile.h"
#include "emscore.h"
#include "log.h"
#include "Base_Types.h"
#include "Particle.h"
#include "Optimizer.h"
#include "Score_Func.h"
#include "Restraint.h"
#include "Complexes_Restraints.h"
#include "Restraint_Set.h"
#include "Rigid_Body.h"
#include "Model_Data.h"
#include "Model.h"
#include "Model_Loader.h"

#endif
