%{
namespace {
void handle_imp_exception(void) {
    try {
      throw;
    } catch (const RMF::IndexException &e) {
      PyErr_SetString(PyExc_IndexError, e.what());
    } catch (const RMF::Exception &e) {
      // for windows where there may be multiple std::exceptions
      // the order  is to avoid warnings about redundancy
      PyErr_SetString(PyExc_IOError, e.what());
    } catch (const std::domain_error &e) {
      PyErr_SetString(PyExc_ValueError, e.what());
    } catch (const std::runtime_error &e) {
      PyErr_SetString(PyExc_RuntimeError, e.what());
    } catch (const std::invalid_argument &e) {
      PyErr_SetString(PyExc_TypeError, e.what());
    } catch (const std::exception &e) {
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
