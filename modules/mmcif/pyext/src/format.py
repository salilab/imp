"""@namespace IMP.mmcif.format
   @brief Utility classes to handle CIF format.

   This module provides classes to write out mmCIF files. While it is
   designed for IMP, it could be used in other Python code.
"""

from __future__ import print_function
import sys

# Python 3 has no 'long' type, so use 'int' instead
if sys.version_info[0] >= 3:
    _long_type = int
else:
    _long_type = long

class _LineWriter(object):
    def __init__(self, writer, line_len=80):
        self.writer = writer
        self.line_len = line_len
        self.column = 0
    def write(self, val):
        if isinstance(val, str) and '\n' in val:
            self.writer.fh.write("\n;")
            self.writer.fh.write(val)
            if not val.endswith('\n'):
                self.writer.fh.write("\n")
            self.writer.fh.write(";\n")
            self.column = 0
            return
        val = self.writer._repr(val)
        if self.column > 0:
            if self.column + len(val) + 1 > self.line_len:
                self.writer.fh.write("\n")
                self.column = 0
            else:
                self.writer.fh.write(" ")
                self.column += 1
        self.writer.fh.write(val)
        self.column += len(val)


class _CifCategoryWriter(object):
    def __init__(self, writer, category):
        self.writer = writer
        self.category = category
    def write(self, **kwargs):
        self.writer._write(self.category, kwargs)
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        pass


class _CifLoopWriter(object):
    def __init__(self, writer, category, keys):
        self.writer = writer
        self.category = category
        self.keys = keys
        # Remove characters that we can't use in Python identifiers
        self.python_keys = [k.replace('[', '').replace(']', '') for k in keys]
        self._empty_loop = True
    def write(self, **kwargs):
        if self._empty_loop:
            f = self.writer.fh
            f.write("#\nloop_\n")
            for k in self.keys:
                f.write("%s.%s\n" % (self.category, k))
            self._empty_loop = False
        l = _LineWriter(self.writer)
        for k in self.python_keys:
            l.write(kwargs.get(k, self.writer.omitted))
        self.writer.fh.write("\n")
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        if not self._empty_loop:
            self.writer.fh.write("#\n")


class _CifWriter(object):
    omitted = '.'
    unknown = '?'
    _boolmap = {False: 'NO', True: 'YES'}

    def __init__(self, fh):
        self.fh = fh
    def category(self, category):
        return _CifCategoryWriter(self, category)
    def loop(self, category, keys):
        return _CifLoopWriter(self, category, keys)
    def write_comment(self, comment):
        for line in textwrap.wrap(comment, 78):
            self.fh.write('# ' + line + '\n')
    def _write(self, category, kwargs):
        for key in kwargs:
            self.fh.write("%s.%s %s\n" % (category, key,
                                          self._repr(kwargs[key])))
    def _repr(self, obj):
        if isinstance(obj, str) and '"' not in obj \
           and "'" not in obj and " " not in obj:
            return obj
        elif isinstance(obj, float):
            return "%.3f" % obj
        elif isinstance(obj, bool):
            return self._boolmap[obj]
        # Don't use repr(x) if type(x) == long since that adds an 'L' suffix,
        # which isn't valid mmCIF syntax. _long_type = long only on Python 2.
        elif isinstance(obj, _long_type):
            return "%d" % obj
        else:
            return repr(obj)
