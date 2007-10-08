%module(directors="1") imp2
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

namespace imp {
%catches(std::out_of_range) Particle::float_index;
%catches(std::out_of_range) Particle::int_index;
%catches(std::out_of_range) Particle::string_index;
}

%include "emfile.h"
%include "emscore.h"
%include "Base_Types.h"
%include "IMP.h"
%include "Score_Func.h"
%include "Model_Data.h"
%include "Restraint.h"
%include "Restraint_Set.h"
%include "Model.h"
%include "Particle.h"
%include "Optimizer.h"
%include "Steepest_Descent.h"
%include "Conjugate_Gradients.h"
%include "Complexes_Restraints.h"
%include "Rigid_Body.h"
