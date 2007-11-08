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

%include "IMP/Index.h"
%include "IMP/Base_Types.h"
%include "IMP.h"
%include "IMP/boost/noncopyable.h"
%include "IMP/ScoreFunc.h"
%include "IMP/ModelData.h"
%include "IMP/DerivativeAccumulator.h"
%include "IMP/restraints/Restraint.h"
%include "IMP/restraints/RestraintSet.h"
%include "IMP/State.h"
%include "IMP/Model.h"
%include "IMP/Particle.h"
%include "IMP/optimizers/Optimizer.h"
%include "IMP/optimizers/SteepestDescent.h"
%include "IMP/optimizers/ConjugateGradients.h"
%include "IMP/restraints/DistanceRestraint.h"
%include "IMP/restraints/TorusRestraint.h"
%include "IMP/restraints/CoordinateRestraint.h"
%include "IMP/restraints/ProximityRestraint.h"
%include "IMP/restraints/ConnectivityRestraint.h"
%include "IMP/restraints/PairConnectivityRestraint.h"
%include "IMP/restraints/ExclusionVolumeRestraint.h"
%include "IMP/RigidBody.h"

namespace IMP {
  %template(IntIndex) Index<Int>;
  %template(FloatIndex) Index<Float>;
  %template(StringIndex) Index<String>;
  %template(ParticleIndex) Index<ParticleTag>;
  %template(RestraintIndex) Index<RestraintTag>;
  %template(StateIndex) Index<StateTag>;
}
