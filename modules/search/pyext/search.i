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

namespace IMP {
  namespace search {
    IMP_OWN_FIRST_CONSTRUCTOR(IntMapScoreState)
  }
}


/* Wrap our own classes */
%include "IMP/search/AttributeMapScoreState.h"


namespace IMP {
  namespace search {
    /*%template(IntMapKey) AttributeMapScoreState<IntKey>;
    %template(IntMapScoreState) AttributeMapScoreState<boost::tuple<IntKey> >;
    %template(IntMapValue) IntMapScoreState::Value;*/
  }
}


