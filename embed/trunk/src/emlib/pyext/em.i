%module (directors="1") EM

%apply int *OUTPUT {int *ierr};

%{
#include "../def.h"
#include "../DensityHeader.h"
#include "../DensityMap.h"
#include "../EMReaderWriter.h"
#include "../XplorReaderWriter.h"
#include "../MRCReaderWriter.h"
#include "../SampledDensityMap.h"
#include "../CoarseCC.h"
#include "../CoarseCCatIntervals.h"
#include "../ParticlesAccessPoint.h"
%}

/* Ignore shared object import/export stuff */
#define EMDLLEXPORT
#define EMDLLLOCAL

%include "std_common.i"
%include "std_ios.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_string.i"
%include "std_iostream.i"
%include "cpointer.i"

namespace std {
  %template(vectori) vector<int>;
  %template(vectorf) vector<float>;
}

%feature("director");

/* Explicitly ignore some stuff to prevent SWIG warning about it */
%ignore DensityMap::operator =;
%ignore operator<<(std::ostream&, const DensityHeader &);
%ignore operator<<(ostream&, const EMHeader &);
%ignore operator<<(ostream&, const MRCHeader &);

%include "../def.h"
%include "../MapReaderWriter.h"
%include "../DensityHeader.h"
%include "../DensityMap.h"
%include "../EMReaderWriter.h"
%include "../XplorReaderWriter.h"
%include "../MRCReaderWriter.h"
%include "../SampledDensityMap.h"
%include "../CoarseCC.h"
%include "../CoarseCCatIntervals.h"
%include "../ParticlesAccessPoint.h"
