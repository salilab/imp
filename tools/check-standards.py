#!/usr/bin/python

import sys
import os.path
from reindent import Reindenter
import re
try:
    import cpp_format
except ImportError:
    cpp_format = None
    print "Cannot conduct additional C++ format checks without the Python "
    print "Pygments (http://pygments.org/) library: please install."
    print "Continuing anyway, but some checks will be postponed " + \
          "to 'svn ci' time..."
    print

def check_c_file(filename, errors):
    """Check each modified C file to make sure it adheres to the standards"""
    fh = file(filename, "r")
    srch = re.compile('\s+$')
    url = re.compile('https?://')
    blank = False
    for (num, line) in enumerate(fh):
        line = line.rstrip('\r\n')
        # No way to split URLs, so let them exceed 80 characters:
        if len(line) > 80 and not url.search(line):
            errors.append('%s:%d: Line is longer than 80 characters.' \
                          % (filename, num+1))
        if line.find('\t') >= 0:
            errors.append('%s:%d: Line contains tabs.' % (filename, num+1))
        if srch.search(line):
            errors.append('%s:%d: Line has trailing whitespace' \
                          % (filename, num+1))
        blank = (len(line) == 0)
        if blank and num == 0:
            errors.append('%s:1: File has leading blank line(s)' % filename)
    if blank:
        errors.append('%s:1000: File has trailing blank line(s)' % filename)

def check_python_file(filename, errors):
    """Check each modified Python file to make sure it adheres to the
       standards"""
    temptest = re.compile('\s+def\s+temp_hide_test.*')
    for (num, line) in enumerate(file(filename, "r")):
        if temptest.match(line):
            errors.append('%s:%d: Test case has the temp_hide_ prefix' \
                          % (filename, num+1))
    fh = file(filename, "r")
    r = Reindenter(fh)
    try:
        if r.run():
            errors.append('Python file ' + filename + ' has odd indentation; ' \
                          + 'please run through reindent.py first.')
    except Exception:
        print >> sys.stderr, "reindent.py FAILED on %s:" % filename
        raise

def get_file(filename):
    return (file(filename, 'r'), filename)

def check_modified_file(filename, errors):
    """Check each modified file to make sure it adheres to the standards"""
    if filename.endswith('.h') or filename.endswith('.cpp') \
       or filename.endswith('.c'):
        check_c_file(filename, errors)
        if cpp_format and filename.endswith('.h'):
            cpp_format.check_header_file(get_file(filename), errors)
        elif cpp_format and filename.endswith('.cpp'):
            cpp_format.check_cpp_file(get_file(filename), errors)
    elif filename.endswith('.py') or filename.endswith('SConscript') \
         or filename.endswith('SConstruct'):
        check_python_file(filename, errors)

def file_matches_re(pathname, excludes):
    for e in excludes:
        if e.match(pathname):
            return True
    return False

def get_all_files():
    modfiles = []
    excludes = ['\.\/kernel\/pyext\/IMP_wrap\.[h|cc]',
                '\.\/kernel\/pyext\/IMP\.py',
                '\.\/modules\/\w+\/pyext\/\w+_wrap\.[h|cc]',
                '\.\/modules\/\w+\/pyext\/IMP\.\w+\.py']
    excludes = [re.compile(x) for x in excludes]
    for root, dirs, files in os.walk('.'):
        if '.sconf_temp' not in root and not root.startswith('./build/'):
            for f in files:
                if not f.startswith('.'):
                    pathname = os.path.join(root, f)
                    if not file_matches_re(pathname, excludes):
                        modfiles.append(pathname)
    return modfiles

def main():
    errors = []

    modfiles = get_all_files()
    for filename in modfiles:
        check_modified_file(filename, errors)

    if len(errors) > 0:
        sys.stderr.write("The following problems needed to be fixed:\n\n")
        sys.stderr.write("\n".join(errors))
        sys.stderr.write("\n")
        sys.exit(1)

if __name__ == '__main__':
    main()
