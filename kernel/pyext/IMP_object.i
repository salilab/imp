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

%include "IMP/Object.h"
