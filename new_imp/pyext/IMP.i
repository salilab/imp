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
%include "std_except.i"

%include "typemaps.i"
%include "cstring.i"

namespace std {
  %template(vectori) vector<int>;
  %template(vectorf) vector<float>;
}

%feature("director");

%apply int *OUTPUT { int *magic };
%apply int *OUTPUT { int *ierr };
%apply int *OUTPUT { int *type };
%apply int *OUTPUT { int *dimx};
%apply int *OUTPUT { int *dimy};
%apply int *OUTPUT { int *dimz};
%cstring_output_allocate(char **comment, free(*$1));
%apply float *OUTPUT { float *voltage, float *Cs, float *Aperture, float *Magnification, float *Postmagnification, float *Exposuretime, float *Objectpixelsize, float *Microscope, float *Pixelsize, float *CCDArea, float *Defocus, float *Astigmatism, float *AstigmatismAngle, float *FocusIncrement, float *CountsPerElectron, float *Intensity, float *EnergySlitwidth, float *EnergyOffset, float *Tiltangle, float *Tiltaxis, float *Marker_X, float *Marker_Y, float *meanval}

namespace IMP {
%catches(std::out_of_range) Particle::get_float_index;
%catches(std::out_of_range) Particle::get_int_index;
%catches(std::out_of_range) Particle::get_string_index;
}

%include "emfile.h"
%include "emscore.h"
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
%include "restraints/CoarseEMRestraint.h"
%include "RigidBody.h"

namespace IMP {
  %template(IntIndex) Index<Int>;
  %template(FloatIndex) Index<Float>;
  %template(StringIndex) Index<String>;
  %template(ParticleIndex) Index<ParticleTag>;
  %template(RestraintIndex) Index<RestraintTag>;
  %template(StateIndex) Index<StateTag>;
}
