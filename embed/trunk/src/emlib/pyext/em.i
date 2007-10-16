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