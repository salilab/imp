#!/usr/bin/env python3

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
import ast


# ast.Constant replaces ast.Num and ast.Str in newer Python (3.8+)
has_constant = hasattr(ast, 'Constant')


def format_value(val):
    """Get a string representation of an ast node."""
    if isinstance(val, ast.Name):
        return val.id
    elif isinstance(val, ast.Add):
        return '+'
    elif isinstance(val, ast.Sub):
        return '-'
    elif isinstance(val, ast.Mult):
        return '*'
    elif isinstance(val, ast.Div):
        return '/'
    elif isinstance(val, ast.Pow):
        return '**'
    elif isinstance(val, ast.Mod):
        return '%'
    elif isinstance(val, ast.LShift):
        return '<<'
    elif isinstance(val, ast.RShift):
        return '>>'
    elif isinstance(val, ast.BitOr):
        return '|'
    elif isinstance(val, ast.BitAnd):
        return '&'
    elif isinstance(val, ast.BitXor):
        return '^'
    elif isinstance(val, ast.FloorDiv):
        return '//'
    elif isinstance(val, ast.Invert):
        return '~'
    elif isinstance(val, ast.Not):
        return 'not '
    elif isinstance(val, ast.USub):
        return '-'
    elif isinstance(val, ast.Attribute):
        return format_value(val.value) + '.' + val.attr
    elif isinstance(val, ast.List):
        return '[' + ", ".join([format_value(x) for x in val.elts]) + ']'
    elif isinstance(val, ast.Tuple):
        return '(' + ", ".join([format_value(x) for x in val.elts]) + ')'
    elif isinstance(val, ast.Dict):
        def format_item(key, val):
            return "%s: %s" % (format_value(key), format_value(val))
        return '{' + ", ".join(
            [format_item(key, val)
             for key, val in zip(val.keys, val.values)]) + '}'
    elif isinstance(val, ast.BinOp):
        return format_value(val.left) + " " + format_value(val.op) \
            + " " + format_value(val.right)
    elif isinstance(val, ast.UnaryOp):
        return format_value(val.op) + format_value(val.operand)
    elif isinstance(val, ast.Starred):
        return "*" + format_value(val.value)
    elif has_constant and isinstance(val, ast.Constant):
        return repr(val.value)
    elif not has_constant and isinstance(val, ast.Num):
        return str(val.n)
    elif not has_constant and isinstance(val, ast.Str):
        return repr(val.s)
    elif isinstance(val, ast.Call):
        args = [format_value(x)
                for x in val.args if not isinstance(x, ast.Starred)] + \
               ["%s=%s" % (x.arg, format_value(x.value))
                for x in val.keywords if x.arg is not None] + \
               [format_value(x)
                for x in val.args if isinstance(x, ast.Starred)] + \
               ["**" + format_value(x.value)
                for x in val.keywords if x.arg is None]
        return format_value(val.func) + '(' + ", ".join(args) + ')'
    raise ValueError("Do not know how to format %s while running %s"
                     % (str(val), str(sys.argv)))


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
        if hasattr(arg[0], 'id'):
            sig = arg[0].id
        else:
            sig = arg[0].arg
        if arg[1]:
            sig += "=" + format_value(arg[1])
        return sig
    args = [format_arg(x) for x in args]
    sig += ", ".join(args)
    if m.args.vararg:
        sig += ", *" + m.args.vararg.arg
    if m.args.kwarg:
        sig += ", **" + m.args.kwarg.arg
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


def get_deprecation_docstring(node):
    """If the node (class, function or method) is decorated with one of IMP's
       deprecation decorators, add the runtime warning to the docstring too."""
    if hasattr(node, 'decorator_list'):
        for d in node.decorator_list:
            if hasattr(d, 'func') and d.func.attr.startswith('deprecated_'):
                return '\n@deprecated_at{%s} %s' % (d.args[0].value,
                                                    d.args[1].value)
    return ''


def get_dump_docstring(node, add_lines=[]):
    lines = []
    doc = ast.get_docstring(node)
    if doc:
        doc += get_deprecation_docstring(node)
        prefix = "## "
        for line in doc.split('\n') + add_lines:
            lines.append(prefix + line)
            prefix = "#  "
    return lines


def dump_function(func, indent, printer, method=False):
    if method:
        add_lines = []
    else:
        add_lines = ['', '@pythononlyfunction']
    d = get_dump_docstring(func, add_lines)
    printer.output_lines(indent, d + [get_function_signature(func)],
                         func.lineno)
    printer.output_line(indent + 4, "pass")


def dump_class(cls, meths, indent, printer):
    if cls.swig:
        add_lines = []
    else:
        add_lines = ['', '@pythononlyclass']
    d = get_dump_docstring(cls, add_lines)
    printer.output_lines(indent, d + [get_class_signature(cls)], cls.lineno)
    if len(meths) == 0:
        printer.output_line(indent + 4, "pass")
    for m in meths:
        dump_function(m, indent + 4, printer, method=True)


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


class OutputPrinter:

    def __init__(self):
        # The number of the next line to be written (1-based)
        self.lineno = 1
        self.last_line_was_comment = False

    def output_line(self, indent, line):
        """Output a single line of text at the given indentation level."""
        print(" " * indent + line)
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
        # Make sure multiple comments (e.g. a namespace docstring followed
        # by the first class docstring) have at least one blank line between
        # them so doxygen knows to start a new paragraph
        if self.last_line_was_comment and lines[0].startswith('#') \
                and pad <= 0:
            pad = 1
        if pad > 0:
            sys.stdout.write('\n' * pad)
            self.lineno += pad
        for line in lines:
            self.output_line(indent, line)
        self.last_line_was_comment = lines[-1].startswith('#')


def parse_file(fname):
    # Exclude non-IMP code
    if '_compat_python' in fname or 'compat_subprocess' in fname:
        return
    # Don't try to document command line tools
    # (which get symlinked into bin dir)
    if '/bin/' in fname:
        return
    # Pass examples through unchanged
    if '/examples/' in fname:
        with open(fname) as fh:
            for line in fh:
                sys.stdout.write(line)
        return
    with open(fname) as fh:
        lines = fh.readlines()
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
