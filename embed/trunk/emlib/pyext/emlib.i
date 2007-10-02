%module(directors="1") emlib
%{
#include "../def.h"
#include "../MapReaderWriter.h"
#include "../DensityHeader.h"
#include "../Map3D.h"
#include "../DensityMap.h"
#include "../EMReaderWriter.h"
#include "../XplorReaderWriter.h"
%}

%include "std_common.i"
%include "std_ios.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_string.i"
%include "std_iostream.i"


namespace std {
  %template(vectori) vector<int>;
  %template(vectorf) vector<float>;
}





%feature("director");

%include "../def.h"
%include "../MapReaderWriter.h"
%include "../DensityHeader.h"
%include "../Map3D.h"
%template(floatMap3D) Map3D<float>;
%include "../DensityMap.h"
%include "../EMReaderWriter.h"
%include "../XplorReaderWriter.h"


