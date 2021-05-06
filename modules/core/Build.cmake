# Build ihm C extension as part of building IMP.core
# Static builds don't include Python extensions
if(NOT IMP_STATIC AND NOT IMP_USE_SYSTEM_IHM)
  set(IMP_core_LIBRARY_EXTRA_DEPENDENCIES ihm-python CACHE INTERNAL "" FORCE)
else()
  unset(IMP_core_LIBRARY_EXTRA_DEPENDENCIES CACHE)
endif()
