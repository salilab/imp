#!/usr/bin/env python

"""
Extract test names from all Python unittest files for the given module
or application.
"""

from optparse import OptionParser
import glob
import os

def get_test_name(meth, fname, clsname, methname, doc):
    docstring = ""
    if doc is not None:
        docstring = doc.split("\n")[0].strip()
    if docstring:
        return docstring
    else:
        return "%s (%s.%s)" % (methname,
                               os.path.splitext(os.path.split(fname)[1])[0],
                               clsname)

TEST_BASE_CLASSES = ('TestCase', 'ApplicationTestCase')

try:
    import ast

    def is_test_class(cls):
        """Return True iff cls is derived from IMP.TestCase"""
        if len(cls.bases) == 1:
            if isinstance(cls.bases[0], ast.Name):
                basecls = cls.bases[0].id
            else:
                basecls = cls.bases[0].attr
            return basecls in TEST_BASE_CLASSES

    def get_test_methods(fname):
        seen_classes = {}
        out = []
        lines = open(fname).readlines()
        lines = [x.rstrip('\r\n') for x in lines]
        a = ast.parse(("\n".join(lines)).rstrip() + '\n', fname)
        for cls in ast.iter_child_nodes(a):
            if isinstance(cls, ast.ClassDef) and is_test_class(cls):
                if cls.name in seen_classes:
                    raise ValueError("Duplicate class %s in %s" \
                                     % (cls.name, fname))
                seen_classes[cls.name] = {}
                seen_methods = {}
                for meth in ast.iter_child_nodes(cls):
                    if isinstance(meth, ast.FunctionDef) \
                       and meth.name.startswith('test'):
                        if meth.name in seen_methods:
                            raise ValueError("Duplicate method %s in %s" \
                                             % (meth.name, fname))
                        seen_methods[meth.name] = {}
                        testname = get_test_name(meth, fname, cls.name,
                                                 meth.name,
                                                 ast.get_docstring(meth, False))
                        out.append("%s.%s %s" % (cls.name, meth.name, testname))
        return out

except ImportError:
    # ast module is not available (needs Python 2.6 or later); use compiler
    # instead
    import compiler

    def is_test_class(cls):
        """Return True iff cls is derived from IMP.TestCase"""
        if len(cls.bases) == 1:
            if isinstance(cls.bases[0], compiler.ast.Name):
                basecls = cls.bases[0].name
            else:
                basecls = cls.bases[0].attrname
            return basecls in TEST_BASE_CLASSES

    def get_test_methods(fname):
        out = []
        a = compiler.parseFile(fname)
        for stmt in a.getChildNodes():
            for cls in stmt.getChildNodes():
                if isinstance(cls, compiler.ast.Class) and is_test_class(cls):
                    for stmt2 in cls.getChildNodes():
                        for meth in stmt2.getChildNodes():
                            if isinstance(meth, compiler.ast.Function) \
                               and meth.name.startswith('test'):
                                testname = get_test_name(meth, fname, cls.name,
                                                         meth.name, meth.doc)
                                out.append("%s.%s %s" % (cls.name, meth.name,
                                                         testname))
        return out

def get_tests(fname, output):
    out = get_test_methods(fname)
    if len(out) > 0:
        outfh = open(output, 'w')
        for o in out:
            print >> outfh, o
    return True

def get_args():
    parser = OptionParser(usage="""%prog [options] source_directory

Extract test names from all Python unittest files for the given module
or application.
""")
    parser.add_option('--module', default=None,
                      help="Extract tests for the given module")
    parser.add_option('--application', default=None,
                      help="Extract tests for the given application")
    opts, args = parser.parse_args()
    if len(args) != 1:
        parser.error("wrong number of arguments")
    if not(opts.module) and not(opts.application):
        parser.error("either --module or --application must be specified")
    return opts, args[0]

def glob_files(pattern, outdir):
    done_ok = True
    for f in glob.glob(pattern):
        outfname = os.path.splitext(os.path.split(f)[1])[0] + '.pytests'
        done_ok = done_ok and get_tests(f, os.path.join(outdir, outfname))
    return done_ok

def main():
    opts, source = get_args()
    pats = ['%s/test_*.py', '%s/*/test_*.py',
            '%s/*_test_*.py', '%s/*/*_test_*.py']
    if opts.module:
        srcpath = '%s/modules/%s/test/' % (source, opts.module)
        outdir = 'test/%s' % opts.module
    else:
        srcpath = '%s/applications/%s/test/' % (source, opts.application)
        outdir = 'test/%s' % opts.application
    done_ok = glob_files('%s/test_*.py' % outdir, outdir)
    for p in pats:
        done_ok = done_ok and glob_files(p % srcpath, outdir)
    if not done_ok:
        sys.exit(1)

if __name__ == '__main__':
    main()
