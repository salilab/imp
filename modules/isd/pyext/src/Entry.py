#!/usr/bin/env python

"""@namespace IMP.isd.Entry
   Classes to handle ISD statistics files.
"""

class Entry:
    """The entry class represents a column in the statistics file.
    Its components are a title, a format and an additional object.
    It's what gets written to the statistics file in a column.
    - title: the title of the column
    - format: a wisely chosen format string (see below)
    - something: either something that can be formattable, a string, a number
      etc. This is a static entry. In that case all remaining arguments are
      discarded and get_value returns the formatted string : format % something.
      If something is a function, this is a dynamic entry, and the format
      string is used on the result of the function call
      something(*args,**kwargs).
    """
    def __init__(self, title, fmt, something, *args, **kwargs):
        self.title = title
        self.format = fmt
        self.is_function = callable(something)
        if self.is_function:
            self.function = something
            self.args = args
            self.kwargs = kwargs
        else:
            self.value = something
        self.was_updated_since_last_get = False

    def __repr__(self):
        if self.is_function:
            return "Entry('%s', '%s', f(...))" % (self.title, self.format)
        else:
            return "Entry('%s', '%s', %s)" % (self.title, self.format,
                    self.value)


    def get_title(self):
        return self.title

    def get_raw_value(self):
        if self.is_function:
            return self.function(*self.args, **self.kwargs)
        else:
            self.was_updated_since_last_get = False
            return self.value

    def get_value(self):
        try:
            return self.format % self.get_raw_value()
        except TypeError:
            return "N/A"

    def set_value(self, val):
        if self.is_function:
            raise RuntimeError, \
                    "Can only set_value on static entries."
        self.value = val
        self.was_updated_since_last_get = True

    def get_was_updated(self):
        return self.is_function or self.was_updated_since_last_get
