cmake_minimum_required(VERSION 2.8)

project(IMP)

if(${PROJECT_SOURCE_DIR} MATCHES ${PROJECT_BINARY_DIR})
   message(ERROR "In-source building not supported: " ${PROJECT_SOURCE_DIR} " == "${PROJECT_BINARY_DIR} )
endif()

message(STATUS "Bootstrapping cmake")
execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_cmake.py
                RESULT_VARIABLE setup
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)
if( ${setup})
message(FATAL_ERROR " Failed to run cmake setup")
endif()

enable_testing()

if(POLICY CMP0017)
cmake_policy(SET CMP0017 NEW)
endif(POLICY CMP0017)


set(IMP_SWIG_PATH CACHE STRING "Colon separated list of places for swig to search")
set(SWIG_EXECUTABLE swig CACHE STRING "Swig program")
set(IMP_PYTHON_INCLUDE_PATH CACHE STRING "Where to look for python headers")
set(IMP_DISABLED_MODULES CACHE STRING "A colon-separated list of disabled modules")
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
set(IMP_MAX_CHECKS INTERNAL CACHE STRING "One of NONE, USAGE, INTERNAL")
set(IMP_MAX_LOG VERBOSE CACHE STRING "One of SILENT, PROGRESS, TERSE, VERBOSE")
else()
set(IMP_MAX_CHECKS USAGE CACHE STRING "One of NONE, USAGE, INTERNAL")
set(IMP_MAX_LOG VERBOSE CACHE STRING "One of SILENT, PROGRESS, TERSE, VERBOSE")
endif()
set(IMP_PER_CPP_COMPILATION "" CACHE STRING "A colon-separated list of modules to build one .cpp at a time.")

string(REGEX MATCHALL "[a-zA-Z]+" percpplist "${IMP_PER_CPP_COMPILATION}")
foreach(m ${percpplist})
message(STATUS ${m} " is percpp")
set(IMP_${m}_IS_PER_CPP 1)
endforeach(m)

if ("${IMP_PYTHON_INCLUDE_PATH}" STREQUAL "")
  message(STATUS "Finding python include path")
  execute_process(COMMAND python -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc();"
                             WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                             OUTPUT_VARIABLE IMP_PYTHON_INCLUDE_PATH
                             OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Got " ${IMP_PYTHON_INCLUDE_PATH})
endif()

include(GNUInstallDirs)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories("${PROJECT_BINARY_DIR}/include")

message(STATUS "Running setup script")
execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup.py
                --source=${PROJECT_SOURCE_DIR}
                --disabled=${IMP_DISABLED_MODULES}
                --datapath=${IMP_DATAPATH}
                RESULT_VARIABLE status
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
if (${status})
message(FATAL_ERROR "Error running IMP setup script")
endif()

execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_containers.py
                ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE status
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
if (${status})
message(FATAL_ERROR "Error running IMP setup_container script")
endif()

execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_all.py
                --source=${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE status
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
if (${status})
message(FATAL_ERROR "Error running IMP setup_container script")
endif()


execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_module_alias.py
                --source=${PROJECT_SOURCE_DIR} --module=base --alias=compatibility
                RESULT_VARIABLE status
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
if (${status})
message(FATAL_ERROR "Error running compatibility alias setup script")
endif()

execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_module_alias.py
                --source=${PROJECT_SOURCE_DIR} --module=kernel "--alias="
                RESULT_VARIABLE status
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
if (${status})
message(FATAL_ERROR "Error running kernel alias setup script")
endif()


# Don't link in libpython (that prevents the extension from being used in
# later compatible versions of Python), except on Windows where all DLL
# symbols must be defined
IF(WIN32)
  set(SWIG_PYTHON_LIBRARIES ${PYTHON_LIBRARIES} RMF)
ELSE(WIN32)
  set(SWIG_PYTHON_LIBRARIES )
ENDIF(WIN32)
# Apple linkers complain by default if there are undefined symbols
IF(APPLE)
  SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
    "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS} -flat_namespace -undefined suppress")
ENDIF(APPLE)

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS system filesystem thread program_options regex graph REQUIRED)

set(BOOST.FILESYSTEM_LIBRARIES ${Boost_FILESYSTEM_LIBRARY})
set(BOOST.SYSTEM_LIBRARIES ${Boost_SYSTEM_LIBRARY})
set(BOOST.THREAD_LIBRARIES ${Boost_THREAD_LIBRARY})
set(BOOST.PROGRAMOPTIONS_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY})
set(BOOST.REGEX_LIBRARIES ${Boost_REGEX_LIBRARY})
set(BOOST.GRAPH_LIBRARIES ${Boost_GRAPH_LIBRARY})

file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.FileSystem" "ok=True")
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.ProgramOptions" "ok=True")
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.Regex" "ok=True")
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.Graph" "ok=True")
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/Boost.System" "ok=True")

file(STRINGS "${PROJECT_BINARY_DIR}/data/build_info/sorted_modules" modules)


foreach(mod ${modules})
  add_subdirectory("${PROJECT_SOURCE_DIR}/modules/${mod}/")
endforeach(mod)

file(STRINGS "${PROJECT_BINARY_DIR}/data/build_info/applications" applications)
foreach(app ${applications})
  add_subdirectory("${PROJECT_SOURCE_DIR}/applications/${app}")
endforeach(app)

execute_process(COMMAND ${PROJECT_SOURCE_DIR}/tools/build/setup_imppy.py
                --python_path=$ENV{PYTHONPATH}
                --ld_path=$ENV{LD_PATH}
                --precommand=
                --path=$ENV{PATH}
                RESULT_VARIABLE status
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
if (${status})
message(FATAL_ERROR "Error running imppy setup script")
endif()
