/* Convert char vectors (used for binary data) to and from Python bytes. Note
   that these strings can contain embedded nulls and non-ASCII characters. */
%typemap(typecheck) const std::vector<char> & {
  $1 = PyBytes_Check($input) ? 1 : 0;
}

%typemap(in) const std::vector<char> & {
  char *buf;
  Py_ssize_t len;
  if (PyBytes_AsStringAndSize($input, &buf, &len) == -1) {
    SWIG_fail;
  } else {
    $1 = new std::vector<char>(len);
    std::copy(buf, buf + len, $1->begin());
  }
}
%typemap(freearg) const std::vector<char> & {
  delete $1;
}

%typemap(out) const std::vector<char> & {
  $result = PyBytes_FromStringAndSize(&($1->front()), $1->size());
}

%extend RMF::BufferConstHandle {
  %pythoncode %{
     def __getstate__(self):
       return self.get_buffer()

     def __setstate__(self, buff):
       self.__init__(buff)
       return self
     %}
  }

%include "RMF/BufferConstHandle.h"
%include "RMF/BufferHandle.h"
