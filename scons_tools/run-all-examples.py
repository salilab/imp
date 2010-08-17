import sys
import time
import os
import traceback

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
        errs.append((example_name, e, traceback.format_exc()))
    sys.stderr.write("Running %s ... " % example_name)
    try:
        exec open(f) in {}
        sys.stderr.write("ok\n")
        return
    except SystemExit, e: # SystemExit is not a subclass of
                          # Exception in newer Pythons
        if e.code == 0:
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
