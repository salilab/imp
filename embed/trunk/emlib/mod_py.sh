#!/bin/sh

MODINSTALLSVN=/diva1/home/modeller/SVN
if test -z "${EXECUTABLE_TYPESVN}"; then EXECUTABLE_TYPESVN=`/diva1/home/modeller/SVN/bin/guess-arch.sh set`; fi
if test -z "${KEY_MODELLERSVN}"; then KEY_MODELLERSVN=MODELIRANJE; fi

LLP=${MODINSTALLSVN}/lib/${EXECUTABLE_TYPESVN}
if test -z "${LD_LIBRARY_PATH}"; then
  LD_LIBRARY_PATH=${LLP}
else
  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${LLP}
fi
if test -z "${DYLD_LIBRARY_PATH}"; then
  DYLD_LIBRARY_PATH=${LLP}
else
  DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${LLP}
fi
if test -z "${LIBPATH}"; then
 LIBPATH=${LLP}
else
  LIBPATH=${LIBPATH}:${LLP}
fi

PP=${MODINSTALLSVN}/lib/${EXECUTABLE_TYPESVN}:${MODINSTALLSVN}/modlib
if test -z "${PYTHONPATH}"; then
  PYTHONPATH=${PP}
else
  PYTHONPATH=${PYTHONPATH}:${PP}
fi


 PYTHONPATH=${PYTHONPATH}:'/viola1/home/kerenl/bioinformatics/RigidBodies/RigidBodies/localRef'


INFERECE_SOURCE_PATH='/viola1/home/kerenl/bioinformatics/RigidBodies/RigidBodies'


PYTHONPATH=${PYTHONPATH}:'/viola1/home/kerenl/bioinformatics/projects/impEM/emlib'

 

export LD_LIBRARY_PATH DYLD_LIBRARY_PATH PYTHONPATH MODINSTALLSVN EXECUTABLE_TYPESVN KEY_MODELLERSVN
$*





