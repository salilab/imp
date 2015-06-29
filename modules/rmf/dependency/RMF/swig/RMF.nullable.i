%pythoncode %{

def _handle_nullable(n):
   if n.get_is_null(): return None
   else: return n.get()
%}

RMF_SWIG_SHOWABLE(RMF, Nullable);

%include "RMF/Nullable.h"

%template(_NullableInt) RMF::Nullable<RMF::Int>;
%template(_NullableFloat) RMF::Nullable<RMF::Float>;
%template(_NullableString) RMF::Nullable<RMF::String>;
%template(_NullableVector3) RMF::Nullable<RMF::Vector<3U> >;
%template(_NullableVector4) RMF::Nullable<RMF::Vector<4U> >;
%template(_NullableInts) RMF::Nullable<RMF::Ints>;
%template(_NullableFloats) RMF::Nullable<RMF::Floats>;
%template(_NullableStrings) RMF::Nullable<RMF::Strings>;
%template(_NullableVector3s) RMF::Nullable<std::vector<RMF::Vector<3U> > >;
