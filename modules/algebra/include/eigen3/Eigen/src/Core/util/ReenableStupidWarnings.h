#ifdef IMP_EIGEN_WARNINGS_DISABLED
#undef IMP_EIGEN_WARNINGS_DISABLED

#ifndef IMP_EIGEN_PERMANENTLY_DISABLE_STUPID_WARNINGS
  #ifdef _MSC_VER
    #pragma warning( pop )
  #elif defined __INTEL_COMPILER
    #pragma warning pop
  #elif defined __clang__
    #pragma clang diagnostic pop
  #endif
#endif

#endif // IMP_EIGEN_WARNINGS_DISABLED
