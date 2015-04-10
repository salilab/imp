RMF_SWIG_VALUE_INSTANCE(RMF, Vector3, Vector<3U>, Vector3s);
RMF_SWIG_VALUE_INSTANCE(RMF, Vector4, Vector<4U>, Vector4s);
%extend RMF::Vector<3U> {
 // hack for testing
  bool __eq__(const Vector<3U>& o) const {
    for (unsigned int i = 0; i< 3; ++i) {
      if (self->operator[](i) != o[i]) return false;
    }
    return true;
  }
  bool __ne__(const Vector<3U>&o) const {
    for (unsigned int i = 0; i< 3; ++i) {
      if (self->operator[](i) != o[i]) return true;
    }
    return false;
  }
%pythoncode %{
    def __str__(self):
       return str([x for x in self])
    def __repr__(self):
         return self.__str__()
  %}
 }

%extend RMF::Vector<4U> {
 // hack for testing
  bool __eq__(const Vector<4U>& o) const {
    for (unsigned int i = 0; i< 4; ++i) {
      if (self->operator[](i) != o[i]) return false;
    }
    return true;
  }
  bool __ne__(const Vector<4U>&o) const {
    for (unsigned int i = 0; i< 4; ++i) {
      if (self->operator[](i) != o[i]) return true;
    }
    return false;
  }
%pythoncode %{
    def __str__(self):
       return str([x for x in self])
    def __repr__(self):
         return self.__str__()
  %}
 }

//%implicitconv RMF::Vector;
%include "RMF/Vector.h"
%template(Vector3) RMF::Vector<3U>;
%template(Vector4) RMF::Vector<4U>;
