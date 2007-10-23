%module(directors="1") IMP
%{
#include "IMP.h"
%}

/* Ignore shared object import/export stuff */
#define IMPDLLEXPORT
#define IMPDLLLOCAL

%include "std_vector.i"
%include "std_map.i"
%include "std_string.i"

%include "IMP_exceptions.i"

namespace std {
  %template(vectori) vector<int>;
  %template(vectorf) vector<float>;
}

%feature("director");

%include "Index.h"
%include "Base_Types.h"
%include "IMP.h"
%include "boost/noncopyable.h"
%include "ScoreFunc.h"
%include "ModelData.h"
%include "restraints/Restraint.h"
%include "restraints/RestraintSet.h"
%include "State.h"
%include "Model.h"
%include "Particle.h"
%include "optimizers/Optimizer.h"
%include "optimizers/SteepestDescent.h"
%include "optimizers/ConjugateGradients.h"
%include "restraints/DistanceRestraint.h"
%include "restraints/TorusRestraint.h"
%include "restraints/CoordinateRestraint.h"
%include "restraints/ProximityRestraint.h"
%include "restraints/ConnectivityRestraint.h"
%include "restraints/PairConnectivityRestraint.h"
%include "restraints/ExclusionVolumeRestraint.h"
%include "RigidBody.h"

namespace IMP {
  %template(IntIndex) Index<Int>;
  %template(FloatIndex) Index<Float>;
  %template(StringIndex) Index<String>;
  %template(ParticleIndex) Index<ParticleTag>;
  %template(RestraintIndex) Index<RestraintTag>;
  %template(StateIndex) Index<StateTag>;
}
