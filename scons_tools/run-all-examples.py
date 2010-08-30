import sys
import time
import os
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


def main(files):
    starttime = time.time()
    errs = []
    for f in files:
        run_example(f, errs)
    runtime = time.time() - starttime
    for e in errs:
        print_error(e)

    print >> sys.stderr, "\n" + "-" * 70
    print >> sys.stderr, "Ran %d examples in %.3fs" % (len(files), runtime)
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

def run_example(f, errs):
    example_name = "example %s" % os.path.basename(f)
    def handle_error(e, errs):
        sys.stderr.write("ERROR\n")
        errs.append((example_name, e, format_exc()))
    sys.stderr.write("Running %s ... " % example_name)
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
    main(sys.argv[1:])
