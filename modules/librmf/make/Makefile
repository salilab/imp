WORKDIR = /tmp/work
PREFIX = /tmp/RMF
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include
PYTHONDIR = $(PREFIX)/lib
PYTHONINCLUDE = /usr/include/python2.6
BUILD_LIB = g++ -shared -I$(INCLUDEDIR) -fPIC -O3 -Wno-deprecated
SWIG = /usr/bin/swig

HEADERS = $(subst include, , $(wildcard include/*.h) $(wildcard include/internal/*.h))


SRCS = $(wildcard src/*.cpp) $(wildcard src/internal/*.cpp) $(WORKDIR)/RMF_config.cpp


all : lib pythonlib

lib : $(SRCS) headers
	mkdir -p $(LIBDIR)
	$(BUILD_LIB) $(SRCS) -o $(LIBDIR)/libRMF.so -lhdf5 -lboost_filesystem-mt -lboost_thread-mt -DNDEBUG

pythonlib : swig lib
	mkdir -p $(WORKDIR)
	${BUILD_LIB} -I$(WORKDIR) $(WORKDIR)/RMF_wrap.cpp -o $(PYTHONDIR)/_RMF.so -lRMF -L$(LIBDIR) -I$(PYTHONINCLUDE)
	mkdir -p $(PYTHONDIR)/RMF
	cp $(WORKDIR)/RMF.py $(PYTHONDIR)/RMF/__init__.py

swig : headers make/RMF.i
	mkdir -p $(WORKDIR)
	$(SWIG) -castmode -interface _RMF -DPySwigIterator=RMF_PySwigIterator -DSwigPyIterator=RMF_SwigPyIterator -python -c++ -naturalvar -fvirtual -Wextra -o $(WORKDIR)/RMF_wrap.cpp -oh $(WORKDIR)/RMF_wrap.h -Ipyext -Iinclude -DIMP_SWIG -I$(INCLUDEDIR) make/RMF.i

headers :
	mkdir -p $(INCLUDEDIR)/RMF/internal
	$(foreach h, $(HEADERS), cp include/$h $(INCLUDEDIR)/RMF/$h;)
	cp make/RMF_config.h-in $(INCLUDEDIR)/RMF/RMF_config.h
	cp make/RMF.h-in $(INCLUDEDIR)/RMF.h

$(WORKDIR)/RMF_config.cpp: make/RMF_config.cpp-in
	mkdir -p $(WORKDIR)
	cp make/RMF_config.cpp-in $(WORKDIR)/RMF_config.cpp