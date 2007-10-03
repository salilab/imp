#!/bin/sh



EMP=/viola1/home/kerenl/bioinformatics/projects/impEM
if test -z "${LD_LIBRARY_PATH}"; then
  LD_LIBRARY_PATH=${EMP}/emlib
else
  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${EMP}/emlib
fi
if test -z "${DYLD_LIBRARY_PATH}"; then
  DYLD_LIBRARY_PATH=${EMP}/emlib
else
  DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${EMP}/emlib
fi

if test -z "${LIBPATH}"; then
  LIBPATH=${EMP}/emlib
else
  LIBPATH=${LIBPATH}:${EMP}/emlib
fi

PP=${EMP}/emlib/pyext
if test -z "${PYTHONPATH}"; then
  PYTHONPATH=${PP}
else
  if test "x${EXETYPE}" = "xi386-w32"; then
    PYTHONPATH="${PYTHONPATH};${PP}"
  else
    PYTHONPATH=${PYTHONPATH}:${PP}
  fi
fi


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

export LD_LIBRARY_PATH DYLD_LIBRARY_PATH PYTHONPATH MODINSTALLSVN EXECUTABLE_TYPESVN KEY_MODELLERSVN
$*





