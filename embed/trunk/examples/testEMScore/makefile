CC = g++
EMLIB = ../../emlib/
CCFLAGS  =  -I$(EMLIB)  -ftemplate-depth-50 -Wall -O2 -fPIC 
LDFLAGS =   -L$(EMLIB) -lem  -Wl 

CLASSES =  test_em_score

# Prepare object and source file list using pattern substitution func.
ALL  = $(CLASSES)
OBJS = $(patsubst %, %.o,  $(ALL))
SRCS = $(patsubst %, %.cc, $(ALL)) 

TARGET = cc_test.Linux

$(TARGET): $(OBJS)
	$(CC) -Xlinker -R/usr/local/lib/gcc32/lib $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.cc
	$(CC) $(CCFLAGS) -c $*.cc

clean:
	/bin/rm -f $(OBJS) $(TARGET)


