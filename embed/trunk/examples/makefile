CC = g++
MOLLIB = ../mollib/
EMLIB = ../emlib/
CCFLAGS  = -I$(MOLLIB) -I$(EMLIB)  -ftemplate-depth-50 -Wall -O2 -fPIC 
LDFLAGS =  -L$(MOLLIB) -lmol  -L$(EMLIB) -lem  -Wl 

CLASSES =  read_write_map

# Prepare object and source file list using pattern substitution func.
ALL  = $(CLASSES)
OBJS = $(patsubst %, %.o,  $(ALL))
SRCS = $(patsubst %, %.cc, $(ALL)) 

TARGET = read_write_map.Linux

$(TARGET): $(OBJS)
	$(CC) -Xlinker -R/usr/local/lib/gcc32/lib $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.cc
	$(CC) $(CCFLAGS) -c $*.cc

clean:
	/bin/rm -f $(OBJS) $(TARGET)


