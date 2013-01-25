%{
namespace {
void handle_imp_exception(void) {
    try {
      throw;
    } catch (const std::runtime_error &e) {
      PyErr_SetString(PyExc_IOError, e.what());
    } catch (...) {
      PyErr_SetString(PyExc_IOError,
                      "Unknown error caught by Python wrapper");
    }
}
}
%}

%exception {
  try {
    $action
  } catch (...) {
    // If Python error indicator is set (e.g. from a failed director method),
    // it will be reraised at the end of the method
    if (!PyErr_Occurred()) {
      handle_imp_exception();
    }
    SWIG_fail;
  }
}
