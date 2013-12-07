#!/usr/bin/env python

"""Extract doxygen-compatible documentation from a Python file.

   This is designed to be used as a doxygen filter. It takes a file name
   as input, and dumps out a filtered version of that Python file on stdout.

   Unlike doxypy, this filter is conservative. It only includes output for
   classes and functions that have docstrings, and makes no attempt to pass
   the original Python code bodies to doxygen. This avoids passing internal
   functions/classes, and SWIG-generated code, to doxygen.

   There is currently no supported for nested classes, although it would
   be straightforward to add.
"""

import sys
try:
    import ast
except ImportError:
    # Fail gracefully on older Pythons (ast needs Python 2.6)
    sys.exit(0)


def format_value(val):
    """Get a string representation of an ast node."""
    if isinstance(val, ast.Num):
        return str(val.n)
    elif isinstance(val, ast.Str):
        return repr(val.s)
    elif isinstance(val, ast.Name):
        return val.id
    elif isinstance(val, ast.Attribute):
        return format_value(val.value) + '.' + val.attr
    elif isinstance(val, ast.List):
        return '[' + ", ".join([format_value(x) for x in val.elts]) + ']'
    elif isinstance(val, ast.Tuple):
        return '(' + ", ".join([format_value(x) for x in val.elts]) + ')'
    elif isinstance(val, ast.Call):
        args = [format_value(x) for x in val.args] + \
               ["%s=%s" % (x.arg, format_value(x.value)) for x in val.keywords]
        if val.starargs:
            args.append('*' + val.starargs.id)
        if val.kwargs:
            args.append('**' + val.kwargs.id)
        return format_value(val.func) + '(' + ", ".join(args) + ')'
    raise ValueError("Do not know how to format %s" % str(val))


def get_class_signature(c):
    """Get a class signature, as a string"""
    bases = [format_value(b) for b in c.bases]
    if len(bases) == 0:
        return "class " + c.name + ":"
    else:
        return "class " + c.name + '(' + ", ".join(bases) + "):"


def get_function_signature(m):
    """Get a function signature, as a string"""
    sig = "def " + m.name + '('
    args = [[a, None] for a in m.args.args]
    off = len(args) - len(m.args.defaults)
    for i, default in enumerate(m.args.defaults):
        args[off + i][1] = default

    def format_arg(arg):
        sig = arg[0].id
        if arg[1]:
            sig += "=" + format_value(arg[1])
        return sig
    args = [format_arg(x) for x in args]
    sig += ", ".join(args)
    if m.args.vararg:
        sig += ", *" + m.args.vararg
    if m.args.kwarg:
        sig += ", **" + m.args.kwarg
    return sig + '):'


def handle_func(f):
    # Exclude internal functions
    if f.name.startswith('_') and not (f.name.startswith('__') and
                                       f.name.endswith('__')):
        return
    doc = ast.get_docstring(f)
    # Don't include undocumented functions
    if not doc:
        return
    # Exclude SWIG-generated docstrings
    if doc:
        for line in doc.split('\n'):
            if line.startswith(f.name + '('):
                return
    return f


def get_dump_docstring(node, add_lines=[]):
    lines = []
    doc = ast.get_docstring(node)
    if doc:
        prefix = "## "
        for line in doc.split('\n') + add_lines:
            lines.append(prefix + line)
            prefix = "#  "
    return lines


def dump_function(func, indent, printer):
    d = get_dump_docstring(func)
    printer.output_lines(indent, d + [get_function_signature(func)],
                         func.lineno)
    printer.output_line(indent + 4, "pass")


def dump_class(cls, meths, indent, printer):
    if cls.swig:
        add_lines = []
    else:
        add_lines = ['', '\\pythononlyclass']
    d = get_dump_docstring(cls, add_lines)
    printer.output_lines(indent, d + [get_class_signature(cls)], cls.lineno)
    if len(meths) == 0:
        printer.output_line(indent + 4, "pass")
    for m in meths:
        dump_function(m, indent + 4, printer)


def handle_class(c, indent, printer):
    if c.name.startswith('_'):
        return
    doc = ast.get_docstring(c)
    # Don't include undocumented classes
    if not doc:
        return
    c.swig = (doc and doc.startswith('Proxy of C++ '))
    meths = []
    for m in ast.iter_child_nodes(c):
        if isinstance(m, ast.FunctionDef):
            f = handle_func(m)
            if f:
                meths.append(f)
    if c.swig and len(meths) == 0:
        return
    dump_class(c, meths, indent, printer)


class OutputPrinter(object):

    def __init__(self):
        # The number of the next line to be written (1-based)
        self.lineno = 1

    def output_line(self, indent, line):
        """Output a single line of text at the given indentation level."""
        print " " * indent + line
        self.lineno += 1

    def output_lines(self, indent, lines, lineno):
        """Output several lines of text. The output is padded to get the last
           line as close to the given lineno as possible, so that errors or
           warnings from doxygen (which uses the filtered text) are reported
           as being close to the correct line numbers, in the original
           unfiltered file. (This might not always be possible, since we move
           each Python docstring from after the class/function definition
           to before it, but since we discard the body of each class/function,
           we stand a good chance.)"""
        pad = lineno - len(lines) - self.lineno + 1
        if pad > 0:
            sys.stdout.write('\n' * pad)
            self.lineno += pad
        for line in lines:
            self.output_line(indent, line)


def parse_file(fname):
    # Exclude non-IMP code
    if '_compat_python' in fname or 'compat_subprocess' in fname:
        return
    # Don't try to document applications (which get symlinked into bin dir)
    if '/bin/' in fname:
        return
    # Pass examples through unchanged
    if '/examples/' in fname:
        for line in open(fname):
            sys.stdout.write(line)
        return
    lines = open(fname).readlines()
    lines = [x.rstrip('\r\n') for x in lines]
    a = ast.parse(("\n".join(lines)).rstrip() + '\n', fname)

    indent = 0
    printer = OutputPrinter()

    if isinstance(a, ast.Module):
        doc = get_dump_docstring(a)
        if len(doc) > 0:
            printer.output_lines(indent, doc, 1)

    for n in ast.iter_child_nodes(a):
        if isinstance(n, ast.ClassDef):
            handle_class(n, indent, printer)
        elif isinstance(n, ast.FunctionDef):
            f = handle_func(n)
            if f:
                dump_function(f, indent, printer)

if __name__ == '__main__':
    parse_file(sys.argv[1])
