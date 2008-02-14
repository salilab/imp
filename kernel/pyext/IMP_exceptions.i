/* IMP exception handling */


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
    handle_imp_exception();
    /* This should be unnecessary, since handle_imp_exception cannot return;
       here just to quell lots of warnings about the 'result' variable not
       being initialized. */
    SWIG_fail;
  }
}
