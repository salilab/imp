CC = g++
PLIB = /usr/include/python2.5/
#CCFLAGS  =   -I$(PLIB) -I$(UTILIB) -I$(LIB)  -Wall -O2 -fPIC -shared 
#LDFLAGS = -I$(PLIB) -lpython2.4  -L$(UTILIB) -lutil  -L$(LIB) -lgamb++

CCFLAGS  =   -I$(PLIB) -I$(MOLLIB)   -Wall -O2 -fPIC -shared 
LDFLAGS = -I$(PLIB) -lpython2.5 -L$(MOLLIB) -lmol



CLASSES =  Vector3 Map3D DensityHeader MapReaderWriter XplorReaderWriter EMReaderWriter  DensityMap  SampledDensityMap CoarseCC

# Prepare object and source file list using pattern substitution func.
ALL  = $(CLASSES)
OBJS = $(patsubst %, %.o,  $(ALL))
SRCS = $(patsubst %, %.cc, $(ALL))

TARGET = _emlib.so

$(TARGET): $(OBJS) 

	/usr/bin/ar src $(TARGET) $(OBJS) 
%.o: %.cc 
	$(CC) $(CCFLAGS) -c $*.cc

clean: 
	/bin/rm -f $(OBJS) $(TARGET)
