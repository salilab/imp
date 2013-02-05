## \example example_nacl_config.py
## Build IMP with NaCl.

root='/Users/drussel/fs-nacl/'
toolpath='/Users/drussel/src/naclports-mine/bin/'

boostversion='104300'
boostlibsuffix='-mt'
boost_filesystem='no'
boost_programoptions='no'
fftw3='no'
endian='little'
netcdfcpp='yes'
netcdfcpplibs='netcdf_c++:netcdf'
netcdflibpath='/Users/drussel/fs-nacl/lib'
netcdfincludepath='/Users/drussel/fs-nacl/include'
cgal='no'
ann='no'
annlibs='ANN'
gsl='yes'
gsllibs='gsl:gslcblas:m'
opencv21='no'
opencv22='no'
bullet='yes'
bulletlibs='BulletSoftBody:BulletDynamics:BulletCollision:LinearMath'
openmm='no'
build='debug'
repository='../svn'
prefix='/Users/drussel/fs-nacl/imp'
platformflags=False
cxxcompiler=toolpath+'g++'
ar=toolpath+'ar'
ranlib=toolpath+'ranlib'
cxxflags=' -fno-common -Wall -Wno-deprecated -Woverloaded-virtual -g'
includepath=root+'include
arliblinkflags=''
binlinkflags='-g -L'+root+'lib -lnosys'
python='False'
rpath='False'
static='True'
dynamic='False'
pkgconfig='False'
path=""
disabledmodules="saxs:em:em2d"
