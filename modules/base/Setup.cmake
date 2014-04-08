
if("${CMAKE_BUILD_TYPE}" MATCHES "")
set(build "RELEASE")
else()
string(TOUPPER "${CMAKE_BUILD_TYPE}" build)
endif()

set(IMP_base_CONFIG IMP_BUILD=IMP_${build}:IMP_HAS_LOG=IMP_${IMP_MAX_LOG}:IMP_HAS_CHECKS=IMP_${IMP_MAX_CHECKS}:IMP_DEBUG=0:IMP_RELEASE=1:IMP_SILENT=0:IMP_PROGRESS=2:IMP_TERSE=3:IMP_VERBOSE=4:IMP_MEMORY=5:IMP_NONE=0:IMP_USAGE=1:IMP_INTERNAL=2:IMP_BASE_HAS_LOG4CXX=0)


imp_execute_process("generate paths.cpp"
               ${CMAKE_BINARY_DIR}
               COMMAND ${CMAKE_SOURCE_DIR}/tools/build/setup_paths.py
                "--datapath=${CMAKE_INSTALL_FULL_DATADIR}/IMP"
                "--examplepath=${CMAKE_INSTALL_FULL_DOCDIR}/examples"
                "--output=src/base/paths.cpp")
