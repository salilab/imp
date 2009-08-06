%module(directors="1") "IMP.search"

%{
#include "IMP.h"
#include "IMP/search.h"
%}


%include "IMP_macros.i"
%include "IMP_exceptions.i"

%include "modules/search/pyext/search_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"
%import "IMP_keys.i"

/* Wrap our own classes */
%include "IMP/search/AttributeMapScoreState.h"


namespace IMP {
  namespace search {
    /*%template(IntMapKey) AttributeMapScoreState<IntKey>;
    %template(IntMapScoreState) AttributeMapScoreState<boost::tuple<IntKey> >;
    %template(IntMapValue) IntMapScoreState::Value;*/
  }
}
