// Set up directors for an IMP kernel class
%define IMP_DIRECTOR_KERNEL_CLASS(TYPE)
%feature("director") IMP::TYPE;
%pythonappend IMP::TYPE::TYPE %{
        if self.__class__ != TYPE:
            _director_objects.register(self)
%}
%enddef

// Set up directors for an IMP module class
%define IMP_DIRECTOR_MODULE_CLASS(MODULE, TYPE)
%feature("director") IMP::MODULE::TYPE;
%pythonappend IMP::MODULE::TYPE::TYPE %{
        if self.__class__ != TYPE:
            IMP._director_objects.register(self)
%}
%enddef

/* Notes on directors and reference counting:

All reference-counted IMP C++ classes should also be managed correctly by the
SWIG interface. When an IMP class is created in Python, three objects are
created:
1. The IMP C++ object itself.
2. A low-level SWIG Python object created using the Python C API, which
   contains little more than a pointer to the IMP C++ object. This object
   calls IMP::internal::ref() and IMP::internal::unref() on the IMP C++ object
   at construction and when it is garbage collected.
3. A pure Python (proxy) object, which holds a Python reference to the low-level
   C API object.
Since all of these pointers are reference counted, the C++ object will be kept
alive as long as the Python proxy exists, and will be cleaned up when both the
proxy disappears *and* no C++ references to it exist. There is nothing,
however, to prevent a proxy object and the low-level SWIG Python object from
disappearing. (This is fine because such objects only wrap functionality in
the C++ objects anyway, and in fact we usually use Python to create C++
objects, then pass them to C++ containers, after which we *want* the Python
proxies to disappear, so as not to litter memory with the things. A new proxy
can be recreated when needed to access the C++ object.)

When directors are used, the situation is similar, except that the C++ object
is not the original IMP class, but a SWIG subclass that takes care of
delegating C++ method calls to the corresponding Python object. This includes
a weak pointer to the Python object. The pure Python class is now a
user-defined subclass of the proxy which holds implementations. So now it is
a problem if the proxy is destroyed, because then the C++ object cannot route
method calls to it. Thus, the proxy must be kept alive as long as the C++
object is. This could be achieved by maintaing a strong pointer from the C++
object to the Python object, but that would create a reference loop, and the
object would never be freed. Ideally the Python object's reference count would
be increased or decreased when the C++ object's count was, but this would
require some major surgery to the C++ base classes (virtual method calls for
every ref/unref, for example, which could hurt performance).

The simple solution implemented here is to add a Python reference for every
director object created, to keep it alive. These references are then cleaned
up periodically by inspecting the real C++ and Python reference counts.
For small numbers of director objects (our expected usage) this works well.
*/

%pythoncode {
import sys
class _DirectorObjects(object):
    """Simple class to keep references to director objects to prevent premature
       deletion. For internal use only."""
    def __init__(self):
        self._objects = []
    def register(self, obj):
        """Take a reference to a director object; will only work for
           refcounted C++ classes"""
        if hasattr(obj, 'get_ref_count'):
            self._objects.append(obj)
    def cleanup(self):
        """Only drop our reference and allow cleanup by Python if no other
           Python references exist (we hold 3 references: one in self._objects,
           one in x, and one in the argument list for getrefcount) *and* no
           other C++ references exist (the Python object always holds one)"""
        objs = [x for x in self._objects if sys.getrefcount(x) > 3 \
                                         or x.get_ref_count() > 1]
        # Do in two steps so the references are kept until the end of the
        # function (deleting references may trigger a fresh call to this method)
        self._objects = objs
    def get_object_count(self):
        """Get number of director objects (useful for testing only)"""
        return len(self._objects)
_director_objects = _DirectorObjects()
}

// Trigger cleanup of unused director objects when other IMP cleanup is done
%pythonappend IMP::Model::~Model %{
        _director_objects.cleanup()
%}
%pythonappend IMP::Optimizer::~Optimizer %{
        _director_objects.cleanup()
%}
