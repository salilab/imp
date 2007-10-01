%module(directors="1") imp2
%{
#include "IMP.h"
#include "Particle.h"
#include "Optimizer.h"
#include "Score_Func.h"
#include "Restraint_Set.h"
#include "Restraint.h"
#include "Complexes_Restraints.h"
#include "Rigid_Body.h"
#include "Model_Data.h"
#include "Model.h"
%}

%include "std_vector.i"
%include "std_map.i"
%include "std_string.i"

namespace std {
  %template(vectori) vector<int>;
  %template(vectorf) vector<float>;
}

%feature("director");

%include "IMP.h"
%include "Model.h"
%include "Model_Data.h"
%include "Particle.h"
%include "Optimizer.h"
%include "Score_Func.h"
%include "Restraint_Set.h"
%include "Complexes_Restraints.h"
%include "Restraint.h"
%include "Rigid_Body.h"
