# Build ihm C extension as part of building IMP.core
# Static builds don't include Python extensions
if(NOT IMP_STATIC)
  set(IMP_core_LIBRARY_EXTRA_DEPENDENCIES ihm-python CACHE INTERNAL "" FORCE)
endif()
