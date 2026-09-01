#!/usr/bin/python3

"""Rebuild the python-ihm _format.pyd extension (which is built for
   Python 3.9) for both Python 3.10 and the Python limited API."""

import subprocess
from pathlib import Path

TOPDIR = Path(__file__).parent.parent.parent.resolve()

# Touch .c files and rerun make to get the commands it ran to build
# the 3.9 extension
(TOPDIR / 'modules/core/dependency/python-ihm/src/ihm_format.c').touch()
Path('src/core_swig/ihm_format_wrap.c').touch()
build_log = subprocess.check_output(['make', 'VERBOSE=1', 'ihm-python'],
                                    universal_newlines=True)
build_commands = [cmd for cmd in build_log.split('\n') if
                  'ihm_python_EXPORTS' in cmd or 'LIBPATH' in cmd]

if len(build_commands) != 3:
    raise ValueError("Expected exactly 3 build commands")
if '3.9' not in build_commands[0] or '3.9' not in build_commands[1]:
    raise ValueError("Compile commands do not include Python 3.9 headers")
if 'python39.lib' not in build_commands[2]:
    raise ValueError("Link command does not include Python 3.9 library")

print("** Rerun python-ihm build to make Python 3.10 extension")
py310_cmds = [cmd.replace('3.9', '3.10').replace('39', '310').replace(
                  '_format.pyd', '_format310.pyd')
              for cmd in build_commands]
py310_cmds = "\n".join(py310_cmds)
print(py310_cmds)
subprocess.check_call(py310_cmds, shell=True)

print("** Rerun python-ihm build to make Python 3.11+ extension")
py311_cmds = [cmd.replace('3.9', '3.11').replace('39', '3').replace(
                  '_format.pyd', '_format_lim.pyd').replace(
                  '-Dihm_python_EXPORTS',
                  '-Dihm_python_EXPORTS -DPy_LIMITED_API=0x030B0000')
              for cmd in build_commands]
py311_cmds = "\n".join(py311_cmds)
print(py311_cmds)
subprocess.check_call(py311_cmds, shell=True)
