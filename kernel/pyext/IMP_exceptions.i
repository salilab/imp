/* IMP exception handling */

/* Runtime check functions from exception.h */
namespace IMP
{
enum CheckLevel {NONE=0, CHEAP=1, EXPENSIVE=2};
void set_check_level(CheckLevel tf);
CheckLevel get_check_level();
void set_print_exceptions(bool tf);
}

/* Python prints exception messages */                        
%pythoncode %{                                                
set_print_exceptions(False)
%}

%{
  /* Code to convert C++ exceptions into scripting language errors. Saves
     having lots of catch statements in every single wrapper. */
  static void handle_imp_exception(void)
  {
    try {
      throw;
    } catch (std::out_of_range &e) {
      SWIG_exception(SWIG_IndexError, e.what());
    } catch (std::domain_error &e) {
      SWIG_exception(SWIG_ValueError, e.what());
    } catch (IMP::IndexException &e) {
      SWIG_exception(SWIG_IndexError, e.what());
    } catch (IMP::InvalidStateException &e) {
      SWIG_exception(SWIG_ValueError, e.what());
    } catch (IMP::InactiveParticleException &e) {
      SWIG_exception(SWIG_ValueError, e.what());
    } catch (IMP::ErrorException &e) {
      SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (IMP::ValueException &e) {
      SWIG_exception(SWIG_ValueError, e.what());
    }
  /* SWIG_exception contains "goto fail" so make sure the label is defined */
  fail:
    return;
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

// show methods, when passed Python file-like objects, can raise Python
// exceptions, but std::ostream swallows these; so, add an extra check to
// these methods for Python exceptions (we don't want to do this for all
// methods, since it adds one extra function call, to PyErr_Occurred,
// regardless of whether the method failed)
%exception *::show {
  try {
    $action
    if (PyErr_Occurred()) {
      SWIG_fail;
    }
  } catch (...) {
    if (!PyErr_Occurred()) {
      handle_imp_exception();
    }
    SWIG_fail;
  }
}

// If Python exceptions are raised in a director method, temporarily reraise
// them as C++ exceptions (will be finally handled as Python exceptions
// again by %exception)
%feature("director:except") {
  if ($error != NULL) {
    throw Swig::DirectorMethodException();
  }
}
