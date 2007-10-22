/* IMP exception handling */


%{
  /* Code to convert C++ exceptions into scripting language errors. Ugly, but
     it saves having lots of catch statements in every single wrapper. */
  static void handle_imp_exception(std::exception &exc)
  {
    std::exception *ept = &exc;
    if (dynamic_cast<std::out_of_range *>(ept)) {
      SWIG_exception(SWIG_IndexError, ept->what());
    }
  /* SWIG_exception contains "goto fail" so make sure the label is defined */
  fail:
    return;
  }
%}

%exception {
  try {
    $action
  } catch (std::exception &exc) {
    handle_imp_exception(exc);
    /* This should be unnecessary, since handle_imp_exception cannot return;
       here just to quell lots of warnings about the 'result' variable not
       being initialized. */
    SWIG_fail;
  }
}
