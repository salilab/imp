// Simple wrapper to give read-only access to IMP's random number generator

namespace IMP {
  class RandomNumberGenerator {
  public:
    //! Set the random number generator seed
    void seed(int x);

    //! Get the next random value from the generator
    int operator()();
  };
  const RandomNumberGenerator random_number_generator;
};

%extend IMP::RandomNumberGenerator {
  //! Get the current state as a binary blob (for serialization)
  PyObject *get_state() const {
    std::ostringstream oss;
    cereal::BinaryOutputArchive ba(oss);
    ba(*self);
    std::string s = oss.str();
    PyObject *p = PyBytes_FromStringAndSize(s.data(), s.size());
    if (p) {
      return p;
    } else {
      throw IMP::IndexException("PyBytes_FromStringAndSize failed");
    }
  }

  //! Set the current state from a binary blob (for unserialization)
  void set_state(PyObject *p) {
    char *buf;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize(p, &buf, &len) < 0) {
      throw IMP::IndexException("PyBytes_AsStringAndSize failed");
    }
    std::string s(buf, len);
    std::istringstream iss(s);
    cereal::BinaryInputArchive ba(iss);
    ba(*self);
  }
}

%include "IMP/random_utils.h"
%inline %{
  // Provide non-templated versions for Python, and return vectors (rather
  // than taking a vector reference)
  namespace IMP {
    Vector<float> get_random_floats_normal(unsigned int n, float mean=0.0,
                                           float stddev=1.0) {
      Vector<float> ret;
      get_random_numbers_normal<float>(ret, n, mean, stddev);
      return ret;
    }
    Vector<double> get_random_doubles_normal(unsigned int n, double mean=0.0,
                                             double stddev=1.0) {
      Vector<double> ret;
      get_random_numbers_normal<double>(ret, n, mean, stddev);
      return ret;
    }
    Vector<float> get_random_floats_uniform(unsigned int n) {
      Vector<float> ret;
      get_random_numbers_uniform<float>(ret, n);
      return ret;
    }
    Vector<double> get_random_doubles_uniform(unsigned int n) {
      Vector<double> ret;
      get_random_numbers_uniform<double>(ret, n);
      return ret;
    }
  }
%}

%include "IMP/random.h"
