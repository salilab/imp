
set(IMP_HAS_DEPRECATED 1 CACHE BOOL "Whether to use deprecated functionality")

if("${CMAKE_BUILD_TYPE}" MATCHES "")
set(build "RELEASE")
else()
string(TOUPPER "${CMAKE_BUILD_TYPE}" build)
endif()

set(IMP_BASE_CONFIG IMP_BUILD=IMP_${build}:IMP_HAS_DEPRECATED=${IMP_HAS_DEPRECATED}:IMP_HAS_LOG=IMP_${IMP_MAX_LOG}:IMP_HAS_CHECKS=IMP_${IMP_MAX_CHECKS}:IMP_DEBUG=0:IMP_RELEASE=1:IMP_SILENT=0:IMP_PROGRESS=2:IMP_TERSE=3:IMP_VERBOSE=4:IMP_NONE=0:IMP_USAGE=1:IMP_INTERNAL=2 CACHE INTERNAL "" FORCE)


imp_execute_process("generate paths.cpp"
               ${PROJECT_BINARY_DIR}
               COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_paths.py
                "--datapath=${CMAKE_INSTALL_FULL_DATADIR}/IMP"
                "--examplepath=${CMAKE_INSTALL_FULL_DOCDIR}/examples"
                "--output=src/base/paths.cpp")
