import os

modeller=os.environ['MODINSTALLSVN']
prefix=os.environ['HOME']+"/fs"
path='/opt/local/bin'
libpath="/opt/local/lib:"+os.environ['HOME']+'/fs/lib'
ldlibpath=os.environ['HOME']+'/fs/lib'
# paths containing infrequently changing headers should be including using the compiler's option
# otherwise scons will scan them every time
cxxflags="-Wall -DCGAL_DISABLE_ROUNDING_MATH_CHECK -I"+os.environ['HOME']+'/fs/include -I/opt/local/include'
pythonpath=os.environ['PYTHONPATH']
# build all modules found in the modules directory
localmodules=True
