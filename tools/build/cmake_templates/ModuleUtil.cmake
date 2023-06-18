include_directories(SYSTEM %(includepath)s)
link_directories(%(libpath)s)
add_definitions("-DIMP_EXECUTABLE")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${%(NAME)s_CXX_FLAGS}")

include(Files.cmake)

foreach (bin ${cppfiles})
   GET_FILENAME_COMPONENT(name ${bin} NAME_WE)
   add_executable(IMP.%(name)s-${name} ${bin})
   target_link_libraries(IMP.%(name)s-${name}     IMP.%(name)s-lib
    %(modules)s
    %(dependencies)s)
   set_target_properties(IMP.%(name)s-${name} PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/module_bin/%(name)s"
                         OUTPUT_NAME ${name})
  set_property(TARGET "IMP.%(name)s-${name}" PROPERTY FOLDER "IMP.%(name)s")
   set(executables ${executables} IMP.%(name)s-${name})
endforeach(bin)

set(IMP_%(name)s_UTILS ${executables} CACHE INTERNAL "" FORCE)
