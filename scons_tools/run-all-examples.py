import sys
import time
import os
import re
import traceback

def format_exc(limit=None):
    # Note that the traceback module in Python 2.3 does not have the format_exc
    # function. We thus provide our own (ported from Python 2.6) so that the
    # build functions correctly on Python 2.3.
    try:
        etype, value, tb = sys.exc_info()
        return ''.join(traceback.format_exception(etype, value, tb, limit))
    finally:
        etype = value = tb = None


def main(disabled_modules, files):
    starttime = time.time()
    errs = []
    for f in files:
        run_example(f, disabled_modules, errs)
    runtime = time.time() - starttime
    for e in errs:
        print_error(e)

    print >> sys.stderr, "\n" + "-" * 70
    suffix = ""
    if len(files) != 1:
        suffix = "s"
    print >> sys.stderr, "Ran %d example%s in %.3fs" % (len(files), suffix,
                                                        runtime)
    if len(errs) > 0:
        print >> sys.stderr, "\nFAILED (errors=%d)" % len(errs)
        sys.exit(1)
    else:
        print >> sys.stderr, "\nOK"

def print_error(e):
    print >> sys.stderr, "\n" + "=" * 70
    print >> sys.stderr, "ERROR: " + e[0]
    print >> sys.stderr, "-" * 70
    if e[2]:
        sys.stderr.write(e[2])
    else:
        print >> sys.stderr, str(e[1])

def get_unmet_module_deps(f, disabled_modules):
    unmet_deps = []
    def check_disabled(modname):
        if modname.startswith("IMP.") and modname[4:] in disabled_modules:
            unmet_deps.append(modname[4:])
    import_re = re.compile('\s*import\s+(.*)\s*')
    from_re = re.compile('\s*from\s+(\S+)\s+import\s+(.*)\s*')
    for line in open(f):
        # Parse lines of the form 'import a.b, a.c'
        m = import_re.match(line)
        if m:
            for modname in [x.strip() for x in m.group(1).split(',')]:
                check_disabled(modname)
        # Parse lines of the form 'from a import b, c'
        m = from_re.match(line)
        if m:
            for modname in [x.strip() for x in m.group(2).split(',')]:
                check_disabled(m.group(1) + '.' + modname)
    return unmet_deps

def run_example(f, disabled_modules, errs):
    unmet_deps = get_unmet_module_deps(f, disabled_modules)
    example_name = "example %s" % os.path.basename(f)
    def handle_error(e, errs):
        sys.stderr.write("ERROR\n")
        errs.append((example_name, e, format_exc()))
    sys.stderr.write("Running %s ... " % example_name)
    if len(unmet_deps) > 0:
        if len(unmet_deps) == 1:
            sys.stderr.write("skipped since module '%s' is disabled\n" \
                             % unmet_deps[0])
        else:
            sys.stderr.write("skipped since modules %s are disabled\n" \
                             % ", ".join(["'%s'" % x for x in unmet_deps]))
        return
    try:
        exec open(f) in {}
        sys.stderr.write("ok\n")
        return
    # SystemExit was moved in the Exception class hierarchy between Python
    # versions, so the only way to catch *all* exceptions on all Python versions
    # is to first catch SystemExit, then Exception.
    except SystemExit, e:
        if e.code == 0 or e.code is None:
            sys.stderr.write("ok\n")
            return
        else:
            handle_error(e, errs)
            return
    except Exception, e:
        handle_error(e, errs)
        return

if __name__ == "__main__":
    main(sys.argv[1].split(":"), sys.argv[2:])
