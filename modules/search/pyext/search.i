%module(directors="1") "IMP.search"

%{
#include "IMP.h"
#include "IMP/search.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "search_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"


/* Wrap our own classes */
%include "IMP/search/AttributeMap1ScoreState.h"
%include "IMP/search/AttributeMap2ScoreState.h"
%include "IMP/search/AttributeMap3ScoreState.h"
%include "IMP/search/AttributeMap4ScoreState.h"

IMP_OWN_FIRST_CONSTRUCTOR(IntBase)

namespace IMP {
  namespace search {
    %template(IntBase) AttributeMap1ScoreState<Int>;
  }
}

namespace IMP {
  namespace search {
    IMP_OWN_FIRST_CONSTRUCTOR(IntAttributeMapScoreState)
  }
}
%include "IMP/search/IntAttributeMapScoreState.h"


