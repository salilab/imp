#!/usr/bin/env python

"""
Extract test names from all Python unittest files for the given module
or application.
"""

import inspect
import traceback
import sys
import glob
import os.path
from optparse import OptionParser

def find_first_class(lines):
    for i, line in enumerate(lines):
        if line.startswith('class'):
            return i
    return len(lines)

def get_test_name(meth, fname, clsname, methname):
    if meth.__doc__ is None:
        return "%s (%s.%s)" % (methname,
                               os.path.splitext(os.path.split(fname)[1])[0],
                               clsname)
    else:
        return meth.__doc__.split("\n")[0].strip()

def get_tests(fname, output):
    lines = open(fname).readlines()
    # Strip off newlines, since exec can get confused by DOS format
    # files otherwise
    lines = [x.rstrip('\r\n') for x in lines]
    # Cut out global init, imports, etc. (anything up to the first class
    # declaration)
    first_class = find_first_class(lines)
    # Add dummy definitions so the module can compile
    lines = ['import sys',
             'class IMP:',
             '  class IntKey:',
             '    def __init__(self, val):',
             '      pass',
             '  class PairScore:',
             '    pass',
             '  class PairPredicate:',
             '    pass',
             '  class SingletonPredicate:',
             '    pass',
             '  class Restraint:',
             '    pass',
             '  class OptimizerState:',
             '    pass',
             '  class ScoreState:',
             '    pass',
             '  class display:',
             '    class TextWriter:',
             '      pass',
             '  class core:',
             '    class PeriodicOptimizerState:',
             '      pass',
             '  class domino:',
             '    class ParticleStates:',
             '      pass',
             '  class test:',
             '    def expectedFailure(f):',
             '      return f',
             '    expectedFailure = staticmethod(expectedFailure)',
             '    class TestCase:',
             '      pass',
             '    class ApplicationTestCase(TestCase):',
             '      pass'] + lines[first_class:]
    globs = {}
    try:
        exec("\n".join(lines) + "\n", globs)
    except Exception, detail:
        print >> sys.stderr, "Problem parsing %s: %s\n%s" \
              % (fname, str(detail), traceback.format_exc())
        return False

    out = []
    for clsname, cls in globs.items():
        if inspect.isclass(cls) and issubclass(cls, globs['IMP'].test.TestCase):
            for methname, meth in inspect.getmembers(cls):
                if inspect.ismethod(meth) and methname.startswith('test'):
                    testname = get_test_name(meth, fname, clsname, methname)
                    out.append('%s.%s %s' % (clsname, methname, testname))
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

def main():
    opts, source = get_args()
    if opts.module:
        srcpath = '%s/modules/%s/test/' % (source, opts.module)
        pats = ['%s/test_*.py', '%s/*/test_*.py',
                '%s/expensive_test_*.py', '%s/*/expensive_test_*.py']
        outdir = 'test/%s' % opts.module
    else:
        srcpath = '%s/applications/%s/test/' % (source, opts.application)
        pats = ['%s/test_*.py', '%s/*/test_*.py']
        outdir = 'test/%s' % opts.application
    done_ok = True
    for p in pats:
        files = glob.glob(p % srcpath)
        for f in files:
            outfname = os.path.splitext(os.path.split(f)[1])[0] + '.pytests'
            done_ok = done_ok and get_tests(f, os.path.join(outdir, outfname))
    if not done_ok:
        sys.exit(1)

if __name__ == '__main__':
    main()
