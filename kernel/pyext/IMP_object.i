// 'print x' should behave similarly to x.show(sys.stdout), except that
// everything is dumped out on a single line, as is typical for Python str().
// Note that repr() is defined by SWIG, so can only be overridden on a class
// by class basis.
%extend IMP::Object {
  %pythoncode {
    def __str__(self):
        return self.get_name()
  }
}
%extend IMP::Object {
    // Two objects count as equal in Python only if their memory
    // addresses are the same
    bool __eq__(const Object &other) {
      return self == &other;
    }
    bool __ne__(const Object &other) {
      return self != &other;
    }
  }

%include "IMP/Object.h"
