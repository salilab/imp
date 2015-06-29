%pythoncode %{

import functools
import contextlib

def deprecated_module(version, module, help_message):
    """Mark a Python module as deprecated.
       @note The `module` argument would normally be `__name__`.
       @see [deprecation support](@ref deprecation)."""
    handle_use_deprecated(
                "Module %s is deprecated. %s" % (module, help_message))

def deprecated_object(version, help_message):
    """Python decorator to mark a class as deprecated.
       @see [deprecation support](@ref deprecation)."""
    def wrapper(obj):
        orig_init = obj.__init__
        # Don't try to copy __module__ since __init__ may not have it
        # (e.g. if we subclassed object but didn't override __init__)
        @functools.wraps(orig_init, ('__name__', '__doc__'))
        def __init__(obj, *args, **keys):
            handle_use_deprecated("Object %s is deprecated. %s"
                                  % (type(obj), help_message))
            orig_init(obj, *args, **keys)
        obj.__init__ = __init__
        return obj
    return wrapper

def deprecated_method(version, help_message):
    """Python decorator to mark a method as deprecated.
       @see [deprecation support](@ref deprecation)."""
    def out_wrapper(obj):
        @functools.wraps(obj)
        def wrapper(cls, *args, **keys):
            handle_use_deprecated("Method %s in %s is deprecated. %s"
                               % (obj.__name__, type(cls), help_message))
            return obj(cls, *args, **keys)
        return wrapper
    return out_wrapper

def deprecated_function(version, help_message):
    """Python decorator to mark a function as deprecated.
       @see [deprecation support](@ref deprecation)."""
    def out_wrapper(obj):
        @functools.wraps(obj)
        def wrapper(*args, **keys):
            handle_use_deprecated("Function %s is deprecated. %s"
                                  % (obj.__name__, help_message))
            return obj(*args, **keys)
        return wrapper
    return out_wrapper

@contextlib.contextmanager
def allow_deprecated(allow=True):
    """Context manager to temporarily allow (or disallow) deprecated code.
       @see [deprecation support](@ref deprecation)."""
    old = get_deprecation_exceptions()
    set_deprecation_exceptions(not allow)
    yield
    set_deprecation_exceptions(old)
%}
