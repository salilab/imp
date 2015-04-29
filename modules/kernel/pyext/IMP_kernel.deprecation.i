%pythoncode %{

def deprecated_module(version, module, help_message):
    handle_use_deprecated(
                "Module %s is deprecated. %s" % (module, help_message))

class __deprecation_base(object):
    def __init__(self, version, help_message):
        self.version, self.help_message = version, help_message

class deprecated_object(__deprecation_base):
    def __call__(self, obj):
        orig_init = obj.__init__
        def __init__(obj, *args, **keys):
            handle_use_deprecated("Object %s is deprecated. %s"
                                  % (type(obj), self.help_message))
            orig_init(obj, *args, **keys)
        obj.__init__ = __init__
        return obj


class deprecated_method(__deprecation_base):
    def __call__(self, obj):
        def wrapper(cls, *args, **keys):
            handle_use_deprecated("Method %s in %s is deprecated. %s"
                               % (obj.__name__, type(cls), self.help_message))
            return obj(cls, *args, **keys)
        return wrapper


class deprecated_function(__deprecation_base):
    def __call__(self, obj):
        def wrapper(*args, **keys):
            handle_use_deprecated("Function %s is deprecated. %s"
                                  % (obj.__name__, self.help_message))
            return obj(*args, **keys)
        return wrapper
%}
