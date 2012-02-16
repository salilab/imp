WORKDIR = /tmp/work
PREFIX = /tmp/RMF
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include
PYTHONDIR = $(PREFIX)/lib
PYTHONINCLUDE = /usr/include/python2.6
BUILD_LIB = g++ -shared -I$(INCLUDEDIR) -fPIC -O3 -Wno-deprecated
SWIG = /usr/bin/swig

HEADERS = Category.h exceptions.h FileConstHandle.h FileHandle.h HDF5ConstAttributes.h HDF5ConstDataSetD.h HDF5ConstFile.h HDF5ConstGroup.h HDF5DataSetD.h HDF5DataSetIndexD.h HDF5File.h HDF5Group.h hdf5_handle.h HDF5MutableAttributes.h HDF5Object.h decorators.h types.h infrastructure_macros.h Key.h names.h NodeConstHandle.h NodeHandle.h NodeID.h NodeSetConstHandle.h NodeSetHandle.h Validator.h internal/errors.h internal/hash.h internal/imp_operations.h internal/intrusive_ptr_object.h internal/map.h internal/set.h internal/shared.h internal/swig_helpers.h


SRCS = src/exceptions.cpp src/hdf5_wrapper.cpp  src/NodeConstHandle.cpp  src/Validator.cpp src/FileConstHandle.cpp src/NodeHandle.cpp src/FileHandle.cpp src/Key.cpp src/NodeTupleHandle.cpp src/names.cpp src/internal/errors.cpp src/internal/shared.cpp $(WORKDIR)/RMF_config.cpp


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