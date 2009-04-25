// 'print x' should behave similarly to x.show(sys.stdout), except that
// everything is dumped out on a single line, as is typical for Python str().
// Note that repr() is defined by SWIG, so can only be overridden on a class
// by class basis.
%extend IMP::Object {
  %pythoncode {
    def __str__(self):
        import StringIO
        s = StringIO.StringIO()
        self.show(s)
        spl = [x for x in s.getvalue().split('\n') if x != '']
        return '<' + '; '.join(spl) + '>'
  }
}

%include "IMP/Object.h"
