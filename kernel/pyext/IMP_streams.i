// Typemaps to allow Python file-like objects to be used for C++ code that
// expects a std::ostream

%typemap(in) std::ostream& (PyFileAdapter *tmp=NULL) {
  tmp = new PyFileAdapter($input);
  $1 = new std::ostream(tmp);
}
%typemap(freearg) std::ostream& {
  if ($1) delete $1;
  if (tmp$argnum) delete tmp$argnum;
}

%typemap(typecheck) std::ostream& = PyObject *;

%{
// Adapter class that acts like a std::streambuf but delegates to a Python
// file-like object, p
class PyFileAdapter : public std::streambuf
{
  PyObject *p_;
public:
  PyFileAdapter(PyObject *p) : p_(p) { Py_XINCREF(p_); }

  virtual ~PyFileAdapter() { Py_XDECREF(p_); }

protected:
  virtual int_type overflow(int_type c) {
    if (c != EOF) {
      char buf[2];
      buf[0] = static_cast<char>(c); buf[1] = '\0';
      xsputn(buf, 1);
    }
    return c;
  }

  virtual std::streamsize xsputn(const char *s, std::streamsize num) {
    // Python API uses char* arguments rather than const char*, so create
    // here to quell the compiler warning
    static char method[] = "write";
    static char fmt[] = "(s#)";
    PyObject *result = PyObject_CallMethod(p_, method, fmt, s, num);
    if (!result) {
      // Python exception will be reraised when SWIG method finishes
      throw Swig::DirectorMethodException();
    } else {
      Py_DECREF(result);
    }
    return num;
  }
};
%}
