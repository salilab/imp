#!/usr/bin/env python

"""Build as many IMP components (modules, applications) as possible, even
   if some of them fail."""

import tools
import time
import os
import sys
import optparse
import subprocess
import shutil
try:
    import cPickle as pickle
except ImportError:
    import pickle

class Component(object):
    """Represent an IMP application or module"""
    def __init__(self, name, target=None):
        self.name = name
        self.target = target or "IMP." + name
        self.done = False
        self.dep_failure = None
    def set_dep_modules(self, comps, modules, dependencies,
                        special_dep_targets):
        self.modules = [comps[m] for m in modules]
        for d in dependencies:
            if d in special_dep_targets and d in comps:
                self.modules.append(comps[d])
    def try_build(self, builder, summary):
        if self.done:
            return False
        for m in self.modules:
            if not m.done:
                return False
            elif m.build_result != 0:
                # Propagate the original failure
                self.dep_failure = m.dep_failure or m
                print "%s: skipped due to previous failure to build %s" \
                      % (self.name, self.dep_failure.name)
                self.build_result = 'depfail'
                self.done = True
                summary.write()
                return True
        # All dependencies built OK (or no dependencies), so we can build too
        self.build_result = 'running'
        summary.write()
        self.build_result, self.build_time = builder.build(self)
        self.done = True
        summary.write()
        return True
    def test(self, builder, test_type, summary, expensive):
        if self.build_result != 0:
            print "%s: %s skipped due to build failure" % (self.name, test_type)
        else:
            setattr(self, test_type+'_result', 'running')
            summary.write()
            result, time = builder.test(self, test_type, expensive)
            setattr(self, test_type+'_result', result)
            setattr(self, test_type+'_time', time)
            summary.write()
    def get_build_summary(self):
        ret = {}
        for typ in ('build', 'test', 'benchmark', 'example'):
            if hasattr(self, typ+'_result'):
                ret[typ+'_result'] = getattr(self, typ+'_result')
            if hasattr(self, typ+'_time'):
                ret[typ+'_time'] = getattr(self, typ+'_time')
        return ret


class Builder(object):
    def __init__(self, makecmd, testcmd, outdir):
        self.makecmd = makecmd
        self.testcmd = testcmd
        self.outdir = outdir

    def test(self, component, typ, expensive):
        cmd = "%s -R '^%s\.' -L %s" % (self.testcmd, component.name, typ)
        if expensive == False:
            cmd += " -E expensive"
        if self.outdir:
            cmd += " -T Test"
        ret = self._run_command(component, cmd, typ)
        if self.outdir:
            # Copy XML into output directory
            subdir = open('Testing/TAG').readline().rstrip('\r\n')
            xml = os.path.join(self.outdir,
                               '%s.%s.xml' % (component.name, typ))
            shutil.copy('Testing/%s/Test.xml' % subdir, xml)
        return ret

    def build(self, component):
        cmd = "%s %s" % (self.makecmd, component.target)
        return self._run_command(component, cmd, 'build')

    def _run_command(self, component, cmd, typ):
        if self.outdir:
            outfile = os.path.join(self.outdir,
                                   '%s.%s.log' % (component.name, typ))
            print "%s > %s" % (cmd, outfile)
            outfh = open(outfile, 'w')
            print >> outfh, "Executing: %s" % cmd
            outfh.flush()
            errfh = subprocess.STDOUT
        else:
            print cmd
            outfh = errfh = None
        sys.stdout.flush()
        starttime = time.time()
        ret = subprocess.call(cmd, shell=True, stdout=outfh, stderr=errfh)
        endtime = time.time()
        if ret != 0:
            print "%s: %s FAILED with exit code %d" % (component.name, typ, ret)
            if self.outdir:
                print >> outfh, "Command FAILED with exit code %d" % ret
        return (ret, endtime - starttime)


def internal_dep(dep):
    """Return True iff the given dependency is being built as part of IMP"""
    libdep = "%s_LIBRARY" % dep
    for line in open('CMakeCache.txt'):
        if line.startswith(libdep):
            return line.rstrip('\r\n').endswith('NOTFOUND')
    return False

def get_all_components():

    comps = {}

    # If RMF is being built as part of IMP, split out its build (rather than
    # building it as part of IMP.rmf)
    special_dep_targets = {"RMF": "RMF_all"}
    for dep, tgt in special_dep_targets.items():
        i = tools.get_dependency_info(dep, "")
        if i['ok'] and internal_dep(dep):
            comps[dep] = Component(dep, target=tgt)
            comps[dep].set_dep_modules(comps, [], [], special_dep_targets)

    modules = tools.get_sorted_order()
    apps = tools.get_all_configured_applications()
    for m in modules:
        comps[m] = Component(m)
    for a in apps:
        comps[a] = Component(a)

    for m in modules:
        i = tools.get_module_info(m, "")
        comps[m].set_dep_modules(comps, i['modules'], i['dependencies'],
                                 special_dep_targets)
    for a in apps:
        i = tools.get_application_info(a, "")
        comps[a].set_dep_modules(comps, i['modules'], i['dependencies'],
                                 special_dep_targets)
    return comps

class SummaryWriter(object):
    def __init__(self, summary, comps):
        self.summary = summary
        self.comps = comps
    def write(self):
        if self.summary:
            fh = open(self.summary, 'wb')
            summary = {}
            for m in self.comps.values():
                summary[m.name] = m.get_build_summary()
            pickle.dump(summary, fh, -1)


def test_all(comps, builder, test_type, summary_writer, expensive=None):
    for m in comps.values():
        m.test(builder, test_type, summary_writer, expensive)

def get_comps_to_build(all_comps, exclude):
    if not exclude:
        return all_comps
    else:
        p = pickle.load(open(exclude))
        for compname, result in p.items():
            if compname in all_comps:
                c = all_comps[compname]
                c.done = True
                c.build_result = result.get('build_result', 1)
        comps = {}
        for key, val in all_comps.items():
            if key not in p:
                comps[key] = val
        return comps

def build_all(builder, opts):
    all_comps = get_all_components()
    comps = get_comps_to_build(all_comps, opts.exclude)

    summary_writer = SummaryWriter(opts.summary, comps)
    summary_writer.write()

    while True:
        built = 0
        for m in comps.values():
            if m.try_build(builder, summary_writer):
                built += 1
        if built == 0:
            break
    # If a component didn't build, there must be a dependency problem somewhere
    for m in comps.values():
        if not m.done:
            print "%s: did not build (circular dependency?)" % m.name
            m.build_result = 'circdep'
    summary_writer.write()
    if opts.tests == 'fast':
        test_all(comps, builder, 'test', summary_writer, expensive=False)
    elif opts.tests == 'all':
        test_all(comps, builder, 'test', summary_writer, expensive=True)
    if opts.examples:
        test_all(comps, builder, 'example', summary_writer)
    if opts.benchmarks:
        test_all(comps, builder, 'benchmark', summary_writer)
    for m in comps.values():
        for typ in ('build', 'benchmark'):
            if getattr(m, typ+'_result', 0) != 0:
                sys.exit(1)

def parse_args():
    from optparse import OptionParser
    usage = """%prog [options] makecmd

Build (and optionally test) all components (modules, applications) using the
given makecmd (e.g. "make", "ninja", "make -j8").

This is similar to just running the makecmd itself, but will build as many
components as possible, rather than stopping at the first failure. Build output
can also be sent to separate files for each component (--outdir).

Certain dependencies (e.g. RMF) are also treated as components if they are
being built as part of IMP. (This allows failures in building RMF to easily
be distinguished from errors in IMP.rmf.)

Exit value is 1 if a build or benchmark failed, or 0 otherwise (test or example
failures are considered to be non-fatal).
"""
    parser = OptionParser(usage=usage)
    parser.add_option("--summary",
                      default=None,
                      help="Dump summmary info as a Python pickle to the "
                           "named file. For each component, the time taken to "
                           "run makecmd is recorded, plus the build result, "
                           "which is either the return value of makecmd, or "
                           "'circdep' (the component was not built due to a "
                           "dependency problem), 'depfail' (not built because "
                           "a dependency failed to build), or 'running' (the "
                           "build hasn't finished yet). (If the build hasn't "
                           "started yet, the key is missing.) The summary "
                           "info is updated after each component build.")
    parser.add_option("--outdir",
                      default=None,
                      help="Direct build output to the given directory; one "
                           "file for each component is generated in the "
                           "directory. If not given, output is sent to "
                           "standard output.")
    parser.add_option("--run-tests", metavar='TESTS', type='choice',
                      dest="tests", choices=['none', 'fast', 'all'],
                      default='none',
                      help="none: don't run tests (default); fast: run only "
                           "fast tests; all: run expensive and fast tests")
    parser.add_option("--run-benchmarks", action="store_true",
                      dest="benchmarks", default=False,
                      help="If set, run benchmarks")
    parser.add_option("--run-examples", action="store_true",
                      dest="examples", default=False,
                      help="If set, run examples")
    parser.add_option("--ctest", default="ctest",
                      help="Command (and optional arguments) to use to run "
                           "tests/benchmarks/examples, e.g. \"ctest -j8\", "
                           "\"ctest28\". Defaults to 'ctest'.")
    parser.add_option("--exclude",
                      default=None,
                      help="Build only those modules *not* mentioned in the "
                           "named file (which should be the output of a "
                           "previous run with --summary).")
    opts, args = parser.parse_args()
    if len(args) != 1:
        parser.error("incorrect number of arguments")
    return opts, args

def main():
    opts, args = parse_args()
    build_all(Builder(args[0], opts.ctest, opts.outdir), opts)

if __name__ == '__main__':
    main()
